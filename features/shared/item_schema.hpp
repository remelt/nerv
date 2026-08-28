#pragma once

#include "../../main.hpp"
#include "../../valve/classes/game_enums.hpp"
#include "../../valve/classes/c_cs_player_pawn.hpp"
#include "../../valve/interfaces/vtables/i_econ_item_system.hpp"
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>

struct item_info_t {
	uint16_t definition_index;
	std::string name;
	const char* model_path;
	CImageProxySource* image;
};

struct paint_kit_info_t {
	int id;
	std::string name;
	std::string skinToken;
	int rarity;
	CImageProxySource* image;
};

struct progress {
	int m_types_left = 0;
	int m_types_count = 0;

	int m_skins_left = 0;
	int m_skins_count = 0;
	const char* m_weapon_name = "";
};

class c_item_schema {
public:
	std::vector<item_info_t> knives;
	std::vector<item_info_t> gloves;
	std::vector<item_info_t> weapons;
	std::vector<item_info_t> agents;

	std::vector<paint_kit_info_t> all_paint_kits;

	std::unordered_map<uint16_t, std::vector<paint_kit_info_t>> item_paint_kits;
	std::unordered_map<uint16_t, std::vector<const char*>> item_paint_kit_names;

	std::vector<const char*> knife_names_cstr;
	std::vector<const char*> glove_names_cstr;
	std::vector<const char*> weapon_names_cstr;
	std::vector<const char*> agent_names_cstr;

	void initialize();
	bool is_initialized() const { return m_initialized; }
	progress get_progress() const { return m_init_progress; }

	const std::vector<const char*>& get_paint_kit_names_for_item(uint16_t def_index) const {
		static std::vector<const char*> empty = { "Default" };
		auto it = item_paint_kit_names.find(def_index);
		return it != item_paint_kit_names.end() ? it->second : empty;
	}

	int get_paint_kit_id_for_item(uint16_t def_index, int index) const {
		auto it = item_paint_kits.find(def_index);
		if (it != item_paint_kits.end() && index >= 0 && index < (int)it->second.size())
			return it->second[index].id;
		return 0;
	}

private:
	bool m_initialized = false;
	progress m_init_progress;

	bool is_paint_kit_for_item(const char* simple_weapon_name, c_paint_kit* paint_kit);
	std::string get_skin_image_path(const std::string& simple_name, const char* paint_kit_name = "", bool check = false);
	CImageProxySource* get_skin_image(const std::string& simple_name, const char* paint_kit_name = "", bool check = false);
	void build_paint_kits_for_item(uint16_t def_index, c_utl_map<int, c_econ_item_definition*>& items, c_utl_map<int, c_paint_kit*>& paint_kit_map);
};

inline const auto g_item_schema = std::make_unique<c_item_schema>();

class c_hud {
public:
	static std::uintptr_t find_hud_element(const char* name);
	static void clear_hud_weapon_icon(std::uintptr_t hud_weapons, std::int32_t index, std::int64_t unk = 0);

	static void clear_hud_weapon_icons();

	static void clear_hud_weapon_icon_for(class c_base_entity* weapon);
	static void regenerate_skins();
};
