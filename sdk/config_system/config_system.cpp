#include "config_system.hpp"
#include "../../config.hpp"
#include "../../utils/utils.hpp"
#include "../../features/skin_changer/skin_changer.hpp"
#include "../../features/glove_changer/glove_changer.hpp"
#include "../includes/xor.hpp"
#include "../console/console.hpp"
#include <iomanip>

void c_config_system::push_item(bool* pointer, const std::string& category, const std::string& name, bool default_value) {
    m_booleans.push_back({ pointer, category, name, default_value });
}

void c_config_system::push_item(int* pointer, const std::string& category, const std::string& name, int default_value) {
    m_ints.push_back({ pointer, category, name, default_value });
}

void c_config_system::push_item(float* pointer, const std::string& category, const std::string& name, float default_value) {
    m_floats.push_back({ pointer, category, name, default_value });
}

void c_config_system::push_item(std::string* pointer, const std::string& category, const std::string& name, const std::string& default_value) {
    m_strings.push_back({ pointer, category, name, default_value });
}

void c_config_system::push_char_array(char* pointer, size_t size, const std::string& category, const std::string& name) {
    m_char_arrays.push_back({ pointer, size, category, name });
}

void c_config_system::setup_values() {

    std::filesystem::create_directories(m_config_path);

    push_item(&g_cfg->knife_changer.m_enabled, "knife_changer", "enabled", false);
    push_item(&g_cfg->knife_changer.m_knife, "knife_changer", "knife", 0);
    push_item(&g_cfg->knife_changer.m_paint_kit, "knife_changer", "paint_kit", 0);
    push_item(&g_cfg->knife_changer.m_wear, "knife_changer", "wear", 0.0001f);
    push_item(&g_cfg->knife_changer.m_seed, "knife_changer", "seed", 0);
    push_char_array(g_cfg->knife_changer.m_custom_name, sizeof(g_cfg->knife_changer.m_custom_name), "knife_changer", "custom_name");
    push_item(&g_cfg->knife_changer.m_custom_color, "knife_changer", "custom_color", false);

    push_item(&g_cfg->glove_changer.m_enabled, "glove_changer", "enabled", false);
    push_item(&g_cfg->glove_changer.m_glove, "glove_changer", "glove", 0);
    push_item(&g_cfg->glove_changer.m_paint_kit, "glove_changer", "paint_kit", 0);
    push_item(&g_cfg->glove_changer.m_wear, "glove_changer", "wear", 0.0001f);
    push_item(&g_cfg->glove_changer.m_seed, "glove_changer", "seed", 0);
    push_item(&g_cfg->glove_changer.m_custom_color, "glove_changer", "custom_color", false);

    push_item(&g_cfg->agent_changer.m_enabled, "agent_changer", "enabled", false);
    push_item(&g_cfg->agent_changer.m_agent, "agent_changer", "agent", 0);

    push_item(&g_cfg->custom_model.m_enabled, "custom_model", "enabled", false);
    push_item(&g_cfg->custom_model.m_should_update, "custom_model", "should_update", false);
    push_item(&g_cfg->custom_model.m_model_path, "custom_model", "model_path", "agents/models/tm_phoenix/tm_phoenix.vmdl");

    push_item(&g_cfg->skin_changer.m_enabled, "skin_changer", "enabled", false);
    push_item(&g_cfg->skin_changer.m_selected_weapon, "skin_changer", "selected_weapon", 0);

    for (int i = 0; i < 100; i++) {
        std::string prefix = "weapon_" + std::to_string(i);
        push_item(&g_cfg->skin_changer.weapon_skins[i].paint_kit, "skin_changer", prefix + "_paint_kit", 0);
        push_item(&g_cfg->skin_changer.weapon_skins[i].wear, "skin_changer", prefix + "_wear", 0.0001f);
        push_item(&g_cfg->skin_changer.weapon_skins[i].seed, "skin_changer", prefix + "_seed", 0);
        push_char_array(g_cfg->skin_changer.weapon_skins[i].custom_name, sizeof(g_cfg->skin_changer.weapon_skins[i].custom_name), "skin_changer", prefix + "_custom_name");
        push_item(&g_cfg->skin_changer.weapon_skins[i].m_custom_color, "skin_changer", prefix + "_custom_color", false);
    }

    push_item(&g_cfg->visuals.m_enable_smoke, "visuals", "remove_smoke", false);
    push_item(&g_cfg->visuals.m_enable_draw_flashbang, "visuals", "remove_flash", false);

    push_item(&g_cfg->visuals.m_enable_draw_legs, "visuals", "draw_legs", false);
    push_item(&g_cfg->visuals.m_change_shadow_dir, "visuals", "change_shadow_dir", false);
    push_item(&g_cfg->visuals.m_shadow_pitch, "visuals", "shadow_pitch", 0.f);
    push_item(&g_cfg->visuals.m_shadow_yaw, "visuals", "shadow_yaw", 0.f);
    push_item(&g_cfg->visuals.m_shadow_roll, "visuals", "shadow_roll", 0.f);
    push_item(&g_cfg->visuals.m_change_color_light, "visuals", "change_color_light", false);

    push_item(&g_cfg->misc.m_menu_key, "misc", "menu_key", 0x2D);

    refresh();
}

void save_last_state(const std::string& config_name)
{
    std::filesystem::create_directories("C:\\Nerv-Fix\\");
    nlohmann::json state;
    state["last_config"] = config_name;
    state["menu_key"] = g_cfg->misc.m_menu_key;

    std::ofstream file("C:\\Nerv-Fix\\state.json");
    if (file.is_open()) {
        file << state.dump(4);
        file.close();
    }
}

void c_config_system::save(const std::string& name)
{
    if (name.empty())
        return;


    std::filesystem::create_directories(m_config_path);

    std::string path = m_config_path + name + ".cfg";

    json data = json::object();

    auto ensure_category = [&](const std::string& category)
        {
            if (data.find(category) == data.end())
                data[category] = json::object();
        };

    for (auto& item : m_booleans)
    {
        ensure_category(item.category);
        data[item.category][item.name] = *item.pointer;
    }

    for (auto& item : m_ints)
    {
        ensure_category(item.category);
        data[item.category][item.name] = *item.pointer;
    }

    for (auto& item : m_floats)
    {
        ensure_category(item.category);
        data[item.category][item.name] = *item.pointer;
    }

    for (auto& item : m_strings)
    {
        ensure_category(item.category);
        data[item.category][item.name] = *item.pointer;
    }

    for (auto& item : m_char_arrays)
    {
        ensure_category(item.category);
        data[item.category][item.name] = std::string(item.pointer);
    }

    std::ofstream file(path);
    if (file.is_open())
    {
        file << std::setw(4) << data << std::endl;
        file.close();
    }
    printf("[Config] Save called on instance: %p\n", this);
    refresh();
}

void c_config_system::load(const std::string& name)
{
    if (name.empty())
        return;

    std::string path = m_config_path + name + ".cfg";

    if (!std::filesystem::exists(path))
        return;

    std::ifstream file(path);
    if (!file.is_open())
        return;

    json data;
    try
    {
        file >> data;
    }
    catch (...)
    {
        LOG_ERROR(xorstr_("[config] failed to parse: %s"), name.c_str());
        file.close();
        return;
    }
    file.close();

    auto has_key = [&](const std::string& category, const std::string& key) -> bool
        {
            return data.find(category) != data.end() && data[category].find(key) != data[category].end();
        };

    for (auto& item : m_booleans)
    {
        if (has_key(item.category, item.name))
            *item.pointer = data[item.category][item.name].get<bool>();
    }

    for (auto& item : m_ints)
    {
        if (has_key(item.category, item.name))
            *item.pointer = data[item.category][item.name].get<int>();
    }

    for (auto& item : m_floats)
    {
        if (has_key(item.category, item.name))
            *item.pointer = data[item.category][item.name].get<float>();
    }

    for (auto& item : m_strings)
    {
        if (has_key(item.category, item.name))
            *item.pointer = data[item.category][item.name].get<std::string>();
    }

    for (auto& item : m_char_arrays)
    {
        if (has_key(item.category, item.name))
        {
            std::string value = data[item.category][item.name].get<std::string>();
            strncpy_s(item.pointer, item.size, value.c_str(), _TRUNCATE);
        }
    }

    g_skin_changer->should_update = true;
    g_glove_changer->should_update = true;
    save_last_state(name);
}

void c_config_system::remove(const std::string& name)
{
    if (name.empty())
        return;

    std::string path = m_config_path + name + ".cfg";

    if (std::filesystem::exists(path))
        std::filesystem::remove(path);

    refresh();
}

void c_config_system::reset()
{

    for (auto& item : m_booleans)
        *item.pointer = item.default_value;

    for (auto& item : m_ints)
        *item.pointer = item.default_value;

    for (auto& item : m_floats)
        *item.pointer = item.default_value;

    for (auto& item : m_strings)
        *item.pointer = item.default_value;

    for (auto& item : m_char_arrays)
        memset(item.pointer, 0, item.size);
}

void c_config_system::refresh()
{
    m_config_files.clear();

    if (!std::filesystem::exists(m_config_path))
        return;

    for (const auto& entry : std::filesystem::directory_iterator(m_config_path))
    {
        if (!entry.is_regular_file())
            continue;

        if (entry.path().extension() != ".cfg")
            continue;

        std::string filename = entry.path().stem().string();
        m_config_files.push_back(filename);
    }
}

void c_config_system::auto_load()
{
  

    std::string state_path = "C:\\Nerv-Fix\\state.json";
    if (!std::filesystem::exists(state_path)) return;

    try {
        std::ifstream file(state_path);
        nlohmann::json state;
        file >> state;

        /*if (state.contains("menu_key")) {
            g_cfg->misc.m_menu_key = state["menu_key"].get<int>();
        }*/

        if (state.contains("last_config")) {
            std::string last_cfg = state["last_config"].get<std::string>();
            load(last_cfg); 
        }
    }
    catch (...) {
       
    }
}

void c_config_system::auto_load_key()
{


    std::string state_path = "C:\\Nerv-Fix\\state.json";
    if (!std::filesystem::exists(state_path)) return;

    try {
        std::ifstream file(state_path);
        nlohmann::json state;
        file >> state;

        if (state.contains("menu_key")) {
            g_cfg->misc.m_menu_key = state["menu_key"].get<int>();
        }

        
    }
    catch (...) {

    }
}

void c_config_system::auto_save()
{
    save(m_default_config);
}
