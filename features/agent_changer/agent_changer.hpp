#pragma once

#include "../shared/data.hpp"
#include "../../main.hpp"
#include "../../valve/classes/c_cs_player_pawn.hpp"
#include "../shared/item_schema.hpp"

struct CustomModelVectors {
	std::vector<const char*> MeshNames;
	bool MeshBools[128];

	bool IsEmpty()
	{
		if (this->MeshNames.empty())
			return true;

		return false;
	}

	size_t GetSize()
	{
		return this->MeshNames.size();
	}

	void Clear()
	{
		std::memset(MeshBools, false, sizeof(MeshBools));
		this->MeshNames.clear();
	}
};

class c_agent_changer {
public:
	void run(c_cs_player_pawn* observer_target, engine_data current);
	bool should_update = false;

	CustomModelVectors custom_model_groups;
private:
	void custom_model(c_cs_player_pawn* observer_target, engine_data current);
	void agent_model(c_cs_player_pawn* observer_target, engine_data current);

	c_config::agent_changer_t agent_backup_cfg;
	engine_data backup_engine_data;

	int m_update_frames = 0;
	uint64_t saved_last_model = 0ULL;
};

inline const auto g_agent_changer = std::make_unique<c_agent_changer>();
