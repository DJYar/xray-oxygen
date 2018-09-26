////////////////////////////////////////////////////////////////////////////
//	Module 		: script_engine_export.cpp
//	Created 	: 01.04.2004
//  Modified 	: 22.06.2004
//	Author		: Dmitriy Iassenev
//	Description : XRay Script Engine export
////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#pragma warning(push)
#pragma warning(disable: 4275)
#define SCRIPT_REGISTRATOR
#include "path_manager.h"
#include "../xrScripts/export/script_export_space.h"
#include "script_engine_export.h"
#pragma warning(pop)

#pragma optimize("s",on)
template <typename TList> struct Register
{
	static void _Register(lua_State *L)
	{
		imdexlib::ts_apply<imdexlib::ts_reverse_t<TList>>([&](const auto type) { registerOne(L, type); });
	}
private:
	template <typename T>
	static void registerOne(lua_State* L, imdexlib::identity<T>) {
		T::script_register(L);
	}
};

void export_classes	(lua_State *L)
{
	Register<script_type_list>::_Register(L);
}
