////////////////////////////////////////////////////////////////////////////
//	Module 		: client_spawn_manager.h
//	Created 	: 08.10.2004
//  Modified 	: 08.10.2004
//	Author		: Dmitriy Iassenev
//	Description : Seniority hierarchy holder
////////////////////////////////////////////////////////////////////////////

#pragma once

#include "../xrAICore/alife_space.h"
#include "script_callback_ex.h"
#include "../xrScripts/export/script_export_space.h"

template <typename _return_type>
class CScriptCallbackEx;

class CGameObject;

class CClientSpawnManager {
public:
	using CALLBACK_TYPE = fastdelegate::FastDelegate1<CObject*>;

public:
	struct CSpawnCallback {
		CALLBACK_TYPE			m_object_callback;
		CScriptCallbackEx<void>	m_callback;
	};

public:
	using REQUESTED_REGISTRY = xr_map<ALife::_OBJECT_ID,CSpawnCallback>;
	using REQUEST_REGISTRY = xr_map<ALife::_OBJECT_ID,REQUESTED_REGISTRY>;

private:
	REQUEST_REGISTRY	m_registry;

protected:
			void		remove					(REQUESTED_REGISTRY &registry, ALife::_OBJECT_ID requesting_id, ALife::_OBJECT_ID requested_id, bool no_warning = false);
			void		merge_spawn_callbacks	(CSpawnCallback &new_callback, CSpawnCallback &old_callback);

public:
						CClientSpawnManager		() = default;
	virtual				~CClientSpawnManager	();
			void		add						(ALife::_OBJECT_ID	requesting_id, ALife::_OBJECT_ID requested_id, const luabind::functor<void> &functor, const luabind::object &object);
			void		add						(ALife::_OBJECT_ID	requesting_id, ALife::_OBJECT_ID requested_id, const luabind::functor<void> &lua_function);
			void		add						(ALife::_OBJECT_ID	requesting_id, ALife::_OBJECT_ID requested_id, const CALLBACK_TYPE &object_callback);
			void		add						(ALife::_OBJECT_ID	requesting_id, ALife::_OBJECT_ID requested_id, CSpawnCallback &callback);
			void		remove					(ALife::_OBJECT_ID	requesting_id, ALife::_OBJECT_ID requested_id);
			void		clear					(ALife::_OBJECT_ID	requested_id);
			void		clear					();
			void		callback				(CObject			*object);
			void		callback				(CSpawnCallback		&spawn_callback, CObject *object);
			const CSpawnCallback *callback		(ALife::_OBJECT_ID	requesting_id, ALife::_OBJECT_ID requested_id) const;
#ifdef DEBUG
	IC		const REQUEST_REGISTRY &registry	() const { return (m_registry); }
			void		dump					() const;
			void		dump					(ALife::_OBJECT_ID	requesting_id) const;
#endif // DEBUG
	DECLARE_SCRIPT_REGISTER_FUNCTION
};
add_to_type_list(CClientSpawnManager)
#undef script_type_list
#define script_type_list save_type_list(CClientSpawnManager)
