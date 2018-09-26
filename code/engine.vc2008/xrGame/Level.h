#pragma once
#include "../xrEngine/igame_level.h"
#include "../xrEngine/IGame_Persistent.h"
#include "../xrEngine/ClS/net_client.h"
#include "../xrScripts/export/script_export_space.h"
#include "../xrEngine/StatGraph.h"
#include "../xrServerEntities/xrMessages.h"
#include "../xrServerEntities/../xrAICore/alife_space.h"
#include "../xrcore/xrDebug.h"
#include "xrServer.h"
#include "GlobalFeelTouch.hpp"
#include "Level_network_map_sync.h"

class	CParticlesObject;
class	game_cl_GameState;
class	NET_Queue_Event;
class	CSE_Abstract;
class	CSpaceRestrictionManager;
class	CSeniorityHierarchyHolder;
class	CClientSpawnManager;
class	CGameObject;
class	CAutosaveManager;
class	CPHCommander;
class	CLevelDebug;
class	CLevelSoundManager;
class	CGameTaskManager;
class	CZoneList;
class	CDebugRenderer;

extern float g_fov;

class CBulletManager;
class CMapManager;

class CLevel : public IGame_Level, public IPureClient
{
	void						ClearAllObjects			();
private:
#ifdef DEBUG
	bool						m_bSynchronization;
	bool						m_bEnvPaused;
#endif
protected:
	using inherited = IGame_Level;
	
	CLevelSoundManager			*m_level_sound_manager;
	NET_Queue_Event*			spawn_events = nullptr;
	// movement restriction manager
	CSpaceRestrictionManager	*m_space_restriction_manager;
	// seniority hierarchy holder
	CSeniorityHierarchyHolder	*m_seniority_hierarchy_holder;
	// client spawn_manager
	CClientSpawnManager			*m_client_spawn_manager;
	// autosave manager
	CAutosaveManager			*m_autosave_manager;
	// debug renderer
	CDebugRenderer				*m_debug_renderer;

	CPHCommander				*m_ph_commander;
	CPHCommander				*m_ph_commander_scripts;
	CPHCommander				*m_ph_commander_physics_worldstep;

	// Local events
	EVENT						eEnvironment;
	EVENT						eEntitySpawn;
	//---------------------------------------------
	CStatGraph					*pStatGraphR;
	u32							m_dwRPC;	//ReceivedPacketsCount
	u32							m_dwRPS;	//ReceivedPacketsSize
	//---------------------------------------------
	
public:
#ifdef DEBUG
	CLevelDebug					*m_level_debug; // level debugger
#endif
	////////////// network ////////////////////////

	virtual void				OnMessage				(void* data, u32 size);
			bool				PostponedSpawn			(u16 id);
private:
	CObject*					pCurrentControlEntity;
	xrServer::EConnect			m_connect_server_err;
public:

	CObject*					CurrentControlEntity	(  ) const		{ return pCurrentControlEntity; }
	void						SetControlEntity		( CObject* O  )		{ pCurrentControlEntity=O; }

public:
	//////////////////////////////////////////////	
	// static particles
	using POVec = xr_vector<CParticlesObject*>;
	POVec						m_StaticParticles;

	game_cl_GameState			*game;
	BOOL						game_configured;
	NET_Queue_Event				*game_events;
	xr_deque<CSE_Abstract*>		game_spawn_queue;
	xrServer*					Server;
	GlobalFeelTouch				m_feel_deny;
	
	CZoneList*					hud_zones_list;
	CZoneList*					create_hud_zones_list();

	HANDLE						m_mtScriptUpdaterEventStart;
	HANDLE						m_mtScriptUpdaterEventEnd;

private:
	// preload sounds registry
    using SoundRegistryMap = xr_map<shared_str, ref_sound>;
	SoundRegistryMap			sound_registry;

public:
	void						PrefetchSound (LPCSTR name);

protected:
	BOOL						net_start_result_total;
	BOOL						deny_m_spawn;		//only for debug...
		
	static void					mtLevelScriptUpdater	(void* pCLevel);
	void						MakeReconnect();
	
	LevelMapSyncData			map_data;
	bool						synchronize_client		();

	bool	xr_stdcall			net_start1				();
	bool	xr_stdcall			net_start2				();
	bool	xr_stdcall			net_start4				();
	bool	xr_stdcall			net_start6				();

	bool	xr_stdcall			net_start_client1				();
	bool	xr_stdcall			net_start_client3				();
	bool	xr_stdcall			net_start_client4				();
	bool	xr_stdcall			net_start_client5				();
	bool	xr_stdcall			net_start_client6				();
public:

	// sounds
	xr_vector<ref_sound*>		static_Sounds;

	// Starting/Loading
	virtual BOOL				net_Start				( LPCSTR op_server, LPCSTR op_client);
	virtual void				net_Stop				( );
	virtual void				net_Update				( );


	virtual BOOL				Load_GameSpecific_Before( );
	virtual BOOL				Load_GameSpecific_After ( );
	virtual void				Load_GameSpecific_CFORM	( CDB::TRI* T, u32 count );

	// Events
	virtual void				OnEvent					( EVENT E, u64 P1, u64 P2 );
	virtual void	_BCL		OnFrame					(  );
	virtual void				OnRender				( );
	
	void						cl_Process_Event		(u16 dest, u16 type, NET_Packet& P);
	void						cl_Process_Spawn		(NET_Packet& P);
	void						ProcessGameEvents		( );
	void						ProcessGameSpawns		( );
	
	// Input
	virtual	void				IR_OnKeyboardPress		( int btn );
	virtual void				IR_OnKeyboardRelease	( int btn );
	virtual void				IR_OnKeyboardHold		( int btn );
	virtual void				IR_OnMousePress			( int btn );
	virtual void				IR_OnMouseRelease		( int btn );
	virtual void				IR_OnMouseHold			( int btn );
	virtual void				IR_OnMouseMove			( int, int);
	virtual void				IR_OnMouseStop			( int, int);
	virtual void				IR_OnMouseWheel			( int direction);
	virtual void				IR_OnActivate			();

	// Game
	void						InitializeClientGame	(NET_Packet& P);
	void						ClientReceive			();
	void						ClientSend				(bool bForce = false);
	void						ClientSave				();
			u32					Objects_net_Save		(NET_Packet* _Packet, u32 start, u32 count);
	virtual	void				Send					(NET_Packet& P);
	
	void						g_cl_Spawn				(LPCSTR name, u8 rp, u16 flags, Fvector pos);	// only ask server
	void						g_sv_Spawn				(CSE_Abstract* E);					// server reply/command spawning
	
	IC CSpaceRestrictionManager		&space_restriction_manager	();
	IC CSeniorityHierarchyHolder	&seniority_holder			();
	IC CClientSpawnManager			&client_spawn_manager		();
	IC CAutosaveManager				&autosave_manager			();
	IC CDebugRenderer				&debug_renderer				();
	void	__stdcall				script_gc					();			// GC-cycle

	IC CPHCommander					&ph_commander				();
	IC CPHCommander					&ph_commander_scripts		();
	IC CPHCommander					&ph_commander_physics_worldstep();

	// C/D
	CLevel();
	virtual ~CLevel();

	//название текущего уровня
	virtual shared_str			name					() const;
	virtual shared_str			name_translated			() const;
			shared_str			version					() const { return map_data.m_map_version; } //this method can be used ONLY from CCC_ChangeGameType
	
	//возвращает время в милисекундах относительно начала игры
	virtual u64			GetStartGameTime		();
	virtual u64			GetGameTime				();
	//возвращает время для энвайронмента в милисекундах относительно начала игры
	ALife::_TIME_ID		GetEnvironmentGameTime	();
	//игровое время в отформатированном виде
	void				GetGameDateTime			(u32& year, u32& month, u32& day, u32& hours, u32& mins, u32& secs, u32& milisecs);

	float				GetGameTimeFactor		();
	void				SetGameTimeFactor		(const float fTimeFactor);
	void				SetGameTimeFactor		(ALife::_TIME_ID GameTime, const float fTimeFactor);
	virtual void		SetEnvironmentGameTimeFactor(u64 const& GameTime, float const& fTimeFactor);

	// gets current daytime [0..23]
	u8					GetDayTime				();
	u32					GetGameDayTimeMS		();
	float				GetGameDayTimeSec		();
	float				GetEnvironmentGameDayTimeSec();

protected:	
	CMapManager *			m_map_manager;
	CGameTaskManager*		m_game_task_manager;

public:
	CMapManager&			MapManager					() const 	{return *m_map_manager;}
	CGameTaskManager&		GameTaskManager				() const	{return *m_game_task_manager;}
	void					ResetLevel					();
	
protected:	
	//работа с пулями
	CBulletManager*		m_pBulletManager;
public:
	IC CBulletManager&	BulletManager() {return	*m_pBulletManager;}

	//by Mad Max 
	CSE_Abstract		*spawn_item						(LPCSTR section, const Fvector &position, u32 level_vertex_id, u16 parent_id, bool return_item = false);
			
public:
	void				remove_objects			();

	DECLARE_SCRIPT_REGISTER_FUNCTION
};
add_to_type_list(CLevel)
#undef script_type_list
#define script_type_list save_type_list(CLevel)

IC CLevel&				Level()		{ return *((CLevel*) g_pGameLevel);			}
IC game_cl_GameState&	Game()		{ return *Level().game;					}
	u32					GameID();


#ifdef DEBUG
IC CLevelDebug&			DBG()		{return *((CLevelDebug*)Level().m_level_debug);}
#endif


IC CSpaceRestrictionManager	&CLevel::space_restriction_manager()
{
	VERIFY				(m_space_restriction_manager);
	return				(*m_space_restriction_manager);
}

IC CSeniorityHierarchyHolder &CLevel::seniority_holder()
{
	VERIFY				(m_seniority_hierarchy_holder);
	return				(*m_seniority_hierarchy_holder);
}

IC CClientSpawnManager &CLevel::client_spawn_manager()
{
	VERIFY				(m_client_spawn_manager);
	return				(*m_client_spawn_manager);
}

IC CAutosaveManager &CLevel::autosave_manager()
{
	VERIFY				(m_autosave_manager);
	return				(*m_autosave_manager);
}

IC CDebugRenderer &CLevel::debug_renderer()
{
	VERIFY				(m_debug_renderer);
	return				(*m_debug_renderer);
}

IC CPHCommander	& CLevel::ph_commander()
{
	VERIFY(m_ph_commander);
	return *m_ph_commander;
}
IC CPHCommander & CLevel::ph_commander_scripts()
{
	VERIFY(m_ph_commander_scripts);
	return *m_ph_commander_scripts;
}
IC CPHCommander & CLevel::ph_commander_physics_worldstep()
{
	VERIFY(m_ph_commander_scripts);
	return *m_ph_commander_physics_worldstep;
}

extern BOOL						g_bDebugEvents;

// -------------------------------------------------------------------------------------------------
