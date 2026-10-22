#pragma once
#include "../modules/modules.hpp"
#include "vtables/i_csgo_input.hpp"
#include "vtables/i_entity_system.hpp"
#include "vtables/i_schema_system.hpp"
#include "vtables/i_econ_item_system.hpp"
#include "vtables/i_file_system.hpp"
#include "vtables/i_game_resource_service.hpp"
#include "vtables/i_network_game_client.hpp"
#include "vtables/i_resource_system.hpp"
#include "vtables/iswapchaindx11.h"
#include "vtables/i_mem_alloc.hpp"
#include "vtables/i_panorama.h"
#include "vtables/i_source2_engine_to_client.h"
#include "vtables/i_prediction.h"
#include "vtables/i_network_string_table.h"
#include "vtables/i_material.h"

template <typename type_t = void*>
inline type_t* get_interface(c_dll* dll, const char* name)
{
	const HINSTANCE module_handle = GetModuleHandle(dll->get_name());
	if (!module_handle)
		return nullptr;

	using create_interface_t = type_t * (*)(const char*, int*);
	const create_interface_t create_interface = reinterpret_cast<create_interface_t>(GetProcAddress(module_handle, "CreateInterface"));
	if (!create_interface)
		return nullptr;

	return create_interface(name, nullptr);
};

class c_interfaces
{
public:
	i_mem_alloc* m_mem_alloc;
	i_csgo_input* m_csgo_input;
	i_entity_system* m_entity_system;
	i_schema_system* m_schema_system;
	void* m_input_system;
	c_source2_client* m_source2_client;
	i_file_system* m_file_system;
	i_game_resource_service* m_game_resource;
	i_network_client_service* m_network_client;
	i_resource_system* m_resource_system;
	i_panorama* m_panorama;
	ISwapChainDx11* m_swap_chain;
	ISource2EngineToClient* m_engine2_to_client;
	c_prediction* m_prediction;
	i_network_string_table* m_network_string_table;
	i_material_system2* m_material_system2;

	void initialize();
};

inline const auto g_interfaces = std::make_unique<c_interfaces>();
