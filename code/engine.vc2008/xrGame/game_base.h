#pragma once
#include "../xrServerEntities/../xrAICore/alife_space.h"
#include "../xrServerEntities/gametype_chooser.h"
#pragma pack(push,1)
class NET_Packet;

#pragma pack(pop)

class game_GameState : public DLL_Pure
{
protected:
	EGameIDs						m_type;
	u16								m_phase;
	u32								m_start_time;

	u32								m_round_start_time;
	string64						m_round_start_time_str;
protected:
	virtual		void				switch_Phase			(u32 new_phase);

public:
									game_GameState			();
	virtual							~game_GameState			()								{}
	IC			EGameIDs const&		Type					() const						{return m_type;};
				u16					Phase					() const						{return m_phase;};
				u32					StartTime				() const						{return m_start_time;};
	virtual		void				Create					(shared_str& options)			{};
	virtual		LPCSTR				type_name				() const						{return "base game";};
//for scripting enhancement
	static		CLASS_ID			getCLASS_ID				(LPCSTR game_type_name, bool bServer);

//moved from game_sv_base (time routines)
private:
	// scripts
	u64								m_qwStartProcessorTime;
	u64								m_qwStartGameTime;
	float							m_fTimeFactor;
	//-------------------------------------------------------
	u64								m_qwEStartProcessorTime;
	u64								m_qwEStartGameTime;
	float							m_fETimeFactor;
	//-------------------------------------------------------
public:

	virtual		u64					GetStartGameTime		();
	virtual		u64					GetGameTime				();	
	virtual		float				GetGameTimeFactor		();	
				void				SetGameTimeFactor		(u64 GameTime, const float fTimeFactor);
	virtual		void				SetGameTimeFactor		(const float fTimeFactor);
	

	virtual		u64					GetEnvironmentGameTime	();
	virtual		float				GetEnvironmentGameTimeFactor		();
	virtual		void				SetEnvironmentGameTimeFactor		(u64 GameTime, const float fTimeFactor);
	virtual		void				SetEnvironmentGameTimeFactor		(const float fTimeFactor);
};

enum EGamePhases
{
	GAME_PHASE_NONE = 0,
	GAME_PHASE_INPROGRESS,
	GAME_PHASE_FORCEDWORD = u32(-1)
};
