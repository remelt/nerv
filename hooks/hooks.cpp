#include "../main.hpp"
#include "../directx/directx.hpp"
#include "../menu/menu.hpp"
#include "../features/shared/item_schema.hpp"
#include "../features/shared/shared.hpp"
#include "../features//agent_changer/agent_changer.hpp"
#include "../valve/interfaces/vtables/i_csgo_input.hpp"
#include "../valve/interfaces/vtables/i_game_event.hpp"
#include "../valve/classes/c_cs_player_pawn.hpp"
#include "../sdk/includes/hash.hpp"
#include "../valve/classes/i_material_system.hpp"

using namespace hooks;

namespace event_hashes {
	constexpr uint32_t round_start = fnv1a::hash_32("round_start");
	constexpr uint32_t player_death = fnv1a::hash_32("player_death");
	constexpr uint32_t item_purchase = fnv1a::hash_32("item_purchase");
}

inline uint32_t hash_event_name(const char* str) {
	uint32_t hash = fnv1a::val_32_const;
	while (*str) {
		hash = (hash ^ static_cast<uint8_t>(*str)) * fnv1a::prime_32_const;
		str++;
	}
	return hash;
}

namespace knife_db {
	struct entry { std::uint16_t def; const char* full; };
	inline constexpr entry kSkinKnives[] = {
		{500, "weapon_bayonet"},                {503, "weapon_knife_css"},
		{505, "weapon_knife_flip"},             {506, "weapon_knife_gut"},
		{507, "weapon_knife_karambit"},         {508, "weapon_knife_m9_bayonet"},
		{509, "weapon_knife_tactical"},         {512, "weapon_knife_falchion"},
		{514, "weapon_knife_survival_bowie"},   {515, "weapon_knife_butterfly"},
		{516, "weapon_knife_push"},             {517, "weapon_knife_cord"},
		{518, "weapon_knife_canis"},            {519, "weapon_knife_ursus"},
		{520, "weapon_knife_gypsy_jackknife"},  {521, "weapon_knife_outdoor"},
		{522, "weapon_knife_stiletto"},         {523, "weapon_knife_widowmaker"},
		{525, "weapon_knife_skeleton"},         {526, "weapon_knife_kukri"},
	};

	inline const char* lookup(std::uint16_t def) {
		for (auto& e : kSkinKnives)
			if (e.def == def) return e.full;
		return nullptr;
	}

	inline bool matches(const char* name) {
		if (!name || !*name) return false;
		if (std::strncmp(name, "weapon_", 7) == 0) name += 7;
		if (std::strcmp(name, "knife") == 0 || std::strcmp(name, "knife_t") == 0)
			return true;
		for (auto& e : kSkinKnives)
			if (std::strcmp(name, e.full + 7) == 0) return true;
		return false;
	}
}

static const char* get_knife_weapon_name(int knife_index) {
	if (knife_index <= 0
		|| !g_item_schema->is_initialized()
		|| knife_index >= (int)g_item_schema->knives.size())
		return nullptr;
	return knife_db::lookup(g_item_schema->knives[knife_index].definition_index);
}

bool c_hooks::initialize() {
	MH_Initialize();

	mouse_input_enabled::m_mouse_input_enabled.hook(vmt::get_v_method(g_interfaces->m_csgo_input, 23), mouse_input_enabled::hk_mouse_input_enabled);
	enable_cursor::m_enable_cursor.hook(vmt::get_v_method(g_interfaces->m_input_system, 76), enable_cursor::hk_enable_cursor);

	frame_stage_notify::m_frame_stage_notify.hook(
		g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "48 89 5C 24 ? 48 89 6C 24 ? 57 48 83 EC 40 48 8B F9 33 ED"),
		frame_stage_notify::hk_frame_stage_notify
	);

	{
		i_game_event::get_name = reinterpret_cast<i_game_event::GetNameFn>(g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "8B 41 14 0F BA E0 1E 73 05 48 8D 41 18 C3"));
		i_game_event::get_string = reinterpret_cast<i_game_event::GetStringFn>(g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "48 83 EC 38 8B 02 48 83 C1 58 89 44 24 20 8B 42 04 89 44 24 24 48 8B 42 08 48 8D 54 24 20 48 89 44 24 28 E8 ? ? ? ? 48 83 C4 38 C3 CC CC CC 33 C9"));
		i_game_event::set_string = reinterpret_cast<i_game_event::SetStringFn>(g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "48 83 EC 38 8B 02 48 83 C1 58 89 44 24 20 41 B1 1A"));
		i_game_event::get_player_controller = reinterpret_cast<i_game_event::GetPlayerControllerFn>(g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "48 83 EC 38 8B 02 4C 8D 44 24 20"));
	}

	//@ida #STR: "?SaveKV3AsJSON@@YA_NPEBVKeyValues3@@PEAVCUtlString@@1@Z"
	if (g_modules->m_modules.afxhooksource2_dll.get()) {
		//for original afxsource2
		unsigned char* address_fire_stage = g_opcodes->scan(g_modules->m_modules.afxhooksource2_dll.get_name(), "48 89 5C 24 10 48 89 6C 24 20");
		//for wangchudi's afxsource2
		if (!address_fire_stage)
			address_fire_stage = g_opcodes->scan(g_modules->m_modules.afxhooksource2_dll.get_name(), "48 89 5C 24 ? 48 89 ? 24 ? 48 89 54 24 ? 48 89 4C 24");

		fire_event_client_side::m_fire_event_client_side.hook(address_fire_stage,
			fire_event_client_side::hk_fire_event_client_side
		);
	}
	else {
		fire_event_client_side::m_fire_event_client_side.hook(
			g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "40 53 41 54 41 56 48 83 EC ? 4C 8B F2"),
			fire_event_client_side::hk_fire_event_client_side
		);
	}

	level_init::m_level_init.hook(
		g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "48 89 74 24 ? 57 48 83 EC ? 48 8B 0D ? ? ? ? 48 8B FA"),
		level_init::hk_level_init
	);

	//	Xrefs from, sub:
	//	#STR: "VolumeMaxs", "VolumeMins", "Priority", "LPVIndex", "`anonymous-namespace'::DynamicLockHelper<struct `anonymous, "`anonymous-namespace'::DynamicLockHelper<struct `anonymous, "LockDynamicConstantBuffer failed in %s\n", "Transform"
	smoke_voxel_draw::m_smoke_voxel_draw.hook(
		g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC 40 48 8B 9C 24 ? ? ? ? 4D 8B F8 48 8B FA 48 8B F1 45 33 C0 BA"),
		smoke_voxel_draw::hk_smoke_voxel_draw
	);
	// #STR: "cs_flash_frame_render_target_split_%d", "FlashbangOverlay", "CsgoForward"
	draw_flashbang_overlay::m_draw_flashbang_overlay.hook(
		g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "85 D2 0F 88 ?? ?? ?? ?? 48 89 4C 24 ?? 55 56 41 55 41 56 41 57 48 8D AC 24"),
		draw_flashbang_overlay::hk_draw_flashbang_overlay
	);
	// #STR: "FirstpersonLegsPass1", "FirstpersonLegsPass2", "CsgoForward", "Firstperson Legs", "FirstpersonLegsPrepass"
	firstperson_legs_prepass::m_firstperson_legs_prepass.hook(
		g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "40 55 53 56 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? F2 0F 10 42"),
		firstperson_legs_prepass::hk_firstperson_legs_prepass
	);

	if (g_directx->m_present_address)
		present::m_present.hook(g_directx->m_present_address, present::hk_present);
	if (g_directx->m_resize_buffers_address)
		resize_buffers::m_resize_buffers.hook(g_directx->m_resize_buffers_address, resize_buffers::hk_resize_buffers);
	if (g_directx->m_create_swap_chain_address)
		create_swap_chain::m_create_swap_chain.hook(g_directx->m_create_swap_chain_address, create_swap_chain::hk_create_swap_chain);

	// sub_180744CF0((__int64)v115, "econ_instance_vars", (int *)&v88);
	build_material::m_build_material.hook(
		g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "48 89 5C 24 ? 48 89 54 24 ? 56 57 41 54 41 55 41 57 48 83 EC 20 33 C0 C6 01 01 48 89"),
		build_material::hk_build_material
	);

	//@ida: scenesystem.dll -> class CLightBinnerGPU -> 3 index vtable
	draw_array_light::m_draw_array_light.hook(
		g_opcodes->scan(g_modules->m_modules.scenesystem_dll.get_name(), "48 89 54 24 ? 55 57 41 56 48 83 EC 50 48 8B FA 48 8B E9 BA ? ? ? ?"),
		draw_array_light::hk_draw_array_light
	);

	auto loc_1808E9E54 = g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "74 2A 48 8B 0D ? ? ? ? 48 8D 44 24 ? 48 89 7C 24 ? 4C 8D 4C 24 ? 4C 8D 45 A0 48 89 44 24 ? 48 8D 55 20");
	if (loc_1808E9E54) {
		DWORD oldProtect;
		VirtualProtect((PVOID)loc_1808E9E54, 0x2, PAGE_EXECUTE_READWRITE, &oldProtect);
		*(WORD*)loc_1808E9E54 = 0x2AEB;
		VirtualProtect((PVOID)loc_1808E9E54, 0x2, oldProtect, &oldProtect);
	}

	return true;
}

void c_hooks::destroy() {
	if (g_menu->m_opened) {
		ShowCursor(TRUE);

		auto original = enable_cursor::m_enable_cursor.get_original<decltype(&enable_cursor::hk_enable_cursor)>();
		if (original) {
			__try {
				original(g_interfaces->m_input_system, enable_cursor::m_enable_cursor_input);
			}
			__except (EXCEPTION_EXECUTE_HANDLER) {}
		}
	}

	g_menu->m_opened = false;

	present::m_present.unhook();
	resize_buffers::m_resize_buffers.unhook();
	create_swap_chain::m_create_swap_chain.unhook();

	Sleep(200);

	mouse_input_enabled::m_mouse_input_enabled.unhook();
	enable_cursor::m_enable_cursor.unhook();
	frame_stage_notify::m_frame_stage_notify.unhook();
	fire_event_client_side::m_fire_event_client_side.unhook();
	level_init::m_level_init.unhook();

	Sleep(100);

	g_directx->uninitialize();

	MH_Uninitialize();
}

bool __fastcall hooks::mouse_input_enabled::hk_mouse_input_enabled(void* ptr) {
	auto original = m_mouse_input_enabled.get_original<decltype(&hk_mouse_input_enabled)>();
	return g_menu->m_opened ? false : original(ptr);
}

void* __fastcall hooks::enable_cursor::hk_enable_cursor(void* rcx, bool active) {
	auto original = m_enable_cursor.get_original<decltype(&hk_enable_cursor)>();

	m_enable_cursor_input = active;

	return original(rcx, active);
}

void hooks::frame_stage_notify::hk_frame_stage_notify(void* source_to_client, int stage) {
	auto original = m_frame_stage_notify.get_original<decltype(&hk_frame_stage_notify)>();

	if (stage == 7) {

		g_ctx->m_local_controller = g_interfaces->m_entity_system->get_local_controller();
		g_ctx->m_local_pawn = g_interfaces->m_entity_system->get_local_pawn();

		if (g_ctx->m_local_pawn != nullptr && g_ctx->m_local_controller != nullptr) {
			g_changer->run();
		}
	}

	original(source_to_client, stage);
}

__int64 __fastcall hooks::level_init::hk_level_init(void* rcx, void* rdx) {
	auto original = m_level_init.get_original<decltype(&hk_level_init)>();

	if (g_cfg->knife_changer.m_enabled || g_cfg->skin_changer.m_enabled) {
		g_skin_changer->should_update = true;
	}

	if (g_cfg->glove_changer.m_enabled) {
		//g_glove_changer->should_update = true;
	}

	g_world->Reset();

	return original(rcx, rdx);
}

bool __fastcall hooks::fire_event_client_side::hk_fire_event_client_side(void* p_game_event_manager, void* p_game_event) {
	auto original = m_fire_event_client_side.get_original<decltype(&hk_fire_event_client_side)>();

	if (!p_game_event || !i_game_event::get_name)
		return original(p_game_event_manager, p_game_event);

	const char* event_name = i_game_event::get_name(p_game_event);
	if (!event_name)
		return original(p_game_event_manager, p_game_event);

	const uint32_t event_hash = hash_event_name(event_name);

	if ((event_hash == event_hashes::round_start || event_hash == event_hashes::item_purchase))
	{
		//g_skin_changer->should_update |= g_cfg->knife_changer.m_enabled || g_cfg->skin_changer.m_enabled;
		//g_glove_changer->should_update |= g_cfg->glove_changer.m_enabled;
		return original(p_game_event_manager, p_game_event);
	}

	if (event_hash != event_hashes::player_death || !g_cfg->knife_changer.m_enabled)
		return original(p_game_event_manager, p_game_event);

	if (!g_ctx->m_observer_target || !i_game_event::get_player_controller)
		return original(p_game_event_manager, p_game_event);

	i_game_event::CUtlStringToken attacker_token("attacker");
	attacker_token.pad = 0xFFFFFFFF;

	c_cs_player_controller* attacker_controller = reinterpret_cast<c_cs_player_controller*>(i_game_event::get_player_controller(p_game_event, &attacker_token));
	if (!valid_ptr(attacker_controller))
		return original(p_game_event_manager, p_game_event);

	const auto attacker_pawn = g_interfaces->m_game_resource->pGameEntitySystem->Get<c_cs_player_pawn>(attacker_controller->m_pawn());

	if (attacker_pawn != g_ctx->m_observer_target)
		return original(p_game_event_manager, p_game_event);

	i_game_event::CUtlStringToken weapon_token("weapon");
	weapon_token.pad = 0xFFFFFFFF;

	const char* weapon_name = i_game_event::get_string(p_game_event, &weapon_token, nullptr);

	if (!valid_ptr(weapon_name))
		return original(p_game_event_manager, p_game_event);

	const bool is_knife = knife_db::matches(weapon_name);

	if (is_knife && g_cfg->knife_changer.m_knife != 0) {
		const char* new_weapon_name = get_knife_weapon_name(g_cfg->knife_changer.m_knife);
		if (new_weapon_name) {
			i_game_event::CUtlStringToken set_token("weapon");
			set_token.pad = 0xFFFFFFFF;
			i_game_event::set_string(p_game_event, &set_token, new_weapon_name, 0);
		}
	}

	return original(p_game_event_manager, p_game_event);
}

void* __fastcall hooks::smoke_voxel_draw::hk_smoke_voxel_draw(void* a1, void* a2, int a3, int a4, void* a5, void* a6)
{
	auto original = m_smoke_voxel_draw.get_original<decltype(&hk_smoke_voxel_draw)>();

	if (g_cfg->visuals.m_enable_smoke)
		return NULL;

	return original(a1, a2, a3, a4, a5, a6);
}

void __fastcall hooks::draw_flashbang_overlay::hk_draw_flashbang_overlay(void* a1, int a2, __int64* a3, __int64 a4, __m128* a5)
{
	auto original = m_draw_flashbang_overlay.get_original<decltype(&hk_draw_flashbang_overlay)>();

	if (g_cfg->visuals.m_enable_draw_flashbang)
		return;

	original(a1, a2, a3, a4, a5);
}

void __fastcall hooks::firstperson_legs_prepass::hk_firstperson_legs_prepass(void* a1, void* a2, void* a3, void* a4, void* a5)
{
	auto original = m_firstperson_legs_prepass.get_original<decltype(&hk_firstperson_legs_prepass)>();

	if (g_cfg->visuals.m_enable_draw_legs)
		return;

	original(a1, a2, a3, a4, a5);
}

HRESULT hooks::present::hk_present(IDXGISwapChain* swap_chain, unsigned int sync_interval, unsigned int flags) {
	auto original = m_present.get_original<decltype(&hk_present)>();

	g_directx->start_frame(swap_chain);

	if (g_directx->get_imgui_init_status() && g_directx->get_window() && g_directx->get_render_target()) {

		g_directx->new_frame();

		g_menu->draw();

		g_directx->end_frame();
	}

	return original(swap_chain, sync_interval, flags);
}

HRESULT hooks::resize_buffers::hk_resize_buffers(IDXGISwapChain* swap_chain, UINT buffer_count, UINT width, UINT height, DXGI_FORMAT new_format, UINT swap_chain_flags) {
	auto original = m_resize_buffers.get_original<decltype(&hk_resize_buffers)>();

	auto result = original(swap_chain, buffer_count, width, height, new_format, swap_chain_flags);
	if (SUCCEEDED(result))
		g_directx->create_render_target();

	return result;
}

HRESULT __stdcall hooks::create_swap_chain::hk_create_swap_chain(IDXGIFactory* factory, IUnknown* device, DXGI_SWAP_CHAIN_DESC* desc, IDXGISwapChain** swap_chain) {
	auto original = m_create_swap_chain.get_original<decltype(&hk_create_swap_chain)>();

	g_directx->destroy_render_target();
	return original(factory, device, desc, swap_chain);
}

//https://www.unknowncheats.me/forum/counter-strike-2-a/754206-custom-paintkit-colors.html
void* __fastcall hooks::build_material::hk_build_material(void* rcx, void* weapon, void* vector)
{
	auto original = m_build_material.get_original<decltype(&hk_build_material)>();

	g_changer->on_build_material_hook(vector, *reinterpret_cast<c_econ_item_view**>((uintptr_t)rcx - 0x38));

	return original(rcx, weapon, vector);
}

void __fastcall hooks::draw_array_light::hk_draw_array_light(CLightBinnerGpu* pLightBinnerGPU, CAggregateSceneObject* pAggregateSceneObject, CSceneObjectInfo* a3)
{
	auto original = m_draw_array_light.get_original<decltype(&hk_draw_array_light)>();

	//Change Light Color
	if (g_cfg->visuals.m_change_color_light)
	{

		if (g_world->m_light_color.is_zero())
		{
			g_world->m_light_color = pAggregateSceneObject->m_cLightColor;
		}

		vec3_t LColor;
		LColor.x = (g_cfg->visuals.m_color_light[0]) * (g_cfg->visuals.m_color_light[3]);
		LColor.y = (g_cfg->visuals.m_color_light[1]) * (g_cfg->visuals.m_color_light[3]);
		LColor.z = (g_cfg->visuals.m_color_light[2]) * (g_cfg->visuals.m_color_light[3]);

		pAggregateSceneObject->m_cLightColor = LColor;

	}
	else
	{
		if (!g_world->m_light_color.is_zero() && g_world->CheckVectors(pAggregateSceneObject->m_cLightColor, g_world->m_light_color))
		{
			pAggregateSceneObject->m_cLightColor = g_world->m_light_color;
		}
	}
	//Change Light Direction
	if (g_cfg->visuals.m_change_shadow_dir)
	{
		if (g_world->m_shadow_dir.is_zero())
		{
			g_world->m_shadow_dir = a3->m_pScene->m_pSceneLightObject->m_vDirectionNormalized;
		}

		vec3_t ShadowDir =
		{
			g_cfg->visuals.m_shadow_pitch,
			g_cfg->visuals.m_shadow_yaw,
			g_cfg->visuals.m_shadow_roll
		};

		ShadowDir.normalize_in_place();

		if (CSceneLightObject* pLightObject = a3->m_pScene->m_pSceneLightObject)
		{
			if (g_world->m_shadow_dir.is_zero())
			{
				g_world->m_shadow_dir = pLightObject->m_vDirectionNormalized;
			}

			pLightObject->m_vDirectionNormalized = ShadowDir;
		}
	}
	else
	{
		if (!g_world->m_shadow_dir.is_zero() && g_world->CheckVectors(a3->m_pScene->m_pSceneLightObject->m_vDirectionNormalized, g_world->m_shadow_dir))
		{
			a3->m_pScene->m_pSceneLightObject->m_vDirectionNormalized = g_world->m_shadow_dir;
		}
	}

	original(pLightBinnerGPU, pAggregateSceneObject, a3);
}
