#include "stdafx.h"
#include "../xrEngine/igame_persistent.h"
#include "../xrParticles/psystem.h"
#include "Level.h"
#include "hudmanager.h"
#include "net_queue.h"
#include "game_cl_base.h"
#include "GameTaskManager.h"
#include "Level_Bullet_Manager.h"
#include "script_process.h"
#include "../xrEngine/date_time.h"
#include "seniority_hierarchy_holder.h"
#include "space_restriction_manager.h"
#include "client_spawn_manager.h"
#include "autosave_manager.h"
#include "ClimableObject.h"
#include "level_graph.h"
#include "phcommander.h"
#include "map_manager.h"
#include "level_sounds.h"
#include "trade_parameters.h"
#include "../xrUICore/MainMenu.h"
#include "player_hud.h"
#include "UI/UIGameTutorial.h"
#include "CustomDetector.h"

#include "ai/stalker/ai_stalker.h"
#include "debug_renderer.h"
#include "physicobject.h"
#include "..\xrEngine\string_table.h"

#ifdef DEBUG
#	include "ai_debug.h"
#	include "phdebug.h"
#	include "level_debug.h"
#endif

extern CUISequencer * g_tutorial;
extern CUISequencer * g_tutorial2;

extern ENGINE_API BOOL g_appLoaded;

//get object ID from spawn data
u16	GetSpawnInfo(NET_Packet &P, u16 &parent_id)
{
	u16 dummy16, id;
	P.r_begin(dummy16);
	shared_str	s_name;
	P.r_stringZ(s_name);
	CSE_Abstract*	E = F_entity_Create(*s_name);
	E->Spawn_Read(P);

	if (E->s_flags.is(M_SPAWN_UPDATE))
		E->UPDATE_Read(P);

	id = E->ID;
	parent_id = E->ID_Parent;
	F_entity_Destroy(E);
	P.r_pos = 0;
	return id;
}

bool CLevel::PostponedSpawn(u16 id)
{
	for (NET_Event& pEvent : spawn_events->queue)
	{
		NET_Packet P;
		if (M_SPAWN != pEvent.ID)
			continue;

		pEvent.implication(P);
		u16 parent_id;
		if (id == GetSpawnInfo(P, parent_id))
			return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void CLevel::mtLevelScriptUpdater(void* pCLevel)
{
	CLevel* pLevel = reinterpret_cast<CLevel*>(pCLevel);
	while (true)
	{
		WaitForSingleObject(pLevel->m_mtScriptUpdaterEventStart, INFINITE);
		if (g_pGameLevel != pLevel) return;

		// Disable objects
		psDeviceFlags.set(rsDisableObjectsAsCrows, false);

		Fvector temp_vector;
		pLevel->m_feel_deny.feel_touch_update(temp_vector, 0.f);

		// Call level script
		CScriptProcess * levelScript = ai().script_engine().script_process(ScriptEngine::eScriptProcessorLevel);
		if (levelScript) levelScript->update();

		SetEvent(pLevel->m_mtScriptUpdaterEventEnd);
	}
}

CLevel::CLevel() :IPureClient(Device.GetTimerGlobal())
{
	g_bDebugEvents = strstr(Core.Params, "-debug_ge") ? TRUE : FALSE;

	Server = nullptr;

	game = nullptr;
	game_events = xr_new<NET_Queue_Event>();

	spawn_events = xr_new<NET_Queue_Event>();
	game_configured = FALSE;
	m_connect_server_err = xrServer::ErrNoError;

	eEnvironment = Engine.Event.Handler_Attach("LEVEL:Environment", this);
	eEntitySpawn = Engine.Event.Handler_Attach("LEVEL:spawn", this);

	m_pBulletManager = xr_new<CBulletManager>();

	m_map_manager = xr_new<CMapManager>();
	m_game_task_manager = xr_new<CGameTaskManager>();

	//----------------------------------------------------
	m_seniority_hierarchy_holder = xr_new<CSeniorityHierarchyHolder>();

	m_level_sound_manager = xr_new<CLevelSoundManager>();
	m_space_restriction_manager = xr_new<CSpaceRestrictionManager>();
	m_client_spawn_manager = xr_new<CClientSpawnManager>();
	m_autosave_manager = xr_new<CAutosaveManager>();

	m_debug_renderer = xr_new<CDebugRenderer>();
#ifdef DEBUG
	m_level_debug = xr_new<CLevelDebug>();
	m_bEnvPaused = false;
	m_bSynchronization = false;
#endif

	m_ph_commander = xr_new<CPHCommander>();
	m_ph_commander_scripts = xr_new<CPHCommander>();

	//---------------------------------------------------------
	pStatGraphR = nullptr;
	//---------------------------------------------------------
	pCurrentControlEntity = nullptr;
	//---------------------------------------------------------	
	R_ASSERT(!g_player_hud);
	g_player_hud = xr_new<player_hud>();
	g_player_hud->load_default();

	hud_zones_list = nullptr;

	m_mtScriptUpdaterEventStart = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	m_mtScriptUpdaterEventEnd = CreateEvent(nullptr, TRUE, FALSE, nullptr);

	thread_spawn(mtLevelScriptUpdater, "X-Ray: Level Script Update", 0, this);
}

CLevel::~CLevel()
{
	g_pGameLevel = nullptr;
	SetEvent(m_mtScriptUpdaterEventStart);
	xr_delete(g_player_hud);
	delete_data(hud_zones_list);
	hud_zones_list = nullptr;

	Msg("- Destroying level");

	Engine.Event.Handler_Detach(eEntitySpawn, this);
	Engine.Event.Handler_Detach(eEnvironment, this);

	if (physics_world())
	{
		destroy_physics_world();
		xr_delete(m_ph_commander_physics_worldstep);
	}

	// destroy PSs
	for (CParticlesObject* &p_it : m_StaticParticles)
		CParticlesObject::Destroy(p_it);

	m_StaticParticles.clear();

	// Unload sounds
	// unload prefetched sounds
	sound_registry.clear();

	// unload static sounds
	for (ref_sound* static_Sound : static_Sounds) {
		static_Sound->destroy();
		xr_delete(static_Sound);
	}
	static_Sounds.clear();

	xr_delete(m_level_sound_manager);
	xr_delete(m_space_restriction_manager);
	xr_delete(m_seniority_hierarchy_holder);
	xr_delete(m_client_spawn_manager);
	xr_delete(m_autosave_manager);
	xr_delete(m_debug_renderer);

	ai().script_engine().remove_script_process(ScriptEngine::eScriptProcessorLevel);

	xr_delete(game);
	xr_delete(game_events);


	//by Dandy
	//destroy bullet manager
	xr_delete(m_pBulletManager);
	//-----------------------------------------------------------
	xr_delete(pStatGraphR);
	//-----------------------------------------------------------
	xr_delete(m_ph_commander);
	xr_delete(m_ph_commander_scripts);
	//-----------------------------------------------------------
	ai().unload();
	//-----------------------------------------------------------	
#ifdef DEBUG	
	xr_delete(m_level_debug);
#endif
	//-----------------------------------------------------------
	xr_delete(m_map_manager);
	delete_data(m_game_task_manager);

	// here we clean default trade params
	// because they should be new for each saved/loaded game
	// and I didn't find better place to put this code in
	CTradeParameters::clean();

	if (g_tutorial && g_tutorial->m_pStoredInputReceiver == this)
		g_tutorial->m_pStoredInputReceiver = nullptr;

	if (g_tutorial2 && g_tutorial2->m_pStoredInputReceiver == this)
		g_tutorial2->m_pStoredInputReceiver = nullptr;

	CloseHandle(m_mtScriptUpdaterEventStart);
	CloseHandle(m_mtScriptUpdaterEventEnd);
}

shared_str CLevel::name() const
{
	return (map_data.m_name);
}

shared_str CLevel::name_translated() const
{
	return CStringTable().translate(map_data.m_name);
}

void CLevel::PrefetchSound(LPCSTR name)
{
	// preprocess sound name
	string_path					tmp;
	xr_strcpy(tmp, name);
	xr_strlwr(tmp);
	if (strext(tmp))			*strext(tmp) = 0;
	shared_str	snd_name = tmp;
	// find in registry
	auto it = sound_registry.find(snd_name);
	// if find failed - preload sound
	if (it == sound_registry.end())
		sound_registry[snd_name].create(snd_name.c_str(), st_Effect, sg_SourceType);
}

BOOL g_bDebugEvents = FALSE;

void CLevel::cl_Process_Event(u16 dest, u16 type, NET_Packet& P)
{
	CObject* O = Objects.net_Find(dest);
	if (!O)	return;

	CGameObject* GO = smart_cast<CGameObject*>(O);
	if (!GO)
	{
		return;
	}

	if (type != GE_DESTROY_REJECT)
	{
		GO->OnEvent(P, type);
	}
	else
	{ // handle GE_DESTROY_REJECT here
		u32				pos = P.r_tell();
		u16				id = P.r_u16();
		P.r_seek(pos);

		bool			ok = true;

		CObject			*D = Objects.net_Find(id);
		if (nullptr == D) {
			ok = false;
		}

		CGameObject		*GD = smart_cast<CGameObject*>(D);
		if (!GD) {
			ok = false;
		}

		GO->OnEvent(P, GE_OWNERSHIP_REJECT);
		if (ok)
		{
			GD->OnEvent(P, GE_DESTROY);
		};
	}
};

static std::recursive_mutex MutexGameEventsLock;

void CLevel::ProcessGameEvents()
{
	// Threadsafe for ProcessGameEvents
	std::lock_guard<std::recursive_mutex> guard(MutexGameEventsLock);

	// Game events
	NET_Packet			P;
	u32 svT = timeServer() - NET_Latency;
	if (g_extraFeatures.is(GAME_EXTRA_SPAWN_ANTIFREEZE))
	{
		while (spawn_events->available(svT))
		{
			u16 ID, dest, type;
			spawn_events->get(ID, dest, type, P);
			game_events->insert(P);
		}
	}
	u32 avail_time = 5;
	u32 elps = Device.frame_elapsed();
	if (elps < 30)
		avail_time = 33 - elps;

	u32 work_limit = elps + avail_time;

	while (game_events->available(svT))
	{
		u16 ID, dest, type;
		game_events->get(ID, dest, type, P);

		if (g_extraFeatures.is(GAME_EXTRA_SPAWN_ANTIFREEZE))
		{
			if (g_appLoaded && M_EVENT == ID && PostponedSpawn(dest))
			{
				spawn_events->insert(P);
				continue;
			}
			if (g_appLoaded && M_SPAWN == ID && Device.frame_elapsed() > work_limit)
			{
				u16 parent_id;
				GetSpawnInfo(P, parent_id);
				if (parent_id < 0xffff)
				{
					if (!spawn_events->available(svT))
						Msg("* ProcessGameEvents, spawn event postponed. Events rest = %d", game_events->queue.size());

					spawn_events->insert(P);
					continue;
				}
			}
		}

		switch (ID)
		{
		case M_SPAWN:
		{
			u16 dummy16;
			P.r_begin(dummy16);
			cl_Process_Spawn(P);
		}break;

		case M_EVENT:	cl_Process_Event(dest, type, P); break;
		default:		R_ASSERT(0); break;
		}
	}
}

void CLevel::MakeReconnect()
{
	if (!Engine.Event.Peek("KERNEL:disconnect"))
	{
		Engine.Event.Defer("KERNEL:disconnect");
		char const * server_options = nullptr;
		char const * client_options = nullptr;
		shared_str serverOption = GamePersistent().GetServerOption();
		shared_str clientOption = GamePersistent().GetClientOption();
		if (serverOption.c_str())
		{
			server_options = xr_strdup(*serverOption);
		}
		else
		{
			server_options = xr_strdup("");
		}
		if (clientOption.c_str())
		{
			client_options = xr_strdup(*clientOption);
		}
		else
		{
			client_options = xr_strdup("");
		}
		Engine.Event.Defer("KERNEL:start", size_t(server_options), size_t(client_options));
	}
}

#include "Inventory.h"
void CLevel::OnFrame()
{
	ResetEvent(m_mtScriptUpdaterEventEnd);
	SetEvent(m_mtScriptUpdaterEventStart);

	// commit events from bullet manager from prev-frame
	BulletManager().CommitEvents();
	ClientReceive();

	// Update game events
	ProcessGameEvents();
#ifdef DEBUG
	DBG_RenderUpdate();
#endif // #ifdef DEBUG

	m_map_manager->Update();

	if (Device.dwPrecacheFrame == 0 && Device.dwFrame % 2)
		GameTaskManager().UpdateTasks();

	// Inherited update
	inherited::OnFrame();

#ifdef DEBUG
	// Draw client/server stats
	if (!psDeviceFlags.test(rsStatistic))
	{
		if (pStatGraphR)
			xr_delete(pStatGraphR);
	}
	g_pGamePersistent->Environment().m_paused = m_bEnvPaused;
#endif
	g_pGamePersistent->Environment().SetGameTime(GetEnvironmentGameDayTimeSec(), game->GetEnvironmentGameTimeFactor());


	m_ph_commander->update();
	m_ph_commander_scripts->update();
	BulletManager().CommitRenderSet();

	// update static sounds
	Device.seqParallel.emplace_back(m_level_sound_manager, &CLevelSoundManager::Update);
	// deffer LUA-GC-STEP
	Device.seqParallel.emplace_back(this, &CLevel::script_gc);
	//-----------------------------------------------------
#ifdef DEBUG
	if (pStatGraphR)
	{
		static	float fRPC_Mult = 10.0f;
		static	float fRPS_Mult = 1.0f;

		pStatGraphR->AppendItem(float(m_dwRPC)*fRPC_Mult, 0xffff0000, 1);
		pStatGraphR->AppendItem(float(m_dwRPS)*fRPS_Mult, 0xff00ff00, 0);
	}
#endif

	// Level Script Updater thread can issue a exception. But it require to process one message from HWND message queue, otherwise, Level script can't show error message
	DWORD WaitResult = WAIT_TIMEOUT;

	do
	{
		WaitResult = WaitForSingleObject(m_mtScriptUpdaterEventEnd, 66); // update message box with 15 fps
		if (WaitResult == WAIT_TIMEOUT)
		{
			Device.ProcessSingleMessage();
		}
	} while (WaitResult == WAIT_TIMEOUT);
}

int		psLUA_GCSTEP = 10;
void	CLevel::script_gc()
{
	lua_gc(ai().script_engine().lua(), LUA_GCSTEP, psLUA_GCSTEP);
}

#ifdef DEBUG_PRECISE_PATH
void test_precise_path();
#endif

#ifdef DEBUG
extern	Flags32	dbg_net_Draw_Flags;
#endif

extern void draw_wnds_rects();

void CLevel::OnRender()
{
	::Render->BeforeWorldRender();
	inherited::OnRender();

	if (!game)
		return;

	HUD().RenderUI();
	Game().OnRender();
	BulletManager().Render();
	::Render->AfterWorldRender();

#ifdef DEBUG
	draw_wnds_rects();
	physics_world()->OnRender();

	if (ai().get_level_graph())
		ai().level_graph().render();

#ifdef DEBUG_PRECISE_PATH
	test_precise_path();
#endif

	CAI_Stalker				*stalker = smart_cast<CAI_Stalker*>(Level().CurrentEntity());
	if (stalker)
		stalker->OnRender();

	if (bDebug)
	{
		for (u32 I = 0; I < Level().Objects.o_count(); I++)
		{
			CObject*	_O = Level().Objects.o_get_by_iterator(I);

			CAI_Stalker*		stalker = smart_cast<CAI_Stalker*>(_O);
			if (stalker)
				stalker->OnRender();

			CCustomMonster*		monster = smart_cast<CCustomMonster*>(_O);
			if (monster)
				monster->OnRender();

			CPhysicObject		*physic_object = smart_cast<CPhysicObject*>(_O);
			if (physic_object)
				physic_object->OnRender();

			CSpaceRestrictor	*space_restrictor = smart_cast<CSpaceRestrictor*>	(_O);
			if (space_restrictor)
				space_restrictor->OnRender();
			CClimableObject		*climable = smart_cast<CClimableObject*>	(_O);
			if (climable)
				climable->OnRender();

			if (dbg_net_Draw_Flags.test(dbg_draw_skeleton)) //draw skeleton
			{
				CGameObject* pGO = smart_cast<CGameObject*>	(_O);
				if (pGO && pGO != Level().CurrentViewEntity() && !pGO->H_Parent())
				{
					if (pGO->Position().distance_to_sqr(Device.vCameraPosition) < 400.0f)
					{
						pGO->dbg_DrawSkeleton();
					}
				}
			};
		}
		//  [7/5/2005]
		if (Server && Server->game) Server->game->OnRender();
		//  [7/5/2005]
		ObjectSpace.dbgRender();

		//---------------------------------------------------------------------
		UI().Font().pFontStat->OutSet(170, 630);
		UI().Font().pFontStat->SetHeight(16.0f);
		UI().Font().pFontStat->SetColor(0xffff0000);

		UI().Font().pFontStat->OutNext("Server Objects:      [%d]", Objects.o_count());

		UI().Font().pFontStat->SetHeight(8.0f);

		//---------------------------------------------------------------------
		DBG().draw_object_info();
		DBG().draw_text();
		DBG().draw_level_info();
	}

	debug_renderer().render();

	DBG().draw_debug_text();


	if (psAI_Flags.is(aiVision)) {
		for (u32 I = 0; I < Level().Objects.o_count(); I++) {
			CObject						*object = Objects.o_get_by_iterator(I);
			CAI_Stalker					*stalker = smart_cast<CAI_Stalker*>(object);
			if (!stalker)
				continue;
			stalker->dbg_draw_vision();
		}
	}


	if (psAI_Flags.test(aiDrawVisibilityRays)) {
		for (u32 I = 0; I < Level().Objects.o_count(); I++) {
			CObject						*object = Objects.o_get_by_iterator(I);
			CAI_Stalker					*stalker = smart_cast<CAI_Stalker*>(object);
			if (!stalker)
				continue;

			stalker->dbg_draw_visibility_rays();
		}
	}
#endif
}

void CLevel::OnEvent(EVENT E, u64 P1, u64 /**P2/**/)
{
	if (E == eEntitySpawn)
	{
		char	Name[128];	Name[0] = 0;
		sscanf(LPCSTR(P1), "%s", Name);
		Level().g_cl_Spawn(Name, 0xff, M_SPAWN_OBJECT_LOCAL, Fvector().set(0, 0, 0));
	}
}

ALife::_TIME_ID CLevel::GetStartGameTime()
{
	return(game->GetStartGameTime());
}

ALife::_TIME_ID CLevel::GetGameTime()
{
	return(game->GetGameTime());
}

ALife::_TIME_ID CLevel::GetEnvironmentGameTime()
{
	return(game->GetEnvironmentGameTime());
}

u8 CLevel::GetDayTime()
{
	u32 dummy32;
	u32 hours;
	GetGameDateTime(dummy32, dummy32, dummy32, hours, dummy32, dummy32, dummy32);
	VERIFY(hours < 256);
	return	u8(hours);
}

float CLevel::GetGameDayTimeSec()
{
	return	(float(s64(GetGameTime() % (24 * 60 * 60 * 1000))) / 1000.f);
}

u32 CLevel::GetGameDayTimeMS()
{
	return	(u32(s64(GetGameTime() % (24 * 60 * 60 * 1000))));
}

float CLevel::GetEnvironmentGameDayTimeSec()
{
	return	(float(s64(GetEnvironmentGameTime() % (24 * 60 * 60 * 1000))) / 1000.f);
}

void CLevel::GetGameDateTime(u32& year, u32& month, u32& day, u32& hours, u32& mins, u32& secs, u32& milisecs)
{
	split_time(GetGameTime(), year, month, day, hours, mins, secs, milisecs);
}


float CLevel::GetGameTimeFactor()
{
	return			(game->GetGameTimeFactor());
}

void CLevel::SetGameTimeFactor(const float fTimeFactor)
{
	game->SetGameTimeFactor(fTimeFactor);
}

void CLevel::SetGameTimeFactor(ALife::_TIME_ID GameTime, const float fTimeFactor)
{
	game->SetGameTimeFactor(GameTime, fTimeFactor);
}

void CLevel::SetEnvironmentGameTimeFactor(u64 const& GameTime, float const& fTimeFactor)
{
	if (!game)
		return;

	game->SetEnvironmentGameTimeFactor(GameTime, fTimeFactor);
}

void CLevel::ResetLevel()
{
	MapManager().ResetStorage();
	GameTaskManager().ResetStorage();
}

u32	GameID()
{
	return Game().Type();
}

// -------------------------------------------------------------------------------------------------
CZoneList* CLevel::create_hud_zones_list()
{
	hud_zones_list = xr_new<CZoneList>();
	hud_zones_list->clear();
	return hud_zones_list;
}

BOOL CZoneList::feel_touch_contact(CObject* O)
{
	TypesMapIt it = m_TypesMap.find(O->cNameSect());
	bool res = (it != m_TypesMap.end());

	CCustomZone *pZone = smart_cast<CCustomZone*>(O);
	if (pZone && !pZone->IsEnabled())
	{
		res = false;
	}
	return res;
}

CZoneList::CZoneList()
{
}

CZoneList::~CZoneList()
{
	clear();
	destroy();
}