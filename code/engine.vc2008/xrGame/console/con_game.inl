
// g_spawn
class CCC_Spawn : public IConsole_Command {
public:
	CCC_Spawn(LPCSTR N) : IConsole_Command(N) { };
	virtual void Execute(LPCSTR args) {
		if (!g_pGameLevel) return;


		if (!pSettings->section_exist(args))
		{
			Msg("! Section [%s] isn`t exist...", args);
			return;
		}

		char	Name[128];	Name[0] = 0;
		sscanf(args, "%s", Name);
		Fvector pos = Actor()->Position();
		pos.y += 3.0f;
		Level().g_cl_Spawn(Name, 0xff, M_SPAWN_OBJECT_LOCAL, pos);
	}
	virtual void	Info(TInfo& I)
	{
		strcpy(I, "name,team,squad,group");
	}
};
// g_spawn 

class CCC_Spawn_to_inventory : public IConsole_Command {
public:
	CCC_Spawn_to_inventory(LPCSTR N) : IConsole_Command(N) { };
	virtual void Execute(LPCSTR args) {
		if (!g_pGameLevel)
		{
			Log("Error: No game level!");
			return;
		}

		if (!pSettings->section_exist(args))
		{
			Msg("! Section [%s] isn`t exist...", args);
			return;
		}

		char	Name[128];	Name[0] = 0;
		sscanf(args, "%s", Name);

		Level().spawn_item(Name, Actor()->Position(), false, Actor()->ID());
	}
	virtual void	Info(TInfo& I)
	{
		strcpy(I, "name,team,squad,group");
	}
};

class CCC_Giveinfo : public IConsole_Command {
public:
	CCC_Giveinfo(LPCSTR N) : IConsole_Command(N) { };
	virtual void Execute(LPCSTR info_id) {
		if (!g_pGameLevel) return;

		char	Name[128];	Name[0] = 0;
		CActor* actor = smart_cast<CActor*>(Level().CurrentEntity());
		if (actor)
			actor->OnReceiveInfo(info_id);

	}
};

class CCC_Disinfo : public IConsole_Command {
public:
	CCC_Disinfo(LPCSTR N) : IConsole_Command(N) { };
	virtual void Execute(LPCSTR info_id) {
		if (!g_pGameLevel) return;

		char	Name[128];	Name[0] = 0;
		CActor* actor = smart_cast<CActor*>(Level().CurrentEntity());
		if (actor)
			actor->OnDisableInfo(info_id);

	}
};

class CCC_GameLanguage : public CCC_Token 
{
public:
	CCC_GameLanguage(LPCSTR N) : CCC_Token(N, (u32*)&g_Language, language_type_token) {};
	virtual void Execute(LPCSTR args) 
	{
		CCC_Token::Execute(args);
		Msg("[GAME] Game language changed!");
		CStringTable().ReInit(g_Language);
	}
	virtual void Info(TInfo& I)
	{
		xr_strcpy(I, "Game language");
	}
};

// console commands
class CCC_GameDifficulty : public CCC_Token {
public:
	CCC_GameDifficulty(LPCSTR N) : CCC_Token(N,(u32*)&g_SingleGameDifficulty,difficulty_type_token)  {};
	virtual void Execute(LPCSTR args) {
		CCC_Token::Execute(args);
		if (g_pGameLevel && Level().game)
		{
			Level().game->OnDifficultyChanged	();
		}
	}
	virtual void	Info	(TInfo& I)		
	{
		xr_strcpy(I,"game difficulty"); 
	}
};

class CCC_UIMapUpdate : public CCC_Float
{
public:
	CCC_UIMapUpdate(LPCSTR N) : CCC_Float(N, &minimap_zoom_factor, 0.1, 6.0)
	{ 
		
	};

	virtual void Execute(const char* args) 
	{
		minimap_zoom_factor = atof(args);
		CUIZoneMap* pZoneMap = GameUI() && GameUI()->UIMainIngameWnd ? GameUI()->UIMainIngameWnd->GetZoneMap() : nullptr;
		if(pZoneMap && pZoneMap->visible)
			pZoneMap->SetupCurrentMap();
	}
};

//-----------------------------------------------------------------------
class CCC_DemoRecord : public IConsole_Command
{
public:

	CCC_DemoRecord(LPCSTR N) : IConsole_Command(N) {};
	virtual void Execute(LPCSTR args) 
	{
		if (g_pGameLevel)
		{
			Console->Hide();

			// Сlose main menu if it is open
			if (MainMenu()->IsActive())
				MainMenu()->Activate(false);

			LPSTR			fn_;
			STRCONCAT(fn_, args, ".xrdemo");
			string_path		fn;
			FS.update_path(fn, "$game_saves$", fn_);

			g_pGameLevel->Cameras().AddCamEffector(xr_new<CDemoRecord>(fn));
		}
		else
			Msg("! There are no level(s) started");
	}
};

class CCC_DemoRecordSetPos : public CCC_Vector3
{
	static Fvector p;
public:

	CCC_DemoRecordSetPos(LPCSTR N) : CCC_Vector3( N, &p, Fvector().set( -FLT_MAX, -FLT_MAX, -FLT_MAX ),Fvector().set( FLT_MAX, FLT_MAX, FLT_MAX ) ) {};
	virtual void Execute(LPCSTR args) 
	{
		CDemoRecord::GetGlobalPosition( p );
		CCC_Vector3::Execute(args);
		CDemoRecord::SetGlobalPosition( p );
	}
	virtual void	Save	(IWriter *F)	{;}

};
Fvector CCC_DemoRecordSetPos::p = {0,0,0};

class CCC_DemoPlay : public IConsole_Command
{
public:
	CCC_DemoPlay(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = TRUE; };
	virtual void Execute(LPCSTR args)
	{
		if (g_pGameLevel)
		{
			Console->Hide();
			string_path fn;
			u32 loops = 0;
			LPSTR comma = strchr(const_cast<LPSTR>(args), ',');
			if (comma)
			{
				loops = atoi(comma + 1);
				*comma = 0;	//. :)
			}
			strconcat(sizeof(fn), fn, args, ".xrdemo");
			FS.update_path(fn, "$game_saves$", fn);
			g_pGameLevel->Cameras().AddCamEffector(xr_new<CDemoPlay>(fn, 1.0f, loops));
		}
		else 
			Msg("! There are no level(s) started");
	}
};

class CCC_LoadLastSave : public IConsole_Command
{
public:
					CCC_LoadLastSave	(LPCSTR N) : IConsole_Command(N)
	{
		bEmptyArgsHandled		= true;
	}

	virtual void	Execute				(LPCSTR args)
	{
		string_path				saved_game = "";
		if ( args )
		{
			strncpy_s			(saved_game, sizeof(saved_game), args, _MAX_PATH - 1 );
		}

		
		if (saved_game && *saved_game)
		{
			xr_strcpy				(g_last_saved_game,saved_game);
			return;
		}

		if (!*g_last_saved_game) {
			Msg					("! cannot load last saved game since it hasn't been specified");
			return;
		}

		if (!CSavedGameWrapper::saved_game_exist(g_last_saved_game)) {
			Msg						("! Cannot find saved game %s",g_last_saved_game);
			return;
		}

		if (!CSavedGameWrapper::valid_saved_game(g_last_saved_game)) {
			Msg						("! Cannot load saved game %s, version mismatch or saved game is corrupted",g_last_saved_game);
			return;
		}

		if ( !valid_saved_game_name(g_last_saved_game) )
		{
			Msg						("! Cannot load saved game %s, invalid file name",g_last_saved_game);
			return;
		}

		LPSTR					command;
		if (ai().get_alife()) {
			STRCONCAT			(command, "load ", g_last_saved_game);
			Console->Execute	(command);
			return;
		}

		STRCONCAT				(command, "start server(", g_last_saved_game, "/single/alife/load)");
		Console->Execute		(command);
	}
	
	virtual void	Save				(IWriter *F)
	{
		if (!*g_last_saved_game)
			return;

		F->w_printf				("%s %s\r\n",cName,g_last_saved_game); 
	}
};

class CCC_FlushLog : public IConsole_Command {
public:
	CCC_FlushLog(LPCSTR N) : IConsole_Command(N)  { bEmptyArgsHandled = true; };
	virtual void Execute(LPCSTR /**args/**/) {
		FlushLog();
		Msg		("* Log file has been saved successfully!");
	}
};

class CCC_ClearLog : public IConsole_Command {
public:
	CCC_ClearLog(LPCSTR N) : IConsole_Command(N)  { bEmptyArgsHandled = true; };
	virtual void Execute(LPCSTR) {
		LogFile->clear	();
		FlushLog				();
		Msg						("* Log file has been cleaned successfully!");
	}
};

class CCC_FloatBlock : public CCC_Float {
public:
	CCC_FloatBlock(LPCSTR N, float* V, float _min=0, float _max=1) :
	  CCC_Float(N,V,_min,_max)
	  {};

	  virtual void	Execute	(LPCSTR args)
	  {
#ifdef _DEBUG
		  CCC_Float::Execute(args);
#else
		  if (!g_pGameLevel)	CCC_Float::Execute(args);
		  else 					Msg ("! Command disabled for this type of game");
#endif
	  }
};

#include "../../xrAICore/game_graph.h"
struct CCC_JumpToLevel : public IConsole_Command {
	CCC_JumpToLevel(LPCSTR N) : IConsole_Command(N)  {};

	virtual void Execute(LPCSTR level)
	{
		if ( !ai().get_alife() )
		{
			Msg				("! ALife simulator is needed to perform specified command!");
			return;
		}

		GameGraph::LEVEL_MAP::const_iterator	I = ai().game_graph().header().levels().begin();
		GameGraph::LEVEL_MAP::const_iterator	E = ai().game_graph().header().levels().end();
		for ( ; I != E; ++I )
			if ( !xr_strcmp((*I).second.name(),level) )
			{
				ai().alife().jump_to_level(level);
				return;
			}
		Msg							("! There is no level \"%s\" in the game graph!",level);
	}

	virtual void	Save	(IWriter *F)	{};
	virtual void	fill_tips(vecTips& tips, u32 mode)
	{
		if ( !ai().get_alife() )
		{
			Msg				("! ALife simulator is needed to perform specified command!");
			return;
		}

		GameGraph::LEVEL_MAP::const_iterator	itb = ai().game_graph().header().levels().begin();
		GameGraph::LEVEL_MAP::const_iterator	ite = ai().game_graph().header().levels().end();
		for ( ; itb != ite; ++itb )
		{
			tips.push_back( (*itb).second.name() );
		}
	}

};

class CCC_Script : public IConsole_Command {
public:
	CCC_Script(LPCSTR N) : IConsole_Command(N)  { bEmptyArgsHandled = false; };
	virtual void Execute( LPCSTR args )
	{
		if ( !xr_strlen(args) )
		{
			Log("* Specify script name!");
		}
		else
		{
			// rescan pathes
			FS_Path* P = FS.get_path("$game_scripts$");
			P->m_Flags.set	(FS_Path::flNeedRescan,TRUE);
			FS.rescan_pathes();
			// run script
			if (ai().script_engine().script_process(ScriptEngine::eScriptProcessorLevel))
				ai().script_engine().script_process(ScriptEngine::eScriptProcessorLevel)->add_script(args,false,true);
		}
	}

	virtual void Status( TStatus& S )
	{
		xr_strcpy( S, "<script_name> (Specify script name!)" );
	}
	virtual void Save( IWriter* F ) {}

	virtual void fill_tips( vecTips& tips, u32 mode )
	{
		get_files_list( tips, "$game_scripts$", ".script" );
	}

};

class CCC_ScriptCommand : public IConsole_Command {
public:
	CCC_ScriptCommand	(LPCSTR N) : IConsole_Command(N)  { bEmptyArgsHandled = false; };
	
	virtual void	Execute				(LPCSTR args)
	{
		if (!xr_strlen(args))
			Log("* Specify string to run!");
		else {
			if (ai().script_engine().script_process(ScriptEngine::eScriptProcessorLevel)) {
				ai().script_engine().script_process(ScriptEngine::eScriptProcessorLevel)->add_script(args,true,true);
				return;
			}

			string4096 S;
			shared_str m_script_name = "console command";
			xr_sprintf(S,"%s\n",args);
			int l_iErrorCode = luaL_loadbuffer(ai().script_engine().lua(),S,xr_strlen(S),"@console_command");
			if (!l_iErrorCode) 
			{
				l_iErrorCode = lua_pcall(ai().script_engine().lua(),0,0,0);
				if (l_iErrorCode) 
				{
					ai().script_engine().print_output	(ai().script_engine().lua(),*m_script_name,l_iErrorCode);
					return;
				}
			}

			ai().script_engine().print_output	(ai().script_engine().lua(),*m_script_name,l_iErrorCode);
		}
	}

	virtual void Status( TStatus& S )
	{
		xr_strcpy( S, "<script_name.function()> (Specify script and function name!)" );
	}
	virtual void Save( IWriter* F ) {}

	virtual void fill_tips( vecTips& tips, u32 mode )
	{
		if ( mode == 1 )
		{
			get_files_list( tips, "$game_scripts$", ".script" );
			return;
		}

		IConsole_Command::fill_tips( tips, mode );
	}
};

class CCC_TimeFactor : public IConsole_Command {
public:
					CCC_TimeFactor	(LPCSTR N) : IConsole_Command(N) {}
	virtual void	Execute			(LPCSTR args)
	{
		float				time_factor = (float)atof(args);
		clamp				(time_factor,EPS,1000.f);
		Device.time_factor	(time_factor);
		    psSpeedOfSound	= time_factor;
	}
	virtual void	Status			(TStatus &S)
	{
		xr_sprintf	(S,sizeof(S),"%f",Device.time_factor());
	}

	virtual void	Info	(TInfo& I)
	{
		xr_strcpy				(I,"[0.001 - 1000.0]");
	}
	
	virtual void	fill_tips(vecTips& tips, u32 mode)
	{
		TStatus  str;
		xr_sprintf( str, sizeof(str), "%3.3f  (current)  [0.001 - 1000.0]", Device.time_factor() );
		tips.emplace_back( str );
		IConsole_Command::fill_tips( tips, mode );
	}
};


class CCC_MainMenu : public IConsole_Command {
public:
	CCC_MainMenu(LPCSTR N) : IConsole_Command(N)  { bEmptyArgsHandled = true; };
	virtual void Execute(LPCSTR args) {

		bool bWhatToDo = TRUE;
		if( 0==xr_strlen(args) ){
			bWhatToDo = !MainMenu()->IsActive();
		};

		if( EQ(args,"on")||EQ(args,"1") )
			bWhatToDo = TRUE;

		if( EQ(args,"off")||EQ(args,"0") )
			bWhatToDo = FALSE;

		MainMenu()->Activate( bWhatToDo );
	}
};

struct CCC_StartTimeSingle : public IConsole_Command {
	CCC_StartTimeSingle(LPCSTR N) : IConsole_Command(N) {};
	virtual void	Execute	(LPCSTR args)
	{
		u32 year = 1, month = 1, day = 1, hours = 0, mins = 0, secs = 0, milisecs = 0;
		sscanf				(args,"%d.%d.%d %d:%d:%d.%d",&year,&month,&day,&hours,&mins,&secs,&milisecs);
		year				= std::max(year,(u32)1);
		month				= std::max(month,(u32)1);
		day					= std::max(day,(u32)1);
		g_qwStartGameTime	= generate_time	(year,month,day,hours,mins,secs,milisecs);

		if (!g_pGameLevel)
			return;

		if (!Level().Server)
			return;

		if (!Level().Server->game)
			return;

		Level().SetGameTimeFactor(g_qwStartGameTime,g_fTimeFactor);
	}

	virtual void	Status	(TStatus& S)
	{
		u32 year = 1, month = 1, day = 1, hours = 0, mins = 0, secs = 0, milisecs = 0;
		split_time	(g_qwStartGameTime, year, month, day, hours, mins, secs, milisecs);
		xr_sprintf		(S,"%d.%d.%d %d:%d:%d.%d",year,month,day,hours,mins,secs,milisecs);
	}
};

struct CCC_TimeFactorSingle : public CCC_Float {
	CCC_TimeFactorSingle(LPCSTR N, float* V, float _min=0.f, float _max=1.f) : CCC_Float(N,V,_min,_max) {};

	virtual void	Execute	(LPCSTR args)
	{
		CCC_Float::Execute	(args);
		
		if (!g_pGameLevel)
			return;

		if (!Level().Server)
			return;

		if (!Level().Server->game)
			return;

		Level().SetGameTimeFactor(g_fTimeFactor);
	}

};

class CCC_TuneAttachableItem : public IConsole_Command
{
public:
	CCC_TuneAttachableItem(LPCSTR N) :IConsole_Command(N) {};
	virtual void Execute(LPCSTR args)
	{
		if (!g_pGameLevel)
			return;

		if (CAttachableItem::m_dbgItem)
		{
			CAttachableItem::m_dbgItem = nullptr;	
			Msg("CCC_TuneAttachableItem switched to off");
			return;
		};

		CObject* obj			= Level().CurrentViewEntity();	VERIFY(obj);
		shared_str ssss			= args;

		CAttachmentOwner* owner = smart_cast<CAttachmentOwner*>(obj);
		CAttachableItem* itm	= owner->attachedItem(ssss);
		if(itm)
		{
			CAttachableItem::m_dbgItem = itm;
		}else
		{
			CInventoryOwner* iowner = smart_cast<CInventoryOwner*>(obj);
			PIItem active_item = iowner->m_inventory->ActiveItem();
			if(active_item && active_item->object().cNameSect()==ssss )
				CAttachableItem::m_dbgItem = active_item->cast_attachable_item();
		}

		if(CAttachableItem::m_dbgItem)
			Msg("CCC_TuneAttachableItem switched to ON for [%s]",args);
		else
			Msg("CCC_TuneAttachableItem cannot find attached item [%s]",args);
	}

	virtual void	Info	(TInfo& I)
	{	
		xr_sprintf(I,"allows to change bind rotation and position offsets for attached item, <section_name> given as arguments");
	}
};

// Change weather immediately
class CCC_SetWeather : public IConsole_Command
{
public:
    CCC_SetWeather(LPCSTR N) : IConsole_Command(N) {};
    virtual void Execute(LPCSTR args)
    {
        if (!xr_strlen(args))
            return;

        if (!Device.editor())
            g_pGamePersistent->Environment().SetWeather(args, true);
    }

	virtual void fill_tips(vecTips& tips, u32 mode)
	{
		get_files_list(tips, "$game_weathers$", ".ltx");
	}
};
