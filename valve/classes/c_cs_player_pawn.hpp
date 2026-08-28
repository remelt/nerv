#pragma once

#include "../schema/schema.hpp"
#include "../../sdk/typedefs/c_handle.hpp"
#include "game_enums.hpp"

class c_econ_item_definition;
class c_paint_kit;
class c_base_player_weapon;
class c_cs_player_controller;

static uint32_t murmur_hash2(const void* key, int len, uint32_t seed) {
	const uint32_t m = 0x5bd1e995;
	const int r = 24;
	uint32_t h = seed ^ len;
	const unsigned char* data = (const unsigned char*)key;

	while (len >= 4) {
		uint32_t k = *(uint32_t*)data;
		k *= m;
		k ^= k >> r;
		k *= m;
		h *= m;
		h ^= k;
		data += 4;
		len -= 4;
	}

	switch (len) {
	case 3: h ^= data[2] << 16; [[fallthrough]];
	case 2: h ^= data[1] << 8; [[fallthrough]];
	case 1: h ^= data[0]; h *= m;
	}

	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;
	return h;
}

static uint32_t string_token_hash(const char* str) {
	if (!str) return 0;
	int len = (int)strlen(str);
	char* lower = (char*)_malloca(len + 1);
	if (!lower) return 0;
	for (int i = 0; i < len; i++) {
		char c = str[i];
		lower[i] = (c >= 'A' && c <= 'Z') ? (c + 32) : c;
	}
	lower[len] = '\0';
	uint32_t hash = murmur_hash2(lower, len, 0x31415926);
	_freea(lower);
	return hash;
}

enum EntityFlags_t : uint32_t {
	EF_IS_INVALID_EHANDLE = 0x1,
	EF_SPAWN_IN_PROGRESS = 0x2,
	EF_IN_STAGING_LIST = 0x4,
	EF_IN_POST_DATA_UPDATE = 0x8,
	EF_DELETE_IN_PROGRESS = 0x10,
	EF_IN_STASIS = 0x20,
	EF_IS_ISOLATED_ALLOCATION_NETWORKABLE = 0x40,
	EF_IS_DORMANT = 0x80,
	EF_IS_PRE_SPAWN = 0x100,
	EF_MARKED_FOR_DELETE = 0x200,
	EF_IS_CONSTRUCTION_IN_PROGRESS = 0x400,
	EF_IS_ISOLATED_ALLOCATION = 0x800,
	EF_HAS_BEEN_UNSERIALIZED = 0x1000,
	EF_IS_SUSPENDED = 0x2000,
	EF_IS_ANONYMOUS_ALLOCATION = 0x4000,
	EF_SUSPEND_OUTSIDE_PVS = 0x8000,
};

enum MoveType_t : int {
	MOVETYPE_NONE = 0,
	MOVETYPE_ISOMETRIC,
	MOVETYPE_WALK,
	MOVETYPE_STEP,
	MOVETYPE_FLY,
	MOVETYPE_FLYGRAVITY,
	MOVETYPE_VPHYSICS,
	MOVETYPE_PUSH,
	MOVETYPE_NOCLIP,
	MOVETYPE_LADDER,
	MOVETYPE_OBSERVER,
	MOVETYPE_CUSTOM,
};

enum Hitbox_t : int {
	HITBOX_HEAD = 0,
	HITBOX_NECK,
	HITBOX_PELVIS,
	HITBOX_STOMACH,
	HITBOX_LOWER_CHEST,
	HITBOX_CHEST,
	HITBOX_UPPER_CHEST,
	HITBOX_RIGHT_THIGH,
	HITBOX_LEFT_THIGH,
	HITBOX_RIGHT_CALF,
	HITBOX_LEFT_CALF,
	HITBOX_RIGHT_FOOT,
	HITBOX_LEFT_FOOT,
	HITBOX_RIGHT_UPPER_ARM,
	HITBOX_RIGHT_FOREARM,
	HITBOX_LEFT_UPPER_ARM,
	HITBOX_LEFT_FOREARM,
	HITBOX_RIGHT_HAND,
	HITBOX_LEFT_HAND,
	HITBOX_MAX
};

enum HitGroup_t : int {
	HITGROUP_GENERIC = 0,
	HITGROUP_HEAD,
	HITGROUP_CHEST,
	HITGROUP_STOMACH,
	HITGROUP_LEFTARM,
	HITGROUP_RIGHTARM,
	HITGROUP_LEFTLEG,
	HITGROUP_RIGHTLEG,
	HITGROUP_NECK,
	HITGROUP_GEAR = 10
};

enum { LOOSE_VAR_COLOR4 = 9 };

struct CompositeMaterialInputLooseVariable_t {
	char* m_strName;                 // CUtlString
	uint8_t  pad_008[0x38];
	int32_t  m_nVariableType;
	uint8_t  pad_044[0x48];
	uint32_t m_cValueColor4;            // rgba
	uint8_t  pad_090[0x288 - 0x90];
};

class c_entity_identity {
public:
	SCHEMA(m_name, const char*, "CEntityIdentity", "m_name");
	SCHEMA(m_designer_name, const char*, "CEntityIdentity", "m_designerName");
	SCHEMA(m_flags, std::uint32_t, "CEntityIdentity", "m_flags");

	bool is_valid() {
		return m_index() != INVALID_EHANDLE_INDEX;
	}

	bool is_in_staging_list() {
		return (m_flags() & EF_IN_STAGING_LIST) != 0;
	}

	bool is_safe_to_modify() {
		uint32_t flags = m_flags();
		uint32_t unsafe_flags = EF_IN_STAGING_LIST | EF_DELETE_IN_PROGRESS |
			EF_SPAWN_IN_PROGRESS | EF_IN_POST_DATA_UPDATE |
			EF_IS_CONSTRUCTION_IN_PROGRESS | EF_MARKED_FOR_DELETE;
		return (flags & unsafe_flags) == 0;
	}

	int get_entry_index() {
		if (!is_valid())
			return ENT_ENTRY_MASK;

		return m_index() & ENT_ENTRY_MASK;
	}

	int get_serial_number() {
		return m_index() >> NUM_SERIAL_NUM_SHIFT_BITS;
	}

	bool is_type(const char* name) {
		return fnv1a::hash_64(name) == fnv1a::hash_64(m_designer_name());
	}

	OFFSET(int, m_index, 0x10);
};

class c_entity_instance {
public:
	c_base_handle get_handle() {
		c_entity_identity* identity = m_entity();
		if (identity == nullptr)
			return c_base_handle();

		return c_base_handle(identity->m_index(), identity->get_serial_number() - (identity->m_flags() & 1));
	}

	const char* get_designer_name() {
		c_entity_identity* identity = m_entity();
		if (!identity)
			return nullptr;
		return *reinterpret_cast<const char**>(reinterpret_cast<uintptr_t>(identity) + 0x20);
	}

	struct c_schema_class_binding {
		void* m_parent;
		const char* m_binding_name;
		const char* m_dll_name;
		int m_size_of;
	};

	c_schema_class_binding* get_schema_class_binding() {
		c_schema_class_binding* binding = nullptr;
		vmt::call_virtual<void>(this, 44, &binding);
		return binding;
	}

	const char* get_class_name() {
		auto* binding = get_schema_class_binding();
		if (!binding)
			return nullptr;
		return binding->m_binding_name;
	}

	bool is_player_pawn() {
		const char* name = get_class_name();
		return name && strcmp(name, "C_CSPlayerPawn") == 0;
	}

	bool is_player_controller() {
		const char* class_name = get_class_name();
		const char* designer_name = get_designer_name();

		bool is_controller = false;
		if (class_name) {
			is_controller = strcmp(class_name, "CCSPlayerController") == 0;
		}
		if (!is_controller && designer_name) {
			is_controller = strcmp(designer_name, "cs_player_controller") == 0;
		}

		return is_controller;
	}

	bool is_weapon() {
		const char* name = get_designer_name();
		return name && strstr(name, "weapon_") != nullptr;
	}

	SCHEMA(m_entity, c_entity_identity*, "CEntityInstance", "m_pEntity");
};

class c_skeleton_instance;

class c_hitbox {
public:
	const char* m_name;
	const char* m_surface_prop;
	const char* m_bone_name;
	vec3_t m_vec_min;
	vec3_t m_vec_max;
	float m_shape_radius;
	int m_bone_index;
	int m_hitgroup;

};

class c_hitbox_set {
public:
	const char* m_name;
	std::byte pad_001[0x20];
	int m_hitbox_count;
	c_hitbox* m_hitboxes;

	c_hitbox* get_hitbox(int index) {
		if (index < 0 || index >= m_hitbox_count || !m_hitboxes)
			return nullptr;
		return reinterpret_cast<c_hitbox*>(reinterpret_cast<uintptr_t>(m_hitboxes) + (index * 0x70));
	}
};

class CUtlString
{
public:
	const char* m_pString;
};

class c_model {
public:
	/*c_hitbox_set* get_hitbox_set() {

		uintptr_t render_meshs_ptr = *reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(this) + 0x78);
		if (!render_meshs_ptr)
			return nullptr;

		uintptr_t render_meshs = *reinterpret_cast<uintptr_t*>(render_meshs_ptr);
		if (!render_meshs)
			return nullptr;

		return *reinterpret_cast<c_hitbox_set**>(render_meshs + 0x150);
	}

	c_hitbox* get_hitbox(int index) {
		c_hitbox_set* hitbox_set = get_hitbox_set();
		if (!hitbox_set)
			return nullptr;
		return hitbox_set->get_hitbox(index);
	}

	int get_hitbox_count() {
		c_hitbox_set* hitbox_set = get_hitbox_set();
		if (!hitbox_set)
			return 0;
		return hitbox_set->m_hitbox_count;
	}

	__declspec(noinline) const char** get_bone_names() {
		uintptr_t data = *reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(this) + 0x0);
		if (!data)
			return nullptr;
		return *reinterpret_cast<const char***>(data + 0x168);
	}

	__declspec(noinline) uint32_t get_bone_count() {
		uintptr_t data = *reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(this) + 0x0);
		if (!data)
			return 0;
		return *reinterpret_cast<uint32_t*>(data + 0x170);
	}

	int32_t bone_parent(int index) {

		uintptr_t model_addr = reinterpret_cast<uintptr_t>(this);

		uintptr_t bone_data_struct = *reinterpret_cast<uintptr_t*>(model_addr + 0x178);
		if (!bone_data_struct)
			return -1;

		uint32_t count = get_bone_count();
		if (index < 0 || static_cast<uint32_t>(index) >= count)
			return -1;

		int16_t* parent_array = *reinterpret_cast<int16_t**>(bone_data_struct + 0x8);
		if (!parent_array)
			return -1;

		return static_cast<int32_t>(parent_array[index]);
	}*/


	char pad_0080[248]; //0x0080
	c_network_utl_vector<CUtlString> m_MeshGroups; //0x00F8
};

template<typename T>
class c_strong_handle {
public:
	T* get() {
		return *reinterpret_cast<T**>(this);
	}

	operator T* () {
		return get();
	}

	T* operator->() {
		return get();
	}
};


class c_model_state {
public:
	SCHEMA(m_model, c_strong_handle<c_model>, "CModelState", "m_hModel");
	SCHEMA(m_ModelName, CUtlString, "CModelState", "m_ModelName"); // CUtlSymbolLarge
	SCHEMA(m_MeshGroupMask, uint64_t, "CModelState", "m_MeshGroupMask");
	OFFSET(c_bone_data*, get_bone_data, 0x80);

	c_model* get_model() {

		uintptr_t model_ptr = *reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(this) + 0xC0);
		if (!model_ptr)
			return nullptr;
		return *reinterpret_cast<c_model**>(model_ptr);
	}
};

class c_skeleton_instance {
public:
	char pad_003[0x1E0];
	int m_bone_count;
	char pad_002[0x18];
	int m_mask;
	char pad_001[0x4];
	matrix2x4_t* m_bone_cache;

	SCHEMA(m_model_state, c_model_state, "CSkeletonInstance", "m_modelState");
	SCHEMA(m_hitbox_set, uint8_t, "CSkeletonInstance", "m_nHitboxSet");

	void calc_world_space_bones(std::uint32_t bone_mask) {
		static const auto fn = reinterpret_cast<void(__fastcall*)(void*, unsigned int)>(
			g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "48 89 4C 24 ? 55 53 56 57 41 54 41 55 41 56 41 57 B8 ? ? ? ? E8 ? ? ? ? 48 2B E0 48 8D 6C 24 ? 48 8B 81")

			);
		if (fn) fn(this, bone_mask);
	}
};

class c_game_scene_node {
public:
	SCHEMA(m_node_to_world, c_transform, "CGameSceneNode", "m_nodeToWorld");
	SCHEMA(m_child, c_game_scene_node*, "CGameSceneNode", "m_pChild");
	SCHEMA(m_next_sibling, c_game_scene_node*, "CGameSceneNode", "m_pNextSibling");
	SCHEMA(m_owner, c_entity_instance*, "CGameSceneNode", "m_pOwner");
	SCHEMA(m_origin, vec3_t, "CGameSceneNode", "m_vecOrigin");
	SCHEMA(m_abs_origin, vec3_t, "CGameSceneNode", "m_vecAbsOrigin");

	c_skeleton_instance* get_skeleton_instance() {
		return vmt::call_virtual<c_skeleton_instance*>(this, 13);
	}

	void set_mesh_group_mask(uint64_t mask) {
		using fn_t = void(__fastcall*)(void*, uint64_t);
		static auto fn = reinterpret_cast<fn_t>(
			g_opcodes->scan(g_modules->m_modules.client_dll.get_name(),
				"48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8D 99 ? ? ? ? 48 8B 71")
			);
		if (fn) {
			fn(this, mask);
		}
	}
};

class c_collision {
public:
	SCHEMA(m_mins, vec3_t, "CCollisionProperty", "m_vecMins");
	SCHEMA(m_maxs, vec3_t, "CCollisionProperty", "m_vecMaxs");
	SCHEMA(m_solid_flags, std::uint8_t, "CCollisionProperty", "m_usSolidFlags");
	SCHEMA(m_collision_group, std::uint8_t, "CCollisionProperty", "m_CollisionGroup");

	std::uint16_t get_collision_mask() {
		return *reinterpret_cast<std::uint16_t*>(reinterpret_cast<std::uintptr_t>(this) + 0x38);
	}
};

class c_cs_player_pawn;

class c_glow_property {
public:
	SCHEMA(m_is_glowing, bool, "CGlowProperty", "m_bGlowing");
	SCHEMA(m_glow_color_override, int, "CGlowProperty", "m_glowColorOverride");
	SCHEMA(m_glow_type, int32_t, "CGlowProperty", "m_iGlowType");
	SCHEMA(m_glow_team, int32_t, "CGlowProperty", "m_iGlowTeam");
	SCHEMA(m_glow_range, int32_t, "CGlowProperty", "m_nGlowRange");
	SCHEMA(m_glow_range_min, int32_t, "CGlowProperty", "m_nGlowRangeMin");
	SCHEMA(m_glow_time, float, "CGlowProperty", "m_flGlowTime");
	SCHEMA(m_glow_start_time, float, "CGlowProperty", "m_flGlowStartTime");

	c_cs_player_pawn* get_owner() {
		return *reinterpret_cast<c_cs_player_pawn**>(reinterpret_cast<uintptr_t>(this) + 0x18);
	}
};


class c_base_entity : public c_entity_instance {
public:

	SCHEMA(m_health, int, "C_BaseEntity", "m_iHealth");
	SCHEMA(m_team_num, int, "C_BaseEntity", "m_iTeamNum");
	SCHEMA(m_flags, int, "C_BaseEntity", "m_fFlags");
	SCHEMA(m_move_type, int, "C_BaseEntity", "m_MoveType");
	SCHEMA(m_nSubclassID, uint32_t, "C_BaseEntity", "m_nSubclassID");
	SCHEMA(m_scene_node, c_game_scene_node*, "C_BaseEntity", "m_pGameSceneNode");
	SCHEMA(m_owner_entity, c_base_handle, "C_BaseEntity", "m_hOwnerEntity");
	SCHEMA(m_collision, c_collision*, "C_BaseEntity", "m_pCollision");
	SCHEMA(m_vec_velocity, vec3_t, "C_BaseEntity", "m_vecVelocity");
	SCHEMA(m_vec_abs_velocity, vec3_t, "C_BaseEntity", "m_vecAbsVelocity");
	SCHEMA(m_sim_time, float, "C_BaseEntity", "m_flSimulationTime");
	SCHEMA(m_simulation_tick, int, "C_BaseEntity", "m_nSimulationTick");


	void set_model(const char* model_path) {
		static auto fn = reinterpret_cast<void(__fastcall*)(void*, const char*)>(
			g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "40 53 48 83 EC ? 48 8B D9 4C 8B C2 48 8B 0D ? ? ? ? 48 8D 54 24 40")
			);
		if (fn) {
			fn(this, model_path);
		}
	}

	bool is_weapon() {
		return vmt::call_virtual<bool>(this, 158);
	}

	c_hitbox_set* get_hitbox_set(unsigned int index) {
		using fn_get_hitbox_set = std::int64_t(__fastcall*)(void*, unsigned int);
		static auto fn = reinterpret_cast<fn_get_hitbox_set>(
			g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "48 89 5C 24 ? 48 89 74 24 ? 57 48 81 EC ? ? ? ? 8B DA 48 8B F9 E8 ? ? ? ? 48 8B F0 48 85 C0 0F 84")
			);
		return reinterpret_cast<c_hitbox_set*>(fn(this, index));
	}
};

class c_attribute_list {
public:
	SCHEMA(m_attributes, uintptr_t, "CAttributeList", "m_Attributes");
};

enum EItemQuality : int {
	QUALITY_DEFAULT = 0,
	QUALITY_GENUINE = 1,
	QUALITY_VINTAGE = 2,
	QUALITY_UNUSUAL = 3,
	QUALITY_UNIQUE = 4,
	QUALITY_COMMUNITY = 5,
	QUALITY_DEVELOPER = 6,
	QUALITY_SELFMADE = 7,
	QUALITY_CUSTOMIZED = 8,
	QUALITY_STRANGE = 9,
	QUALITY_COMPLETED = 10,
	QUALITY_HAUNTED = 11,
	QUALITY_TOURNAMENT = 12,
};

class c_econ_item_view {
public:
	SCHEMA(m_definition_index, uint16_t, "C_EconItemView", "m_iItemDefinitionIndex");
	SCHEMA(m_entity_quality, int32_t, "C_EconItemView", "m_iEntityQuality");
	SCHEMA(m_item_id, uint64_t, "C_EconItemView", "m_iItemID");
	SCHEMA(m_item_id_high, uint32_t, "C_EconItemView", "m_iItemIDHigh");
	SCHEMA(m_item_id_low, uint32_t, "C_EconItemView", "m_iItemIDLow");
	SCHEMA(m_account_id, uint32_t, "C_EconItemView", "m_iAccountID");
	SCHEMA(m_initialized, bool, "C_EconItemView", "m_bInitialized");
	SCHEMA_ARRAY(m_attribute_list, c_attribute_list, "C_EconItemView", "m_AttributeList");
	SCHEMA(m_bDisallowSOCm, bool, "C_EconItemView", "m_bDisallowSOC");
	SCHEMA(m_bRestoreCustomMaterialAfterPrecache, bool, "C_EconItemView", "m_bRestoreCustomMaterialAfterPrecache");
	SCHEMA_ARRAY(m_custom_name, char, "C_EconItemView", "m_szCustomName");
	OFFSET(std::uintptr_t, m_name_description_ptr, 0x200);

	c_econ_item_definition* get_static_data() {
		return vmt::call_virtual<c_econ_item_definition*>(this, 13);
	}

	c_paint_kit* construct_paint_kit() {
		static auto fn = reinterpret_cast<c_paint_kit * (__fastcall*)(void*)>(
			g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "48 89 5C 24 ? 56 48 83 EC ? 48 8B 01 FF 50")
			);
		if (fn) {
			return fn(this);
		}
		return nullptr;
	}

	int get_custom_paint_kit() {
		return vmt::call_virtual<int>(this, 2);
	}
};

class c_attribute_container {
public:
	SCHEMA_ARRAY(m_item, c_econ_item_view, "C_AttributeContainer", "m_Item");
};

class c_econ_entity : public c_base_entity {
public:
	SCHEMA_ARRAY(m_attribute_manager, c_attribute_container, "C_EconEntity", "m_AttributeManager");
	SCHEMA(m_paint_kit, int, "C_EconEntity", "m_nFallbackPaintKit");
	SCHEMA(m_seed, int, "C_EconEntity", "m_nFallbackSeed");
	SCHEMA(m_wear, float, "C_EconEntity", "m_flFallbackWear");
	SCHEMA(m_StatTrak, int, "C_EconEntity", "m_nFallbackStatTrak");

	SCHEMA(m_original_owner_xuid_low, uint32_t, "C_EconEntity", "m_OriginalOwnerXuidLow");
	SCHEMA(m_original_owner_xuid_high, uint32_t, "C_EconEntity", "m_OriginalOwnerXuidHigh");


	uint64_t get_original_owner_xuid() {
		return (static_cast<uint64_t>(m_original_owner_xuid_high()) << 32) | m_original_owner_xuid_low();
	}

	void update_composite(bool force = true) {
		vmt::call_virtual<void*>(this, 10, force);
	}

	void update_subclass(uint16_t def_index = 0) {
		static auto fn = reinterpret_cast<void(__fastcall*)(void*)>(
			g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "4C 8B DC 53 48 81 EC ?? ?? ?? ?? 48 8B 41")
			);

		if (def_index != 0) {
			char buf[16];
			sprintf_s(buf, "%d", def_index);
			uint32_t hash = string_token_hash(buf);
			m_nSubclassID() = hash;
		}
		fn(this);
	}

	void update_skin(bool force = true) {

		vmt::call_virtual<void>(this, 111, force);
	}

	void update_weapon_data() {

		vmt::call_virtual<void*>(this, 197);

	}

	c_econ_item_view* get_econ_item_view() {
		return this->m_attribute_manager()->m_item();
	}

	void add_stattrak_entity() {
		static auto fn = reinterpret_cast<void(__fastcall*)(void*, void*)>(
			g_opcodes->scan(
				g_modules->m_modules.client_dll.get_name(),
				"48 8B C4 48 89 58 08 48 89 70 10 57 48 83 EC 50 33 F6 8B FA"
			)
			);

		if (fn) {
			fn(this, this->get_econ_item_view());
		}
	}

	void add_nametag_entity() {
		static auto fn = reinterpret_cast<void(__fastcall*)(void*, void*)>(
			g_opcodes->scan(
				g_modules->m_modules.client_dll.get_name(),
				"40 55 53 41 56 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B DA 4C 8B"
			)
			);

		if (fn) {
			fn(this, this->get_econ_item_view());
		}

	}

	void rebuild_animgraph() {
		static auto fn = reinterpret_cast<void(__fastcall*)(void*)>(
			g_opcodes->scan(
				g_modules->m_modules.client_dll.get_name(),
				"40 55 56 48 83 EC 28 4C 89 74 24 58 48 8B F1 80"
			)
			);

		if (fn) {
			fn(this);
		}
	}

	
};

class c_player_weapon_service {
public:
	SCHEMA(m_active_weapon, c_base_handle, "CPlayer_WeaponServices", "m_hActiveWeapon");
	SCHEMA(my_weapons, c_network_utl_vector<c_base_handle>, "CPlayer_WeaponServices", "m_hMyWeapons");
};

using firing_float_t = float[2];

class c_cs_weapon_base_v_data {
public:
	SCHEMA(m_max_clip1, int32_t, "CBasePlayerWeaponVData", "m_iMaxClip1");
	SCHEMA(m_max_clip2, int32_t, "CBasePlayerWeaponVData", "m_iMaxClip2");
	SCHEMA(m_default_clip1, int32_t, "CBasePlayerWeaponVData", "m_iDefaultClip1");
	SCHEMA(m_default_clip2, int32_t, "CBasePlayerWeaponVData", "m_iDefaultClip2");
	SCHEMA(m_weight, int32_t, "CBasePlayerWeaponVData", "m_iWeight");
	SCHEMA(m_name, const char*, "CCSWeaponBaseVData", "m_szName");
	SCHEMA(m_cycle_time, firing_float_t, "CCSWeaponBaseVData", "m_flCycleTime");
	SCHEMA(m_damage, int, "CCSWeaponBaseVData", "m_nDamage");
	SCHEMA(m_armor_ratio, float, "CCSWeaponBaseVData", "m_flArmorRatio");
	SCHEMA(m_range, float, "CCSWeaponBaseVData", "m_flRange");
	SCHEMA(m_range_modifier, float, "CCSWeaponBaseVData", "m_flRangeModifier");
	SCHEMA(m_penetration, float, "CCSWeaponBaseVData", "m_flPenetration");
	SCHEMA(m_headshot_multiplier, float, "CCSWeaponBaseVData", "m_flHeadshotMultiplier");
	SCHEMA(m_spread, float, "CCSWeaponBaseVData", "m_flSpread");
	SCHEMA(m_bullets, int, "CCSWeaponBaseVData", "m_nNumBullets");
	SCHEMA(m_weapon_type, int, "CCSWeaponBaseVData", "m_WeaponType");
};

class c_base_player_weapon : public c_econ_entity {
public:
	SCHEMA(m_next_primary_attack, int32_t, "C_BasePlayerWeapon", "m_nNextPrimaryAttackTick");
	SCHEMA(m_next_primary_attack_ratio, float, "C_BasePlayerWeapon", "m_flNextPrimaryAttackTickRatio");
	SCHEMA(m_next_secondary_attack, int32_t, "C_BasePlayerWeapon", "m_nNextSecondaryAttackTick");
	SCHEMA(m_next_secondary_attack_ratio, float, "C_BasePlayerWeapon", "m_flNextSecondaryAttackTickRatio");
	SCHEMA(m_clip1, int32_t, "C_BasePlayerWeapon", "m_iClip1");
	SCHEMA(m_clip2, int32_t, "C_BasePlayerWeapon", "m_iClip2");
	SCHEMA(m_in_reload, bool, "C_CSWeaponBase", "m_bInReload");
	SCHEMA(m_burst_mode, bool, "C_CSWeaponBase", "m_bBurstMode");
	SCHEMA(m_burst_shots_remaining, int, "C_CSWeaponBaseGun", "m_iBurstShotsRemaining");

	SCHEMA_WITH_OFFSET(m_get_sub_class_id, void*, "C_BaseEntity", "m_nSubclassID", 0x8);

	c_cs_weapon_base_v_data* get_weapon_data() {
		return static_cast<c_cs_weapon_base_v_data*>(m_get_sub_class_id());
	}

	float get_max_speed() {
		return vmt::call_virtual<float>(this, 340);
	}
};



class c_cs_weapon_base : public c_base_player_weapon {
public:
	SCHEMA(m_zoom_level, int, "C_CSWeaponBaseGun", "m_zoomLevel");
	SCHEMA(m_i_original_team_number, int, "C_CSWeaponBase", "m_iOriginalTeamNumber");
	SCHEMA(m_is_in_reload, bool, "C_CSWeaponBase", "m_bInReload");
	SCHEMA(m_h_prev_owner, uint32_t, "C_CSWeaponBase", "m_hPrevOwner"); // CBaseHandle
	SCHEMA(m_b_ui_weapon, bool, "C_CSWeaponBase", "m_bUIWeapon");
	SCHEMA(m_weapon_mode, int, "C_CSWeaponBase", "m_weaponMode");
	SCHEMA(m_b_inspect_pending, bool, "C_CSWeaponBase", "m_bInspectPending");
	SCHEMA(m_b_inspect_should_loop, bool, "C_CSWeaponBase", "m_bInspectShouldLoop");
	SCHEMA(m_fl_inspect_cancel_complete_time, float, "C_CSWeaponBase", "m_flInspectCancelCompleteTime");

	float get_spread() {
		using fn_t = float(__fastcall*)(void*);
		static fn_t fn = reinterpret_cast<fn_t>(
			g_opcodes->scan(g_modules->m_modules.client_dll.get_name(),
				"48 83 EC ? 48 63 91 ? ? ? ? 48 8B 81 ? ? ? ? 0F 29 74 24 ? 85 D2")
			);
		return fn ? fn(this) : 0.0f;
	}

	float get_inaccuracy() {
		using fn_t = float(__fastcall*)(void*, float*, float*);
		static fn_t fn = reinterpret_cast<fn_t>(
			g_opcodes->scan(g_modules->m_modules.client_dll.get_name(),
				"48 89 5C 24 10 55 56 57 48 81 EC ? ? ? ? 44 0F 29 84 24 80 00 00 00")
			);
		return fn ? fn(this, nullptr, nullptr) : 0.0f;
	}

	void update_accuracy_penalty() {
		using fn_t = void(__fastcall*)(void*);
		auto** vt = *reinterpret_cast<void***>(this);
		auto fn = reinterpret_cast<fn_t>(vt[414]);
		fn(this);
	}
};


class c_base_cs_grenade : public c_cs_weapon_base {
public:
	SCHEMA(m_held_by_player, bool, "C_BaseCSGrenade", "m_bIsHeldByPlayer");
	SCHEMA(m_pin_pulled, bool, "C_BaseCSGrenade", "m_bPinPulled");
	SCHEMA(m_throw_time, float, "C_BaseCSGrenade", "m_fThrowTime");
	SCHEMA(m_throw_strength, float, "C_BaseCSGrenade", "m_flThrowStrength");
};

class c_cs_player_item_service {
public:
	SCHEMA(m_has_helmet, bool, "CCSPlayer_ItemServices", "m_bHasHelmet");
};

class c_player_movement_service {
public:
	SCHEMA(m_max_speed, float, "CPlayer_MovementServices", "m_flMaxspeed");
	SCHEMA(m_surface_friction, float, "CPlayer_MovementServices_Humanoid", "m_flSurfaceFriction");

};

class c_cs_player_modern_jump {
public:
	SCHEMA(m_last_landed_frac, float, "CCSPlayerModernJump", "m_flLastLandedFrac");
	SCHEMA(m_last_landed_velocity_x, float, "CCSPlayerModernJump", "m_flLastLandedVelocityX");
	SCHEMA(m_last_landed_velocity_y, float, "CCSPlayerModernJump", "m_flLastLandedVelocityY");
	SCHEMA(m_last_landed_velocity_z, float, "CCSPlayerModernJump", "m_flLastLandedVelocityZ");
};

class c_cs_player_movement_services : public c_player_movement_service {
public:
	SCHEMA(m_duck_amount, float, "CCSPlayer_MovementServices", "m_flDuckAmount");
	SCHEMA(m_duck_speed, float, "CCSPlayer_MovementServices", "m_flDuckSpeed");
	SCHEMA(m_ducked, bool, "CCSPlayer_MovementServices", "m_bDucked");
	SCHEMA(m_ducking, bool, "CCSPlayer_MovementServices", "m_bDucking");
	SCHEMA(m_stamina, float, "CCSPlayer_MovementServices", "m_flStamina");
	SCHEMA(m_offset_tick_complete_time, float, "CCSPlayer_MovementServices", "m_flOffsetTickCompleteTime");
	SCHEMA(m_offset_tick_stashed_speed, float, "CCSPlayer_MovementServices", "m_flOffsetTickStashedSpeed");
	SCHEMA_ARRAY(m_arr_force_subtick_move_when, float, "CPlayer_MovementServices", "m_arrForceSubtickMoveWhen");

	c_cs_player_modern_jump* m_modern_jump() {
		static short offset = schema_get_offset("CCSPlayer_MovementServices", "m_ModernJump");
		if (!offset)
			return nullptr;
		return reinterpret_cast<c_cs_player_modern_jump*>(reinterpret_cast<uintptr_t>(this) + offset);
	}
};

class c_player_observer_services {
public:
	SCHEMA(m_observer_mode, uint8_t, "CPlayer_ObserverServices", "m_iObserverMode");
	SCHEMA(m_observer_target, c_base_handle, "CPlayer_ObserverServices", "m_hObserverTarget");
};

class c_cs_player_pawn : public c_base_entity {
public:
	SCHEMA(m_weapon_services, c_player_weapon_service*, "C_BasePlayerPawn", "m_pWeaponServices");
	SCHEMA(m_controller, c_base_handle, "C_BasePlayerPawn", "m_hController");
	SCHEMA(m_controller_default, c_base_handle, "C_BasePlayerPawn", "m_hDefaultController");
	SCHEMA_ARRAY(m_econ_gloves, c_econ_item_view, "C_CSPlayerPawn", "m_EconGloves");
	SCHEMA(m_need_to_reapply_gloves, bool, "C_CSPlayerPawn", "m_bNeedToReApplyGloves");
	SCHEMA(m_hud_model_arms, c_base_handle, "C_CSPlayerPawn", "m_hHudModelArms");
	SCHEMA(m_last_spawn_time_index, float, "C_CSPlayerPawnBase", "m_flLastSpawnTimeIndex");
	SCHEMA(m_is_buy_menu_open, bool, "C_CSPlayerPawn", "m_bIsBuyMenuOpen");
	SCHEMA(m_armor_value, int, "C_CSPlayerPawn", "m_ArmorValue");
	SCHEMA(m_movement_services, c_player_movement_service*, "C_BasePlayerPawn", "m_pMovementServices");
	SCHEMA(m_item_services, c_cs_player_item_service*, "C_BasePlayerPawn", "m_pItemServices");
	SCHEMA(m_scoped, bool, "C_CSPlayerPawn", "m_bIsScoped");
	SCHEMA(m_observer_services, c_player_observer_services*, "C_BasePlayerPawn", "m_pObserverServices");
	SCHEMA(m_has_female_voice, bool, "C_CSPlayerPawn", "m_bHasFemaleVoice");

	bool is_alive() {
		return m_health() > 0;
	}

	c_cs_player_controller* get_controller() {
		if (!m_controller().is_valid())
			return nullptr;

		extern void* get_entity_by_index(int index);
		return reinterpret_cast<c_cs_player_controller*>(get_entity_by_index(m_controller().get_entry_index()));
	}

	c_cs_player_controller* get_default_controller() {
		if (!m_controller_default().is_valid())
			return nullptr;

		extern void* get_entity_by_index(int index);
		return reinterpret_cast<c_cs_player_controller*>(get_entity_by_index(m_controller().get_entry_index()));
	}

	void set_body_group(const char* name = "first_or_third_person") {

		using fn_t = void(__fastcall*)(void*, const char*, unsigned int);
		static auto fn = reinterpret_cast<fn_t>(
			g_opcodes->scan_absolute(g_modules->m_modules.client_dll.get_name(),
				"E8 ? ? ? ? EB 0C 48 8B CF", 0x1)
			);
		if (fn) {
			fn(this, name, 1);
		}
	}

	vec3_t get_eye_pos() {
		vec3_t view_;
		vmt::call_virtual<void>(this, 174, &view_);
		return view_;
	}

	int get_bone_index(const char* name) {
		static const auto fn = reinterpret_cast<int(__fastcall*)(void*, const char*)>(
			g_opcodes->scan_absolute(g_modules->m_modules.client_dll.get_name(), "E8 ? ? ? ? 85 C0 78 ? 4C 8D 4C 24 ? 4C 8B C7", 0x1)
			);
		return fn(this, name);
	}

	vec3_t get_bone_position(int index) {
		using GetBonePosition_t = int(__fastcall*)(void*, int, vec3_t&, vec3_t&);
		static GetBonePosition_t fn = reinterpret_cast<GetBonePosition_t>(
			g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC ? 4D 8B F1 49 8B E8")
			);

		vec3_t position;
		vec3_t rotation;
		fn(this, index, position, rotation);
		return position;
	}

	c_base_player_weapon* get_active_weapon() {
		c_player_weapon_service* weapon_services = this->m_weapon_services();
		if (!weapon_services)
			return nullptr;

		if (!weapon_services->m_active_weapon().is_valid())
			return nullptr;

		extern void* get_entity_by_index(int index);
		return reinterpret_cast<c_base_player_weapon*>(get_entity_by_index(weapon_services->m_active_weapon().get_entry_index()));
	}

	bool has_armor(int hitgroup) {
		if (hitgroup == HITGROUP_HEAD)
			return this->m_item_services() && this->m_item_services()->m_has_helmet();
		return this->m_armor_value() > 0;
	}

	bool is_throwing() {
		c_base_player_weapon* active_weapon = this->get_active_weapon();
		if (!active_weapon)
			return false;

		c_base_cs_grenade* grenade = reinterpret_cast<c_base_cs_grenade*>(active_weapon);
		c_cs_weapon_base_v_data* weapon_data = active_weapon->get_weapon_data();

		if (!grenade || !weapon_data)
			return false;

		if (weapon_data->m_weapon_type() == WEAPONTYPE_GRENADE && grenade->m_throw_time() != 0.f)
			return true;

		return false;
	}

};

class c_cs_player_controller : public c_entity_instance {
public:
	SCHEMA(m_pawn, c_base_handle, "CBasePlayerController", "m_hPawn");
	SCHEMA(m_pawn_is_alive, bool, "CCSPlayerController", "m_bPawnIsAlive");
	SCHEMA(m_tick_base, uint32_t, "CBasePlayerController", "m_nTickBase");
	SCHEMA(m_steam_id, uint64_t, "CBasePlayerController", "m_steamID");
	SCHEMA_ARRAY(m_player_name, char, "CBasePlayerController", "m_iszPlayerName");

	bool physics_run_think() {
		using fn_t = bool(__fastcall*)(void*);
		static fn_t fn = reinterpret_cast<fn_t>(
			g_opcodes->scan(g_modules->m_modules.client_dll.get_name(),
				"48 89 5C 24 ? 57 48 81 EC D0 06 00 00 48 8B 01 48 8B F9 FF 90 C8 06 00 00 83 BF 88 07 00 00 00")
			);
		return fn ? fn(this) : false;
	}
};

class c_econ_item_definition {
public:
	bool is_knife(bool exclude_default) {
		static auto csgo_type_knife = fnv1a::hash_64("#CSGO_Type_Knife");
		if (fnv1a::hash_64(m_item_type_name) != csgo_type_knife)
			return false;

		return exclude_default ? m_definition_index >= 500 : true;
	}

	bool is_glove(bool exclude_default) {
		static auto type_hands = fnv1a::hash_64("#Type_Hands");
		if (fnv1a::hash_64(m_item_type_name) != type_hands)
			return false;

		const bool default_glove = m_definition_index == 5028 || m_definition_index == 5029;
		return exclude_default ? !default_glove : true;
	}

	bool is_agent() {
		static auto type_custom_player = fnv1a::hash_64("#Type_CustomPlayer");
		return fnv1a::hash_64(m_item_type_name) == type_custom_player;
	}

	const char* get_model_name() {
		return *reinterpret_cast<const char**>((uintptr_t)(this) + 0x148);
	}

	const char* get_simple_weapon_name() {
		return *reinterpret_cast<const char**>((uintptr_t)(this) + 0x230);
	}

	const char* get_weapon_name() {
		return *reinterpret_cast<const char**>((uintptr_t)(this) + 0x248);
	}

	const char* get_item_name() {
		return *reinterpret_cast<const char**>((uintptr_t)(this) + 0x260);
	}

	const char* get_view_model() {
		return *reinterpret_cast<const char**>((uintptr_t)(this) + 0x348);
	}

	const char* get_vo_prefix() {
		return *reinterpret_cast<const char**>((uintptr_t)(this) + 0xEB0);
	}

public:
	std::byte pad_001[0x8];
	void* m_kv_item;
	std::uint16_t m_definition_index;
	std::byte pad_002[0x1e];
	bool m_enabled;
	std::byte pad_003[0xf];
	std::uint8_t m_min_item_level;
	std::uint8_t m_max_item_level;
	std::uint8_t m_item_rarity;
	std::uint8_t m_item_quality;
	std::byte pad_004[0x2c];
	char* m_item_base_name;
	std::byte pad_005[0x8];
	char* m_item_type_name;
	std::byte pad_006[0x8];
	char* m_item_desc;
};

