#pragma once

#include "../../modules/modules.hpp"
#include "../../../utils/utils.hpp"
#include "../../schema/schema.hpp"
#include "../../../sdk/typedefs/c_handle.hpp"

class c_base_entity;
class c_base_handle;

class i_entity_system {
public:
	template <class C = c_base_entity>
	C* get_base_entity(int index) {
		static auto get_client_entity = reinterpret_cast<C * (__fastcall*)(i_entity_system*, int)>(
			g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "4C 8D 49 ? 81 FA")
			);

		if (!get_client_entity)
			return nullptr;
		return get_client_entity(this, index);
	}

	/// GetClientEntity
	template <typename T = c_base_entity>
	T* Get(int nIndex)
	{
		return reinterpret_cast<T*>(this->GetEntityByIndex(nIndex));
	}

	/// GetClientEntityFromHandle
	template <typename T = c_base_entity>
	T* Get(c_base_handle hHandle)
	{
		if (!hHandle.is_valid())
			return nullptr;

		return reinterpret_cast<T*>(this->GetEntityByIndex(hHandle.get_entry_index()));
	}

	void* get_local_pawn() {

		static auto fn = reinterpret_cast<void* (__fastcall*)(int)>(
			g_opcodes->get_absolute_address(g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "E8 ? ? ? ? 48 85 C0 0F 84 ? ? ? ? 44 8B 45 17 48 8B 38 4C 8B 4D 0F 8B 75 0B"), 0x1)
			);
		if (fn)
			return fn(-1);
		return nullptr;
	}

	void* get_local_controller() {

		static auto fn = reinterpret_cast<void* (__fastcall*)(int)>(
			g_opcodes->get_absolute_address(g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "E8 ? ? ? ? 48 85 C0 0F 84 ? ? ? ? 44 8B 45 17 48 8B 38 4C 8B 55"), 0x1)
			);
		if (fn)
			return fn(-1);
		return nullptr;
	}

	int get_highest_entity_index() {
		return *reinterpret_cast<int*>(reinterpret_cast<std::uintptr_t>(this) + 0x20A0);
	}

	void* GetEntityByIndex(int nIndex)
	{
		//@ida: #STR: "no such entity %d\n,", "\tEntity param \"%s\" returned NULL.\n", "particles/generic_fx/fizz.vpcf" | or find "cl_showents" cvar -> look for callback
		//	do { pEntity = GetBaseEntityByIndex(g_pGameEntitySystem, nCurrentIndex); ... }
		using fnGetBaseEntity = void* (__thiscall*)(void*, int);
		static auto GetBaseEntity = reinterpret_cast<fnGetBaseEntity>(g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "4C 8D 49 10 81 FA ? ? ? ?"));
		return GetBaseEntity(this, nIndex);
	}
};
