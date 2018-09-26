////////////////////////////////////////////////////////////////////////////
//	Module 		: ai_space.h
//	Created 	: 12.11.2003
//  Modified 	: 12.11.2003
//	Author		: Dmitriy Iassenev
//	Description : AI space class
////////////////////////////////////////////////////////////////////////////
#pragma once
#include "../xrAICore/ai_space_interface.h"

class CGameLevelCrossTable;
class CLevelGraph;
class CGraphEngine;
class CEF_Storage;
class CALifeSimulator;
class CCoverManager;
class CScriptEngine;
class CPatrolPathStorage;
class moving_objects;

namespace doors 
{
	class manager;
} // namespace doors

class CAI_Space: public CAI_SpaceInterface
{
	using inherited = CAI_SpaceInterface;

private:
	friend class CALifeSimulator;
	friend class CALifeGraphRegistry;
	friend class CALifeSpawnRegistry;
	friend class CALifeSpawnRegistry;
	friend class CLevel;

private:
	CLevelGraph							*m_level_graph;
	CEF_Storage							*m_ef_storage;
	CALifeSimulator						*m_alife_simulator;
	CCoverManager						*m_cover_manager;
	CScriptEngine						*m_script_engine;
	CPatrolPathStorage					*m_patrol_path_storage;
	moving_objects						*m_moving_objects;
	doors::manager						*m_doors_manager;

private:
			void						load					(LPCSTR level_name);
			void						unload					(bool reload = false);
			void						patrol_path_storage_raw	(IReader &stream);
			void						patrol_path_storage		(IReader &stream);
			void						set_alife				(CALifeSimulator *alife_simulator);

public:
										CAI_Space				();
	virtual								~CAI_Space				();
	virtual void						init					();
	IC		CLevelGraph					&level_graph			() const;
	IC		const CLevelGraph			*get_level_graph		() const;
	IC		const CPatrolPathStorage	&patrol_paths			() const;
	IC		CEF_Storage					&ef_storage				() const;
	IC		const CALifeSimulator		&alife					() const;
	IC		const CALifeSimulator		*get_alife				() const;
	IC		const CCoverManager			&cover_manager			() const;
	IC		CScriptEngine				&script_engine			() const;
	IC		moving_objects				&moving_objects			() const;
	IC		doors::manager&				doors					() const;

#ifdef DEBUG
			void						validate				(const u32			level_id) const;
#endif
};

IC	CAI_Space	&ai	();

#include "ai_space_inline.h"