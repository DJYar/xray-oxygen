////////////////////////////////////////////////////////////////////////////
//	Module 		: alife_spawn_registry.h
//	Created 	: 15.01.2003
//  Modified 	: 12.05.2004
//	Author		: Dmitriy Iassenev
//	Description : ALife spawn registry
////////////////////////////////////////////////////////////////////////////

#pragma once

#include "alife_spawn_registry_header.h"
#include "xrServer_Objects_ALife_Monsters.h"
#include "../xrAICore/../xrAICore/../xrAICore/game_graph.h"
#include "graph_abstract.h"
#include "server_entity_wrapper.h"
#include "ai_debug.h"

class CServerEntityWrapper;
class CGameGraph;

class CALifeSpawnRegistry : CRandom 
{
public:
	using ARTEFACT_SPAWNS = CGameGraph::LEVEL_POINT_VECTOR;
	using SPAWN_GRAPH = CGraphAbstractSerialize<CServerEntityWrapper*,float,ALife::_SPAWN_ID>;

public:
	typedef xr_vector<ALife::_SPAWN_ID>												SPAWN_IDS;
	typedef associative_vector<ALife::_SPAWN_STORY_ID,ALife::_OBJECT_ID>			SPAWN_STORY_IDS;

private:
	CALifeSpawnHeader						m_header;
	SPAWN_GRAPH								m_spawns;
	ARTEFACT_SPAWNS							m_artefact_spawn_positions;
	shared_str								m_spawn_name;
	SPAWN_IDS								m_spawn_roots;
	SPAWN_IDS								m_temp0;
	SPAWN_IDS								m_temp1;
	SPAWN_STORY_IDS							m_spawn_story_ids;

private:
	IReader									*m_file;
	IReader									*m_chunk;
	CGameGraph								*m_game_graph;

protected:
			void							save_updates				(IWriter &stream);
			void							load_updates				(IReader &stream);
			void							build_story_spawns			();
			void							build_root_spawns			();
			void							fill_new_spawns_single		(SPAWN_GRAPH::CVertex *vertex, xr_vector<ALife::_SPAWN_ID> &spawns, ALife::_TIME_ID game_time, xr_vector<ALife::_SPAWN_ID> &objects);
			void							fill_new_spawns				(SPAWN_GRAPH::CVertex *vertex, xr_vector<ALife::_SPAWN_ID> &spawns, ALife::_TIME_ID game_time, xr_vector<ALife::_SPAWN_ID> &objects);
	IC		void							process_spawns				(xr_vector<ALife::_SPAWN_ID> &spawns);
	IC		bool							redundant					(CSE_Abstract &abstract);
	IC		bool							new_spawn					(CSE_Abstract &abstract);
	IC		bool							enabled_spawn				(CSE_Abstract &abstract) const;
	IC		bool							count_limit					(CSE_Abstract &abstract) const;
	IC		bool							time_limit					(CSE_Abstract &abstract, ALife::_TIME_ID game_time) const;
	IC		bool							spawned_item				(CSE_Abstract &abstract, xr_vector<ALife::_SPAWN_ID> &objects) const;
	IC		bool							spawned_item				(SPAWN_GRAPH::CVertex *vertex, xr_vector<ALife::_SPAWN_ID> &objects);
	IC		bool							object_existance_limit		(CSE_Abstract &abstract, xr_vector<ALife::_SPAWN_ID> &objects) const;
	IC		bool							can_spawn					(CSE_Abstract &abstract, ALife::_TIME_ID game_time, xr_vector<ALife::_SPAWN_ID> &objects) const;

public:
											CALifeSpawnRegistry			(LPCSTR section);
	virtual									~CALifeSpawnRegistry		();
	virtual void							load						(IReader &file_stream, xrGUID *save_guid = 0);
	virtual void							save						(IWriter &memory_stream);
			void							load						(IReader &file_stream, LPCSTR game_name);
			void							load						(LPCSTR spawn_name);
			void							fill_new_spawns				(xr_vector<ALife::_SPAWN_ID> &spawns, ALife::_TIME_ID game_time, xr_vector<ALife::_SPAWN_ID> &objects);
	IC		const CALifeSpawnHeader			&header						() const;
	IC		const SPAWN_GRAPH				&spawns						() const;
	IC		void							assign_artefact_position	(CSE_ALifeAnomalousZone	*anomaly, CSE_ALifeDynamicObject *object) const;
	IC		const ALife::_SPAWN_ID			&spawn_id					(const ALife::_SPAWN_STORY_ID &spawn_story_id) const;
	shared_str const&						get_spawn_name				() const {return m_spawn_name;}
	IReader*								get_spawn_file				() const {return m_file;}
};

IC const CALifeSpawnHeader &CALifeSpawnRegistry::header() const
{
	return (m_header);
}

IC void CALifeSpawnRegistry::assign_artefact_position(CSE_ALifeAnomalousZone	*anomaly, CSE_ALifeDynamicObject *object) const
{
	object->m_tGraphID = anomaly->m_tGraphID;
	VERIFY3(anomaly->m_artefact_spawn_count, "Anomaly is outside of the AI-map but is used for artefact generation : ", anomaly->name_replace());
	u32 index = anomaly->m_artefact_position_offset + anomaly->randI(anomaly->m_artefact_spawn_count);
	object->o_Position = m_artefact_spawn_positions[index].level_point();
	object->m_tNodeID = m_artefact_spawn_positions[index].level_vertex_id();
	object->m_fDistance = m_artefact_spawn_positions[index].distance();
}

IC const CALifeSpawnRegistry::SPAWN_GRAPH &CALifeSpawnRegistry::spawns() const
{
	return (m_spawns);
}

IC void CALifeSpawnRegistry::process_spawns(SPAWN_IDS &spawns)
{
	std::sort(spawns.begin(), spawns.end());
	spawns.erase(std::unique(spawns.begin(), spawns.end()), spawns.end());
}

IC const ALife::_SPAWN_ID &CALifeSpawnRegistry::spawn_id(const ALife::_SPAWN_STORY_ID &spawn_story_id) const
{
	auto spawn_find = m_spawn_story_ids.find(spawn_story_id);
	VERIFY2(spawn_find != m_spawn_story_ids.end(), "Spawn story id cannot be found");
	return (*spawn_find).second;
}
