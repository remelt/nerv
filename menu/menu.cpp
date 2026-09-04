#include "../main.hpp"
#include "menu.hpp"

#include <algorithm>
#include <vector>
#include <string>
#include <sdk/includes/imgui/imgui_internal.h>

#include "../features/shared/item_schema.hpp"
#include "../features/skin_changer/skin_changer.hpp"
#include "../features/glove_changer/glove_changer.hpp"
#include "../features/agent_changer/agent_changer.hpp"
#include "../sdk/config_system/config_system.hpp"
#include "../valve/classes/c_cs_player_pawn.hpp"
#include "../valve/interfaces/interfaces.hpp"
#include "../sdk/includes/imgui/imgui_stdlib.h"

static constexpr const ImGuiColorEditFlags no_alpha = ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_PickerHueBar;
static constexpr const ImGuiColorEditFlags with_alpha = ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_PickerHueBar;


ImU32 GetRarityColor(int rarity) {
	switch (rarity) {
	case 1: return IM_COL32(176, 195, 217, 255); // Consumer
	case 2: return IM_COL32(94, 152, 217, 255);  // Industrial
	case 3: return IM_COL32(75, 105, 255, 255);  // Mil-Spec
	case 4: return IM_COL32(136, 71, 255, 255);  // Restricted
	case 5: return IM_COL32(211, 44, 230, 255);  // Classified
	case 6: return IM_COL32(235, 75, 75, 255);   // Covert
	case 7: return IM_COL32(228, 174, 57, 255);  // Contraband
	default: return IM_COL32(255, 255, 255, 255); // Default
	}
}

void c_menu::rebuild_fonts(float scale) {
	auto& io = ImGui::GetIO();

	io.Fonts->Clear();

	ImFontConfig config;
	config.SizePixels = 14.0f * scale;
	config.OversampleH = 2;
	config.OversampleV = 1;
	config.PixelSnapH = true;

	ImFontGlyphRangesBuilder Builder;
	Builder.AddRanges(io.Fonts->GetGlyphRangesDefault());
	Builder.AddRanges(io.Fonts->GetGlyphRangesCyrillic());
	Builder.AddRanges(io.Fonts->GetGlyphRangesThai());
	Builder.AddRanges(io.Fonts->GetGlyphRangesVietnamese());
	Builder.AddRanges(io.Fonts->GetGlyphRangesKorean());
	Builder.AddRanges(io.Fonts->GetGlyphRangesChineseFull());
	Builder.AddRanges(io.Fonts->GetGlyphRangesJapanese());

	ImVector<ImWchar> AllRanges;
	Builder.BuildRanges(&AllRanges);

	//  "tahoma.ttf"  "segoeuib.ttf" (Segoe UI)  "arial.ttf"
	ImFont* font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arial.ttf", 14.0f * scale, &config, AllRanges.Data);

	if (font == nullptr) {
		io.Fonts->AddFontDefault(&config);
	}

	io.Fonts->Build();

	ImGui_ImplDX11_InvalidateDeviceObjects();
	ImGui_ImplDX11_CreateDeviceObjects();

	m_dpi_scale = scale;
	m_style_initialized = false;
}
static std::string get_key_name(int vk) {
	if (vk == 0) return "None";

	switch (vk) {
	case VK_TAB: return "Tab";
	case VK_LEFT: return "Left Arrow";
	case VK_RIGHT: return "Right Arrow";
	case VK_UP: return "Up Arrow";
	case VK_DOWN: return "Down Arrow";
	case VK_PRIOR: return "Page Up";
	case VK_NEXT: return "Page Down";
	case VK_HOME: return "Home";
	case VK_INSERT: return "Insert";
	case VK_DELETE: return "Delete";
	case VK_BACK: return "Backspace";
	case VK_SPACE: return "Space";
	case VK_RETURN: return "Enter";
	case VK_ESCAPE: return "Escape";
	case VK_OEM_COMMA: return "Comma";
	case VK_OEM_PERIOD: return "Period";
	case VK_CAPITAL: return "Caps Lock";
	case VK_SCROLL: return "Scroll Lock";
	case VK_NUMLOCK: return "Num Lock";
	case VK_SNAPSHOT: return "Print Screen";
	case VK_PAUSE: return "Pause";
	case VK_LSHIFT: return "Left Shift";
	case VK_LCONTROL: return "Left Ctrl";
	case VK_LMENU: return "Left Alt";
	case VK_RSHIFT: return "Right Shift";
	case VK_RCONTROL: return "Right Ctrl";
	case VK_RMENU: return "Right Alt";
	case VK_LBUTTON: return "Mouse 1";
	case VK_RBUTTON: return "Mouse 2";
	case VK_MBUTTON: return "Mouse 3";
	case VK_XBUTTON1: return "Mouse 4";
	case VK_XBUTTON2: return "Mouse 5";
	}


	if (vk >= VK_F1 && vk <= VK_F12)
		return "F" + std::to_string(vk - VK_F1 + 1);

	if ((vk >= '0' && vk <= '9') || (vk >= 'A' && vk <= 'Z'))
		return std::string(1, (char)vk);

	char name[64];
	if (GetKeyNameTextA(MapVirtualKeyA(vk, MAPVK_VK_TO_VSC) << 16, name, sizeof(name)))
		return name;

	return "Unknown (" + std::to_string(vk) + ")";
}

void c_menu::setup_style() {
	if (m_style_initialized) return;

	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;
	float scale = m_dpi_scale;

	style.WindowRounding = 8.0f * scale;
	style.ChildRounding = 6.0f * scale;
	style.FrameRounding = 4.0f * scale;
	style.ScrollbarSize = 14.0f * scale;
	style.ScrollbarRounding = 9.0f * scale;
	style.WindowBorderSize = 1.0f * scale;
	style.FrameBorderSize = 1.0f * scale;
	style.WindowPadding = ImVec2(8, 8) * scale;
	style.WindowBorderSize = 1.0f * scale;
	style.ChildBorderSize = 1.0f * scale;
	style.PopupBorderSize = 1.0f * scale;
	style.FramePadding = ImVec2(4, 3) * scale;
	style.ItemSpacing = ImVec2(8, 4) * scale;
	style.ItemInnerSpacing = ImVec2(4, 4) * scale;
	style.CellPadding = ImVec2(4, 2) * scale;
	style.ColumnsMinSpacing = 6.0f * scale;  
	style.GrabMinSize = 12.0f * scale;

	colors[ImGuiCol_WindowBg] = ImVec4(0.98f, 0.98f, 1.00f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.96f, 0.96f, 0.98f, 1.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.96f, 0.96f, 0.98f, 1.00f);
	colors[ImGuiCol_Border] = ImVec4(0.85f, 0.85f, 0.90f, 1.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.96f, 0.96f, 0.98f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.88f, 0.85f, 0.95f, 1.00f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.85f, 0.80f, 0.92f, 1.00f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.95f, 0.95f, 0.98f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.98f, 0.98f, 1.00f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.60f, 0.40f, 0.90f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.60f, 0.40f, 0.90f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.60f, 0.40f, 0.90f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.90f, 0.88f, 0.95f, 1.00f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.85f, 0.80f, 0.95f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.60f, 0.40f, 0.90f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.92f, 0.90f, 0.98f, 1.00f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.85f, 0.80f, 0.95f, 1.00f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.60f, 0.40f, 0.90f, 1.00f);
	colors[ImGuiCol_Tab] = ImVec4(0.92f, 0.92f, 0.95f, 1.00f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.60f, 0.40f, 0.90f, 0.80f);
	colors[ImGuiCol_TabActive] = ImVec4(0.60f, 0.40f, 0.90f, 1.00f);
	colors[ImGuiCol_Text] = ImVec4(0.20f, 0.20f, 0.25f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.96f, 0.96f, 0.98f, 1.00f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.90f, 0.88f, 0.95f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.85f, 0.80f, 0.95f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.60f, 0.40f, 0.90f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.85f, 0.80f, 0.95f, 1.00f);
	style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.60f, 0.40f, 0.90f, 1.00f);

	m_style_initialized = true;
}

void get_loading_progress() {
	const auto progress = g_item_schema->get_progress();

	if (progress.m_types_count && !progress.m_skins_count) {
		ImGui::TextDisabled("Loading types: %d from %d", progress.m_types_left, progress.m_types_count);
	}
	else if (progress.m_types_count && progress.m_skins_count) {
		//ImGui::TextDisabled("Loading skins: %d from %d for %s", progress.m_skins_left, progress.m_skins_count, progress.m_weapon_name); // skins are loading too fast
		ImGui::TextDisabled("Loading skins for %s", progress.m_weapon_name);
	}
	else {
		ImGui::TextDisabled("Loading...");
	}
}

void draw_skins_combo(const char* combo_label, ImGuiTextFilter& filter, const char* filter_label, const uint16_t& selected_def_index, int& paint_kit, bool render_preview = true) {
	auto& kits = g_item_schema->item_paint_kits[selected_def_index];
	if (kits.empty()) {
		paint_kit = 0;
		return;
	}

	paint_kit = std::clamp(paint_kit, 0, (int)kits.size() - 1);

	ImGuiStyle& style = ImGui::GetStyle();
	filter.Draw(filter_label); ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ style.FramePadding.x, style.ItemSpacing.y }); ImGui::SameLine(); ImGui::Text(("Search")); ImGui::PopStyleVar();

	const char* preview_value = kits[paint_kit].name.c_str();

	if (!ImGui::BeginCombo(combo_label, preview_value))
		return;

	for (int i = 0; i < (int)kits.size(); i++) {
		const bool is_selected = (paint_kit == i);

		if (filter.PassFilter(kits[i].name.c_str())) {
			ImGui::PushID(i);

			if (ImGui::Selectable(kits[i].name.c_str(), is_selected)) {
				paint_kit = i;
			}

			const float m_dpi_scale = g_menu->get_dpi_scale();

			ImU32 kit_color = GetRarityColor(kits[i].rarity);
			ImVec2 p_min = ImGui::GetItemRectMin();
			ImVec2 p_max = ImGui::GetItemRectMax();

			const float padding{ 1.0f * m_dpi_scale };
			const float padding2{ 4.0f * m_dpi_scale };
			ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(p_min.x + padding, p_min.y + padding), ImVec2(p_min.x + padding2, p_max.y - padding), kit_color);
			
			if (ImGui::IsItemHovered() && kits[i].image != nullptr && render_preview) {
				const float spacing = style.ItemSpacing.x;
				const static float rounding = 0.f;

				const ImVec2 image_size_raw = kits[i].image->GetImageSize();
				//const ImVec2 image_size_raw {512, 384};
				if (kits[i].image->GetNativeTexture() != nullptr && image_size_raw.x != 0 && image_size_raw.y != 0) {

					const ImVec2 window_position = ImGui::GetWindowPos();
					const ImVec2 window_size = ImGui::GetWindowSize();
					const ImVec2 image_size = ImVec2(window_size.y * image_size_raw.x / image_size_raw.y, window_size.y);

					ImRect hint_bb(window_position + ImVec2(spacing + window_size.x, 0), window_position + ImVec2(window_size.x + image_size.x + spacing, image_size.y));

					ImGui::PushClipRect(hint_bb.Min, hint_bb.Max, true);
					ImGui::GetForegroundDrawList()->AddRectFilled(hint_bb.Min, hint_bb.Max, ImGui::GetColorU32(ImGuiCol_ChildBg), rounding);
					ImGui::GetForegroundDrawList()->AddRect(hint_bb.Min, hint_bb.Max, ImGui::GetColorU32(ImGuiCol_Border), rounding);
					ImGui::GetForegroundDrawList()->AddImage((ImTextureID)kits[i].image->GetNativeTexture(), hint_bb.Min, hint_bb.Max);
					ImGui::PopClipRect();
				}
			}

			if (is_selected) ImGui::SetItemDefaultFocus();

			ImGui::PopID();
		}
	}
	ImGui::EndCombo();
}

static void draw_skins_tab() {
	ImGui::Text("Knife Changer");
	ImGui::Separator();
	ImGui::Checkbox("Enabled##knife", &g_cfg->knife_changer.m_enabled);

	if (g_cfg->knife_changer.m_enabled) {
		static int last_knife = 0;
		static ImGuiTextFilter filter;
		uint16_t selected_knife = 0;

		const bool is_parsed = g_item_schema->is_initialized() && !g_item_schema->knife_names_cstr.empty();

		if (is_parsed) {
			g_cfg->knife_changer.m_knife = std::clamp(g_cfg->knife_changer.m_knife, 0, (int)g_item_schema->knife_names_cstr.size() - 1);

			ImGui::Combo("Knife Model", &g_cfg->knife_changer.m_knife,
				g_item_schema->knife_names_cstr.data(),
				(int)g_item_schema->knife_names_cstr.size());

			selected_knife = g_item_schema->knives[g_cfg->knife_changer.m_knife].definition_index;

			if (last_knife != g_cfg->knife_changer.m_knife) {
				g_cfg->knife_changer.m_paint_kit = 0;
				last_knife = g_cfg->knife_changer.m_knife;
			}

			const bool are_settings_visible = selected_knife > 0 && selected_knife != WEAPON_KNIFE_T && selected_knife != WEAPON_KNIFE && g_cfg->knife_changer.m_knife;

			if (are_settings_visible) {

				draw_skins_combo("Knife Skin", filter, "##FilterKnife", selected_knife, g_cfg->knife_changer.m_paint_kit);

				if (g_cfg->knife_changer.m_paint_kit) {
					static float temp_wear = g_cfg->knife_changer.m_wear;
					float item_width = ImGui::CalcItemWidth();
					float half_width = (item_width - ImGui::GetStyle().ItemSpacing.x) * 0.5f;

					const float group_w = ImGui::GetCurrentWindow()->Size.x - ImGui::GetStyle().ScrollbarSize;

					ImGui::SetNextItemWidth(half_width);
					if (ImGui::SliderFloat("##Wear", &temp_wear, 0.0f, 1.0f, "%.4f")) {
						if (!ImGui::IsMouseDown(0))
							g_cfg->knife_changer.m_wear = temp_wear;
					}
					if (!ImGui::IsItemActive() && temp_wear != g_cfg->knife_changer.m_wear)
						g_cfg->knife_changer.m_wear = temp_wear;
					ImGui::SameLine();
					ImGui::SetNextItemWidth(half_width);
					ImGui::InputInt("Wear / Seed", &g_cfg->knife_changer.m_seed, 0, 0);
					ImGui::InputText("Custom Name", g_cfg->knife_changer.m_custom_name,
						sizeof(g_cfg->knife_changer.m_custom_name));

					ImGui::Checkbox("Enable Custom Color##Knife", &g_cfg->knife_changer.m_custom_color);
					if (g_cfg->knife_changer.m_custom_color)
					{
						ImGui::SameLine(group_w - ImGui::GetFrameHeight() - ImGui::GetStyle().FramePadding.x);
						ImGui::ColorEdit4(("##modulate1knife"), g_cfg->knife_changer.m_color1, no_alpha);
						ImGui::SameLine(group_w - (ImGui::GetFrameHeight() + ImGui::GetStyle().FramePadding.x) * 2);
						ImGui::ColorEdit4(("##modulate2knife"), g_cfg->knife_changer.m_color2, no_alpha);
						ImGui::SameLine(group_w - (ImGui::GetFrameHeight() + ImGui::GetStyle().FramePadding.x) * 3);
						ImGui::ColorEdit4(("##modulate3knife"), g_cfg->knife_changer.m_color3, no_alpha);
						ImGui::SameLine(group_w - (ImGui::GetFrameHeight() + ImGui::GetStyle().FramePadding.x) * 4);
						ImGui::ColorEdit4(("##modulate4knife"), g_cfg->knife_changer.m_color4, no_alpha);
					}
				}
			}
		}
		else {
			get_loading_progress();
		}
	}

	ImGui::Spacing();
	ImGui::Text("Glove Changer");
	ImGui::Separator();
	ImGui::Checkbox("Enabled##glove", &g_cfg->glove_changer.m_enabled);

	if (g_cfg->glove_changer.m_enabled) {
		static int last_glove = 0;
		static ImGuiTextFilter filter;
		uint16_t selected_glove = 0;

		const bool is_parsed = g_item_schema->is_initialized() && !g_item_schema->glove_names_cstr.empty();

		if (is_parsed) {
			g_cfg->glove_changer.m_glove = std::clamp(g_cfg->glove_changer.m_glove, 0, (int)g_item_schema->glove_names_cstr.size() - 1);

			ImGui::Combo("Glove Model", &g_cfg->glove_changer.m_glove,
				g_item_schema->glove_names_cstr.data(),
				(int)g_item_schema->glove_names_cstr.size());

			selected_glove = g_item_schema->gloves[g_cfg->glove_changer.m_glove].definition_index;

			if (last_glove != g_cfg->glove_changer.m_glove) {
				auto& glove_skins = g_item_schema->get_paint_kit_names_for_item(selected_glove);
				g_cfg->glove_changer.m_paint_kit = (glove_skins.size() > 1) ? 1 : 0;
				last_glove = g_cfg->glove_changer.m_glove;
			}
			if (selected_glove) {

				draw_skins_combo("Glove Skin", filter, "##FilterGlove", selected_glove, g_cfg->glove_changer.m_paint_kit);

				static float temp_glove_wear = g_cfg->glove_changer.m_wear;
				float glove_item_width = ImGui::CalcItemWidth();
				float glove_half_width = (glove_item_width - ImGui::GetStyle().ItemSpacing.x) * 0.5f;

				const float group_w = ImGui::GetCurrentWindow()->Size.x - ImGui::GetStyle().ScrollbarSize;

				ImGui::SetNextItemWidth(glove_half_width);
				if (ImGui::SliderFloat("##GloveWear", &temp_glove_wear, 0.0f, 1.0f, "%.4f")) {
					if (!ImGui::IsMouseDown(0))
						g_cfg->glove_changer.m_wear = temp_glove_wear;
				}
				if (!ImGui::IsItemActive() && temp_glove_wear != g_cfg->glove_changer.m_wear)
					g_cfg->glove_changer.m_wear = temp_glove_wear;
				ImGui::SameLine();
				ImGui::SetNextItemWidth(glove_half_width);
				ImGui::InputInt("Wear / Seed##glove", &g_cfg->glove_changer.m_seed, 0, 0);

				//ImGui::Checkbox("Enable Custom Color##Glove", &g_cfg->glove_changer.m_custom_color);
				//if (g_cfg->glove_changer.m_custom_color)
				//{
				//	ImGui::SameLine(group_w - ImGui::GetFrameHeight() - ImGui::GetStyle().FramePadding.x);
				//	ImGui::ColorEdit4(("##modulate1glove"), g_cfg->glove_changer.m_color1, no_alpha);
				//	ImGui::SameLine(group_w - (ImGui::GetFrameHeight() + ImGui::GetStyle().FramePadding.x) * 2);
				//	ImGui::ColorEdit4(("##modulate2glove"), g_cfg->glove_changer.m_color2, no_alpha);
				//	ImGui::SameLine(group_w - (ImGui::GetFrameHeight() + ImGui::GetStyle().FramePadding.x) * 3);
				//	ImGui::ColorEdit4(("##modulate3glove"), g_cfg->glove_changer.m_color3, no_alpha);
				//	ImGui::SameLine(group_w - (ImGui::GetFrameHeight() + ImGui::GetStyle().FramePadding.x) * 4);
				//	ImGui::ColorEdit4(("##modulate4glove"), g_cfg->glove_changer.m_color4, no_alpha);
				//}
			}
			if (ImGui::Button("Apply Gloves##gloves_apply")) {
				g_glove_changer->should_update = true;
			}
		}
		else {
			get_loading_progress();
		}
	}

	ImGui::Spacing();
	ImGui::Text("Skin Changer");
	ImGui::Separator();
	ImGui::Checkbox("Enabled##skin", &g_cfg->skin_changer.m_enabled);

	if (g_cfg->skin_changer.m_enabled) {
		static ImGuiTextFilter filter;
		uint16_t selected_weapon_def = 0;
		const bool is_parsed = g_item_schema->is_initialized() && !g_item_schema->weapon_names_cstr.empty();

		if (is_parsed) {
			g_cfg->skin_changer.m_selected_weapon = std::clamp(g_cfg->skin_changer.m_selected_weapon, 0, (int)g_item_schema->weapon_names_cstr.size() - 1);

			ImGui::Combo("Weapon", &g_cfg->skin_changer.m_selected_weapon,
				g_item_schema->weapon_names_cstr.data(),
				(int)g_item_schema->weapon_names_cstr.size());

			selected_weapon_def = g_item_schema->weapons[g_cfg->skin_changer.m_selected_weapon].definition_index;

			if (selected_weapon_def) {
				int config_index = c_config::skin_changer_t::get_config_index(selected_weapon_def);
				auto& weapon_skin = g_cfg->skin_changer.weapon_skins[config_index];

				draw_skins_combo("Skin##weapon_skin", filter, "##FilterSkins", selected_weapon_def, weapon_skin.paint_kit);

				if (weapon_skin.paint_kit) {
					float weapon_item_width = ImGui::CalcItemWidth();
					float weapon_half_width = (weapon_item_width - ImGui::GetStyle().ItemSpacing.x) * 0.5f;

					const float group_w = ImGui::GetCurrentWindow()->Size.x - ImGui::GetStyle().ScrollbarSize;

					ImGui::SetNextItemWidth(weapon_half_width);
					ImGui::SliderFloat("##WeaponWear", &weapon_skin.wear, 0.0f, 1.0f, "%.4f");
					ImGui::SameLine();
					ImGui::SetNextItemWidth(weapon_half_width);
					ImGui::InputInt("Wear / Seed##weapon", &weapon_skin.seed, 0, 0);
					ImGui::InputText("Name##weapon_name", weapon_skin.custom_name,
						sizeof(weapon_skin.custom_name));

					ImGui::Checkbox("Enable Custom Color##Skin", &weapon_skin.m_custom_color);
					if (weapon_skin.m_custom_color)
					{
						ImGui::SameLine(group_w - ImGui::GetFrameHeight() - ImGui::GetStyle().FramePadding.x);
						ImGui::ColorEdit4(("##modulate1skin"), weapon_skin.m_color1, no_alpha);
						ImGui::SameLine(group_w - (ImGui::GetFrameHeight() + ImGui::GetStyle().FramePadding.x) * 2);
						ImGui::ColorEdit4(("##modulate2skin"), weapon_skin.m_color2, no_alpha);
						ImGui::SameLine(group_w - (ImGui::GetFrameHeight() + ImGui::GetStyle().FramePadding.x) * 3);
						ImGui::ColorEdit4(("##modulate3skin"), weapon_skin.m_color3, no_alpha);
						ImGui::SameLine(group_w - (ImGui::GetFrameHeight() + ImGui::GetStyle().FramePadding.x) * 4);
						ImGui::ColorEdit4(("##modulate4skin"), weapon_skin.m_color4, no_alpha);
					}
				}
			}
			if (ImGui::Button("Apply Skins##skin_apply")) {
				g_skin_changer->should_update = true;
			}
		}
		else {
			get_loading_progress();
		}
	}

	ImGui::Spacing();
	ImGui::Text("Agent Changer");
	ImGui::Separator();
	ImGui::Checkbox("Enabled##agent", &g_cfg->agent_changer.m_enabled);

	if (g_cfg->agent_changer.m_enabled)
	{
		static ImGuiTextFilter filter;
		if (g_item_schema->is_initialized() && !g_item_schema->agent_names_cstr.empty())
		{
			ImGuiStyle& style = ImGui::GetStyle();
			filter.Draw("##FilterAgents"); ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ style.FramePadding.x, style.ItemSpacing.y }); ImGui::SameLine(); ImGui::Text(("Search")); ImGui::PopStyleVar();

			g_cfg->agent_changer.m_agent = std::clamp(g_cfg->agent_changer.m_agent, 0, (int)g_item_schema->agent_names_cstr.size() - 1);
			const char* preview_value = g_item_schema->agent_names_cstr[g_cfg->agent_changer.m_agent];

			if (ImGui::BeginCombo("Agent Model", preview_value)) {
				for (int i = 0; i < (int)g_item_schema->agent_names_cstr.size(); i++) {
					const bool is_selected = (g_cfg->agent_changer.m_agent == i);

					if (filter.PassFilter(g_item_schema->agent_names_cstr[i])) {
						ImGui::PushID(i);

						if (ImGui::Selectable(g_item_schema->agent_names_cstr[i], is_selected)) {
							g_cfg->agent_changer.m_agent = i;
						}

						// agent image l8r

						if (ImGui::IsItemHovered() && g_item_schema->agents[i].image != nullptr) {
							const float spacing = style.ItemSpacing.x;
							const static float rounding = 0.f;

							const ImVec2 image_size_raw = g_item_schema->agents[i].image->GetImageSize();
							//const ImVec2 image_size_raw {512, 384};
							if (g_item_schema->agents[i].image->GetNativeTexture() != nullptr && image_size_raw.x != 0 && image_size_raw.y != 0) {

								const ImVec2 window_position = ImGui::GetWindowPos();
								const ImVec2 window_size = ImGui::GetWindowSize();
								const ImVec2 image_size = ImVec2(window_size.y * image_size_raw.x / image_size_raw.y, window_size.y);

								ImRect hint_bb(window_position + ImVec2(spacing + window_size.x, 0), window_position + ImVec2(window_size.x + image_size.x + spacing, image_size.y));

								ImGui::PushClipRect(hint_bb.Min, hint_bb.Max, true);
								ImGui::GetForegroundDrawList()->AddRectFilled(hint_bb.Min, hint_bb.Max, ImGui::GetColorU32(ImGuiCol_ChildBg), rounding);
								ImGui::GetForegroundDrawList()->AddRect(hint_bb.Min, hint_bb.Max, ImGui::GetColorU32(ImGuiCol_Border), rounding);
								ImGui::GetForegroundDrawList()->AddImage((ImTextureID)g_item_schema->agents[i].image->GetNativeTexture(), hint_bb.Min, hint_bb.Max);
								ImGui::PopClipRect();
							}
						}

						if (is_selected)
							ImGui::SetItemDefaultFocus();

						ImGui::PopID();
					}
				}
				ImGui::EndCombo();
			}
		}
		else
		{
			get_loading_progress();
		}
	}

	ImGui::Spacing();
	ImGui::Text("Player Model Changer");
	ImGui::Separator();
	ImGui::Checkbox("Enabled##custom_model", &g_cfg->custom_model.m_enabled);
	if (g_cfg->custom_model.m_enabled) {
		if (g_item_schema->is_initialized()) {
			ImGui::Text("Player Model Path");
			ImGui::InputText("##CustomModel", &g_cfg->custom_model.m_model_path);
			if (ImGui::Button("Apply Model"))
			{
				g_cfg->custom_model.m_should_update = true;
			}

			ImGui::Text("Mesh groups(ThirdPerson)");
			ImGui::MultiCombo(("##MeshgroupsTP"), g_agent_changer->custom_model_groups.MeshNames.data(), g_agent_changer->custom_model_groups.MeshBools, g_agent_changer->custom_model_groups.MeshNames.size());
			ImGui::Text("Mesh groups(FirstPerson)");
			ImGui::MultiCombo(("##MeshgroupsFP"), g_agent_changer->custom_viewmodel_groups.MeshNames.data(), g_agent_changer->custom_viewmodel_groups.MeshBools, g_agent_changer->custom_viewmodel_groups.MeshNames.size());

		}
		else {
			get_loading_progress();
		}
	}
}

static void draw_config_tab(float scale) {
	ImGui::Text("Config System");
	ImGui::Separator();

	g_config_system->refresh();
	auto& configs = g_config_system->get_config_files();

	ImGui::Text("Saved Configs:");
	ImGui::BeginChild("config_list", ImVec2(0, 150.0f * scale), true);
	{
		for (const auto& cfg : configs) {
			bool is_selected = (g_config_system->m_selected_config == cfg);
			if (ImGui::Selectable(cfg.c_str(), is_selected))
				g_config_system->m_selected_config = cfg;
		}
	}
	ImGui::EndChild();

	static char config_name[64] = "";
	ImGui::InputText("Config Name", config_name, sizeof(config_name));

	ImGui::Spacing();

	if (ImGui::Button("Create", ImVec2(80.0f * scale, 0))) {
		if (strlen(config_name) > 0) {
			g_config_system->save(config_name);
			g_config_system->m_selected_config = config_name;
			config_name[0] = '\0';
		}
	}
	ImGui::SameLine();

	if (ImGui::Button("Save", ImVec2(80.0f * scale, 0))) {
		if (!g_config_system->m_selected_config.empty())
			g_config_system->save(g_config_system->m_selected_config);
	}
	ImGui::SameLine();

	if (ImGui::Button("Load", ImVec2(80.0f * scale, 0))) {
		if (!g_config_system->m_selected_config.empty())
			g_config_system->load(g_config_system->m_selected_config);
	}

	if (ImGui::Button("Delete", ImVec2(80.0f * scale, 0))) {
		if (!g_config_system->m_selected_config.empty()) {
			g_config_system->remove(g_config_system->m_selected_config);
			g_config_system->m_selected_config.clear();
		}
	}
	ImGui::SameLine();

	if (ImGui::Button("Reset", ImVec2(80.0f * scale, 0)))
		g_config_system->reset();
	ImGui::SameLine();

	if (ImGui::Button("Refresh", ImVec2(80.0f * scale, 0)))
		g_config_system->refresh();
}

static void draw_visauls_tab()
{
	ImGui::Text("Visuals");
	ImGui::Separator();

	ImGui::Checkbox("Remove Smoke", &g_cfg->visuals.m_enable_smoke);
	ImGui::Checkbox("Remove Flash", &g_cfg->visuals.m_enable_draw_flashbang);
	ImGui::Checkbox("Remove Legs", &g_cfg->visuals.m_enable_draw_legs);

	ImGui::Text("World");
	ImGui::Separator();

	ImGui::Checkbox(("Change Shadow Dir##shadows"), &g_cfg->visuals.m_change_shadow_dir);
	ImGui::BeginDisabled(!g_cfg->visuals.m_change_shadow_dir);
	{
		ImGui::Text("Shadow Pitch");
		ImGui::SliderFloat(("##ShadowPitch"), &g_cfg->visuals.m_shadow_pitch, -360.f, 360.f);
		ImGui::Text("Shadow Yaw");
		ImGui::SliderFloat(("##ShadowYaw"), &g_cfg->visuals.m_shadow_yaw, -360.f, 360.f);
		ImGui::Text("Shadow Roll");
		ImGui::SliderFloat(("##ShadowRoll"), &g_cfg->visuals.m_shadow_roll, -360.f, 360.f);
	}
	ImGui::EndDisabled();

	ImGui::Checkbox("Change Light Color##lightcolor", &g_cfg->visuals.m_change_color_light);
	if (g_cfg->visuals.m_change_color_light)
	{
		ImGui::SameLine(); ImGui::ColorEdit4("##LightColor", g_cfg->visuals.m_color_light, with_alpha);
	}
}

bool g_waiting_for_key = false;

static void draw_settings_tab() {
	ImGui::Text("Settings");
	ImGui::Separator();

	ImGui::Spacing();
	ImGui::Text("Menu key");
	ImGui::Separator();

	std::string btn_label = g_waiting_for_key ?
		"Press a key..." :
		get_key_name(g_cfg->misc.m_menu_key);

	if (ImGui::Button(btn_label.c_str(), ImVec2(200, 0))) {
		g_waiting_for_key = true;
	}
}

void c_menu::draw() {
	if (!m_opened)
		return;

	setup_style();

	const float scale = m_dpi_scale;
	const ImVec2 window_size(m_main_window_size.x * scale, m_main_window_size.y * scale);

	ImGui::SetNextWindowSize(window_size);
	ImGui::Begin("Nerv", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

	m_main_window_pos = ImGui::GetWindowPos();

	ImGui::BeginChild("tabs", ImVec2(100.0f * scale, 0), true);
	{
		static constexpr const char* tabs[]{ "Skins", "Visuals", "Config","Settings" };

		for (std::size_t i = 0; i < IM_ARRAYSIZE(tabs); ++i) {
			if (ImGui::Selectable(tabs[i], m_selected_tab == static_cast<int>(i)))
				m_selected_tab = static_cast<int>(i);
		}
	}
	ImGui::EndChild();
	ImGui::SameLine();

	if (m_selected_tab == 0) {
		ImGui::BeginChild("skins_content", ImVec2(0, 0), true);
		draw_skins_tab();
		ImGui::EndChild();
	}

	if (m_selected_tab == 1) {
		ImGui::BeginChild("visuals_content", ImVec2(0, 0), true);
		draw_visauls_tab();
		ImGui::EndChild();
	}

	if (m_selected_tab == 2) {
		ImGui::BeginChild("config_content", ImVec2(0, 0), true);
		draw_config_tab(scale);
		ImGui::EndChild();
	}

	if (m_selected_tab == 3) {
		ImGui::BeginChild("settings_content", ImVec2(0, 0), true);
		draw_settings_tab();
		ImGui::EndChild();
	}

	ImGui::End();
}