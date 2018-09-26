////////////////////////////////////////////////////////////////////////////
//	Module 		: script_world_state_script.cpp
//	Created 	: 19.03.2004
//  Modified 	: 19.03.2004
//	Author		: Dmitriy Iassenev
//	Description : Script world state script export
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "script_world_state.h"
#include "../xrAICore/condition_state.h"

#include "../../SDK/include/luabind/operator.hpp"

#include "luabind/luabind.hpp"
using namespace luabind;

#pragma optimize("s",on)
void CScriptWorldStateWrapper::script_register(lua_State *L)
{
	module(L)
	[
		class_<CScriptWorldState>("world_state")
			.def(								constructor<>())
			.def(								constructor<CScriptWorldState>())
			.def("add_property",				(void (CScriptWorldState::*)(const CScriptWorldState::COperatorCondition &))(&CScriptWorldState::add_condition))
			.def("remove_property",				(void (CScriptWorldState::*)(const CScriptWorldState::COperatorCondition::_condition_type &))(&CScriptWorldState::remove_condition))
			.def("clear",						&CScriptWorldState::clear)
			.def("includes",					&CScriptWorldState::includes)
			.def("property",					&CScriptWorldState::property)
			.def(const_self < CScriptWorldState())
			.def(const_self == CScriptWorldState())
	];
}

#include "script_world_property.h"
#include "../xrAICore/operator_abstract.h"


#pragma optimize("s",on)
void CScriptWorldPropertyWrapper::script_register(lua_State *L)
{
	module(L)
	[
		class_<CScriptWorldProperty>("world_property")
			.def(								constructor<CScriptWorldProperty::_condition_type, CScriptWorldProperty::_value_type>())
			.def("condition",					&CScriptWorldProperty::condition)
			.def("value",						&CScriptWorldProperty::value)
			.def(const_self < other<CScriptWorldProperty>())
			.def(const_self == other<CScriptWorldProperty>())
	];
}