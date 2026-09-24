#pragma once

#include "../../../main.hpp"

class c_econ_item_schema;
class c_econ_item_definition;

class c_paint_kit {
public:
	int m_id;
	const char* m_name;
	const char* m_description_string;
	const char* m_description_tag;
	std::byte pad0[0x8];
	std::byte pad1[0x8];
	std::byte pad2[0x8];
	std::byte pad3[0x8];
	std::byte pad4[0x4];
	int m_rarity;

	bool uses_old_model() {
		return *reinterpret_cast<bool*>(reinterpret_cast<uintptr_t>(this) + 0xAE);
	}
};

class c_music_kit {
public:
	int32_t m_id;
	int32_t m_unk0;
	const char* m_name;
	const char* m_name_loc_token;
	const char* m_loc_description;
	const char* m_pedestal_display_model;
	const char* m_inventory_image;
private:
	std::byte pad[0x10];
};

template<typename K, typename V>
class c_utl_map {
public:
	struct node_t {
		int m_left;
		int m_right;
		V m_value;
		int m_flag;
		K m_key;
	};
	static_assert(sizeof(node_t) == 24, "c_utl_map::node_t must be 24 bytes to match game layout");

	int m_count;
	int m_capacity_flags;
	node_t* m_elements;

	int count() const { return m_count; }

	node_t& element(int i) { return m_elements[i]; }
	const node_t& element(int i) const { return m_elements[i]; }

	V find_by_key(K key) {
		for (int i = 0; i < m_count; i++) {
			if (m_elements[i].m_key == key)
				return m_elements[i].m_value;
		}
		return nullptr;
	}
};

template<>
class c_utl_map<int, c_paint_kit*> {
public:
	struct node_t {
		int m_left;
		int m_right;
		int _pad_8;
		int _pad_12;
		int m_key;
		int _pad_20;
		c_paint_kit* m_value;
	};
	static_assert(sizeof(node_t) == 32, "paint-kit node_t must be 32 bytes");

	int m_count;
	int m_capacity_flags;
	node_t* m_elements;

	int count() const { return m_count; }

	node_t& element(int i) { return m_elements[i]; }
	const node_t& element(int i) const { return m_elements[i]; }

	c_paint_kit* find_by_key(int key) {
		for (int i = 0; i < m_count; i++) {
			if (m_elements[i].m_key == key)
				return m_elements[i].m_value;
		}
		return nullptr;
	}
};

class c_econ_item_schema {
public:

	static constexpr auto OFFSET_SORTED_ITEM_DEF_MAP = 0xF8;
	static constexpr auto OFFSET_PAINT_KITS = 0x2F0;
	static constexpr auto OFFSET_MUSIC_KITS = 0x4D0;

	c_utl_map<int, c_econ_item_definition*>& get_sorted_item_definition_map() {
		return *reinterpret_cast<c_utl_map<int, c_econ_item_definition*>*>(
			reinterpret_cast<std::uintptr_t>(this) + OFFSET_SORTED_ITEM_DEF_MAP
		);
	}

	c_utl_map<int, c_paint_kit*>& get_paint_kits() {
		return *reinterpret_cast<c_utl_map<int, c_paint_kit*>*>(
			reinterpret_cast<std::uintptr_t>(this) + OFFSET_PAINT_KITS
		);
	}

	c_utl_map<int, c_music_kit*>& get_music_kits() {
		return *reinterpret_cast<c_utl_map<int, c_music_kit*>*>(
			reinterpret_cast<std::uintptr_t>(this) + OFFSET_MUSIC_KITS
		);
	}
};

class c_econ_item_system {
public:
	c_econ_item_schema* get_econ_item_schema() {

		return *reinterpret_cast<c_econ_item_schema**>(
			reinterpret_cast<std::uintptr_t>(this) + 0x8
		);
	}
};

struct view_matrix_t {
	float m[4][4];

	vec3_t transform(const vec3_t& point) const {
		float w = m[3][0] * point.x + m[3][1] * point.y + m[3][2] * point.z + m[3][3];
		if (w < 0.001f)
			return vec3_t(-1.f, -1.f, -1.f);

		float x = m[0][0] * point.x + m[0][1] * point.y + m[0][2] * point.z + m[0][3];
		float y = m[1][0] * point.x + m[1][1] * point.y + m[1][2] * point.z + m[1][3];

		return vec3_t(x / w, y / w, 0.f);
	}
};

class c_source2_client {
public:
	c_econ_item_system* get_econ_item_system() {
		return vmt::call_virtual<c_econ_item_system*>(this, 129);
	}
};
