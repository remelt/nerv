#include <vector>
#include <utility>

#include "shared.hpp"

void c_changer::run() {
	auto game_client = g_interfaces->m_network_client->GetNetworkGameClient();
	if (!game_client)
		return;

	if (!g_ctx->m_local_pawn || !g_interfaces->m_network_client || !g_item_schema->is_initialized())
		return;

	auto* local_pawn = reinterpret_cast<c_cs_player_pawn*>(g_ctx->m_local_pawn);
	if (!valid_ptr(local_pawn) || local_pawn->m_health() <= 0)
		return;

	auto observer_service = local_pawn->m_observer_services();
	if (!observer_service || !valid_ptr(observer_service) || observer_service->m_observer_mode() == 0)
		return;

	auto observer_target = g_interfaces->m_game_resource->pGameEntitySystem->Get<c_cs_player_pawn>(observer_service->m_observer_target());
	if (!observer_target || !valid_ptr(observer_target) || observer_target->m_health() <= 0)
		return;

	g_ctx->m_observer_target = observer_target;

	engine_data cur_engine_data;
	cur_engine_data.m_spawn_time = observer_target->m_last_spawn_time_index();
	cur_engine_data.m_team = observer_target->m_team_num();
	cur_engine_data.m_demo_tick = g_interfaces->m_engine2_to_client->GetCurrentDemoTick();

	auto* controller = observer_target->get_controller();

	g_agent_changer->run(observer_target, cur_engine_data);

	if (controller) {
		cur_engine_data.m_steam_id = controller->m_steam_id();

		g_glove_changer->run(observer_target, cur_engine_data);
		g_skin_changer->run(observer_target, cur_engine_data);
	}

	general_engine_backup = cur_engine_data;
}

static char* tier0_dup(const char* s)
{
	auto* p = (char*)g_interfaces->m_mem_alloc->alloc(strlen(s) + 1);
	memcpy(p, s, strlen(s) + 1);
	return p;
}

inline uint32_t to_hex(float Color[3])
{
	return 0xFF000000 + ((uint32_t)(Color[2] * 255) << 16) | ((uint32_t)(Color[1] * 255) << 8) | (uint32_t)(Color[0] * 255);
}

void process_weapon_color(const uint16_t& def_index, std::vector<uint32_t>& buffer) {

	int config_index = c_config::skin_changer_t::get_config_index(def_index);
	if (config_index == 0)
		return;

	auto& skin = g_cfg->skin_changer.weapon_skins[config_index];
	if (skin.paint_kit == 0 || !skin.m_custom_color)
		return;

	buffer = {
		to_hex(skin.m_color1),
		to_hex(skin.m_color2),
		to_hex(skin.m_color3),
		to_hex(skin.m_color4)
	};
}

void process_knife_color(const uint16_t& def_index, std::vector<uint32_t>& buffer) {

	const uint16_t selected_knife = g_item_schema->knives[g_cfg->knife_changer.m_knife].definition_index;
	if (selected_knife == 0)
		return;

	if (g_cfg->knife_changer.m_paint_kit == 0 || !g_cfg->knife_changer.m_custom_color)
		return;

	buffer = {
		to_hex(g_cfg->knife_changer.m_color1),
		to_hex(g_cfg->knife_changer.m_color2),
		to_hex(g_cfg->knife_changer.m_color3),
		to_hex(g_cfg->knife_changer.m_color4)
	};
}

void c_changer::on_build_material_hook(void* vector, c_econ_item_view* m_item)
{
	if (!m_item)
		return;

	const uint16_t def_index = m_item->m_definition_index();
	const bool is_knife = (def_index == WEAPON_KNIFE || def_index == WEAPON_KNIFE_T || (def_index >= WEAPON_BAYONET && def_index <= WEAPON_KNIFEKUKRI));

	std::vector<uint32_t> buffer;

	if (is_knife)
		process_knife_color(def_index, buffer);
	else
		process_weapon_color(def_index, buffer);

	if (buffer.empty())
		return;

	for (int i = 0; i < 4; i++)
	{
		char name[16];
		snprintf(name, sizeof(name), "g_vColor%d", i);

		CompositeMaterialInputLooseVariable_t v{};
		v.m_strName = tier0_dup(name);
		v.m_nVariableType = LOOSE_VAR_COLOR4;
		v.m_cValueColor4 = buffer[i];

		//v125 = sub_180BF5380(v170, "g_nRandomSeed", v124);
		//g_append(&v136, v125);							<--
		static auto g_append = reinterpret_cast<void(__fastcall*)(void*, const CompositeMaterialInputLooseVariable_t*)>(g_opcodes->get_absolute_address(g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "E8 ? ? ? ? 0F 28 B4 24 ? ? ? ? 4C 39 A5"), 0x1));
		g_append(vector, &v);
	}
}
