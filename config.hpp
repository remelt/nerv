#pragma once
#include <cstdint>
#include <memory>
#include <string>

#ifdef _DEBUG
    #define CONSOLE_ENABLED
#endif

class c_config {
public:
	struct knife_changer_t {
		bool m_enabled = false;
		int m_knife = 0;
		int m_paint_kit = 0;
		float m_wear = 0.0001f;
		int m_seed = 0;
		char m_custom_name[161] = {};
		bool m_custom_color = false;
		float m_color1[3];
		float m_color2[3];
		float m_color3[3];
		float m_color4[3];
	} knife_changer;

	struct glove_changer_t {
		bool m_enabled = false;
		int m_glove = 0;
		int m_paint_kit = 0;
		float m_wear = 0.0001f;
		int m_seed = 0;
		bool m_custom_color = false;
		float m_color1[3];
		float m_color2[3];
		float m_color3[3];
		float m_color4[3];
	} glove_changer;

	struct agent_changer_t {
		bool m_enabled = false;
		int m_agent = 0;
	} agent_changer;

	struct custom_model_t {
		bool m_enabled = false;
		bool m_should_update = false;
		std::string m_model_path = "agents/models/tm_phoenix/tm_phoenix.vmdl";
	} custom_model;

	struct skin_changer_t {
		bool m_enabled = false;
		int m_selected_weapon = 0;

		struct weapon_skin_t {
			int paint_kit = 0;
			float wear = 0.0001f;
			int seed = 0;
			char custom_name[161] = {};
			bool m_custom_color = false;
			float m_color1[3];
			float m_color2[3];
			float m_color3[3];
			float m_color4[3];
		};

		weapon_skin_t weapon_skins[100];

		static int get_config_index(uint16_t def_index) {
			if (def_index >= 1 && def_index <= 70) return def_index;
			return 0;
		}
	} skin_changer;

	struct misc_t
	{
		
		int m_menu_key = 0x2D;
	} misc;

	struct visuals_t
	{
		bool m_enable_smoke = false;
		bool m_enable_draw_flashbang = false;
		bool m_enable_draw_legs = false;

		bool m_change_shadow_dir = false;
		float m_shadow_pitch = 0.f;
		float m_shadow_yaw = 0.f;
		float m_shadow_roll = 0.f;

		bool m_change_color_light = false;
		float m_color_light[4] = { 1.f, 1.f, 1.f, 1.f };

	} visuals;

};

#define g_cfg g_config
inline const auto g_config = std::make_unique<c_config>();
