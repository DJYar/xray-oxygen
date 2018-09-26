#pragma once
#include "linker.h"
// Forward
class CGameGraph;
class CGameLevelCrossTable;
class CGraphEngine;

class AI_API CAI_SpaceInterface
{
protected:
	CGameGraph*   m_game_graph;
	CGraphEngine* m_graph_engine;
	
public:
	virtual ~CAI_SpaceInterface() = default;
	virtual void init();

public:
	inline virtual CGraphEngine& graph_engine	() const noexcept { return (*m_graph_engine); }
	inline virtual CGameGraph &  game_graph		() const noexcept { return (*m_game_graph); }
	inline virtual CGameGraph*   get_game_graph	() const noexcept { return (m_game_graph); }
	       virtual void          game_graph		(CGameGraph * game_graph);

	virtual const CGameLevelCrossTable& cross_table		() const;
	virtual const CGameLevelCrossTable* get_cross_table	() const;
};

extern AI_API CAI_SpaceInterface *g_ai_space;