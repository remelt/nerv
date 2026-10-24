#pragma once
#include "../../main.hpp"

#include <filesystem>

inline const char* SkyBoxNames[] = {
    "none",
    "cs_italy",
    "cs_italy_fog",
    "cs_italy_lighting",
    "cs_office",
    "rush_cloudy",
    "ar_baggage",
    "cs15_daylight01",
    "csgo_cloudy01",
    "de_annubis",
    "de_dust2",
    "de_mirage",
    "de_nuke",
    "de_overpass_01",
    "de_train03",
    "de_vertigo",
    "hr_aztec_02",
    "overcast_01",
    "rain_night"
};

inline const char* SkyBoxPaths[] = {
    "none",
    "materials/skybox/cs_italy_s2_skybox_2.vmat",
    "materials/skybox/cs_italy_s2_skybox_2_fog.vmat",
    "materials/skybox/cs_italy_s2_skybox_2_lighting.vmat",
    "materials/skybox/cs_office_45_0.vmat",
    "materials/skybox/rush_cloudy_01a.vmat",
    "materials/skybox/sky_ar_baggage_01.vmat",
    "materials/skybox/sky_cs15_daylight01_hdr.vmat",
    "materials/skybox/sky_csgo_cloudy01.vmat",
    "materials/skybox/sky_de_annubis.vmat",
    "materials/skybox/sky_de_dust2.vmat",
    "materials/skybox/sky_de_mirage.vmat",
    "materials/skybox/sky_de_nuke.vmat",
    "materials/skybox/sky_de_overpass_01.vmat",
    "materials/skybox/sky_de_train03.vmat",
    "materials/skybox/sky_de_vertigo.vmat",
    "materials/skybox/sky_hr_aztec_02_v1.vmat",
    "materials/skybox/sky_overcast_01.vmat",
    "materials/skybox/sky_rain_night_01.vmat"
};

struct CSkyBoxClass
{
    std::vector<std::string> SkyBoxName;
    std::vector<std::string> SkyBoxPath;

    void emplace(const std::string& name, const std::string& path) {
        SkyBoxName.push_back(name);
        SkyBoxPath.push_back(path);
    }

    size_t GetSize()
    {
        return this->SkyBoxName.size();
    }

};

class c_visuals {
public:
    void skybox_changer();
    void skybox_parser();

    CSkyBoxClass SkyBoxesArray;
    bool should_update = false;

private:
    std::filesystem::path get_skybox_folder();
};

inline const auto g_visuals = std::make_unique<c_visuals>();