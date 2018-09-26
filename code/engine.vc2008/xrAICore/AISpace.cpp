#include "stdafx.h"
#include "ai_space_interface.h"
#include "game_graph.h"
#include "graph_engine.h"
#include "associative_vector.h"

AI_API CAI_SpaceInterface* g_ai_space = nullptr;

void CAI_SpaceInterface::init()
{
	m_graph_engine = xr_new<CGraphEngine>(1024);
}

const CGameLevelCrossTable &CAI_SpaceInterface::cross_table() const
{
	return (game_graph().cross_table());
}

const CGameLevelCrossTable *CAI_SpaceInterface::get_cross_table() const
{
	return (&game_graph().cross_table());
}

void CAI_SpaceInterface::game_graph(CGameGraph *game_graph)
{
	VERIFY(m_alife_simulator);
	VERIFY(game_graph);
	VERIFY(!m_game_graph);
	m_game_graph = game_graph;

	xr_delete(m_graph_engine);
	m_graph_engine = new CGraphEngine(game_graph->header().vertex_count());
}