#include "../../main.hpp"
#include "vtables/iswapchaindx11.h"

void* get_entity_by_index(int index) {
	if (!g_interfaces->m_entity_system)
		return nullptr;
	return g_interfaces->m_entity_system->get_base_entity(index);
}

void c_interfaces::initialize()
{
	HMODULE tier0 = GetModuleHandleA("tier0.dll");

	m_mem_alloc = *reinterpret_cast<i_mem_alloc**>(g_opcodes->export_fn((unsigned __int64)tier0, ("g_pMemAlloc")));
	CHECK(xorstr_("IMemAlloc"), m_mem_alloc);

	fn_get_game_directory = *reinterpret_cast<const char*(*)()>(g_opcodes->export_fn((unsigned __int64)tier0, ("Plat_GetGameDirectory")));
	CHECK(xorstr_("Game Directory"), fn_get_game_directory);

	m_csgo_input = *reinterpret_cast<i_csgo_input**>(g_opcodes->scan_absolute(g_modules->m_modules.client_dll.get_name(), xorstr_("48 8B 0D ? ? ? ? 4C 8B C6 8B 10 E8"), 0x3));
	CHECK(xorstr_("CSGO Input"), m_csgo_input);

	m_entity_system = *reinterpret_cast<i_entity_system**>(g_opcodes->scan_absolute(g_modules->m_modules.client_dll.get_name(), xorstr_("48 8B 0D ? ? ? ? 48 89 7C 24 ? 8B FA C1 EB"), 0x3));
	CHECK(xorstr_("Entity System"), m_entity_system);

	m_schema_system = get_interface<i_schema_system>(&g_modules->m_modules.schemasystem_dll, xorstr_("SchemaSystem_001"));
	CHECK(xorstr_("Schema System"), m_schema_system);

	m_input_system = get_interface(&g_modules->m_modules.input_system, xorstr_("InputSystemVersion001"));
	CHECK(xorstr_("Input System"), m_input_system);

	m_source2_client = get_interface<c_source2_client>(&g_modules->m_modules.client_dll, xorstr_("Source2Client002"));
	CHECK(xorstr_("Source2Client"), m_source2_client);

	m_engine2_to_client = get_interface<ISource2EngineToClient>(&g_modules->m_modules.engine_dll, xorstr_("Source2EngineToClient001"));
	CHECK(xorstr_("Source2EngineToClient"), m_engine2_to_client);

	m_file_system = get_interface<i_file_system>(&g_modules->m_modules.filesystem_stdio, xorstr_("VFileSystem017"));
	CHECK(xorstr_("FileSystem"), m_file_system);

	m_game_resource = get_interface<i_game_resource_service>(&g_modules->m_modules.engine_dll, xorstr_("GameResourceServiceClientV001"));
	CHECK(xorstr_("Game Resource"), m_game_resource);

	m_network_client = get_interface<i_network_client_service>(&g_modules->m_modules.engine_dll, xorstr_("NetworkClientService_001"));
	CHECK(xorstr_("Network Client"), m_network_client);

	m_panorama = get_interface<i_panorama>(&g_modules->m_modules.panorama_dll, xorstr_("PanoramaUIEngine001"));
	CHECK(xorstr_("Panorama"), m_panorama);

	m_resource_system = get_interface<i_resource_system>(&g_modules->m_modules.resourcesystem_dll, xorstr_("ResourceSystem013"));
	CHECK(xorstr_("Resource Client"), m_resource_system);

	//m_prediction = get_interface<c_prediction>(&g_modules->m_modules.client_dll, xorstr_("Source2ClientPrediction001"));
	//CHECK(xorstr_("Client Prediction"), m_prediction);
	
	m_material_system2 = get_interface<i_material_system2>(&g_modules->m_modules.materialsystem_dll, xorstr_("VMaterialSystem2_001"));
	CHECK(xorstr_("Material System"), m_material_system2);

	m_network_string_table = *reinterpret_cast<i_network_string_table**>(g_opcodes->scan_absolute(g_modules->m_modules.client_dll.get_name(), xorstr_("48 8B 0D ? ? ? ? 48 8B 01 4C 8B 40 58 8B 45 E4"), 0x3));
	CHECK(xorstr_("Network String Table"), m_network_string_table);

	m_swap_chain = **reinterpret_cast<ISwapChainDx11***>(g_opcodes->scan_absolute(g_modules->m_modules.rendersystem_dll.get_name(), xorstr_("48 89 2D ? ? ? ? 66 0F 7F 05 ? ? ? ? FF 15"), 0x3));
	CHECK(xorstr_("Swap Chain"), m_swap_chain);
}
