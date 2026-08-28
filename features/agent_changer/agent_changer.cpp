#include "agent_changer.hpp"
#include "../shared/econ_item_attribute_manager.hpp"
#include "../../valve/interfaces/interfaces.hpp"

void c_agent_changer::custom_model(c_cs_player_pawn* observer_target, engine_data current)
{
	c_model_state& p_model_state = observer_target->m_scene_node()->get_skeleton_instance()->m_model_state();
	if (!custom_model_groups.IsEmpty() && p_model_state.get_model())
	{
		std::uint64_t mesh_group = 0;
		const std::uint64_t mesh_mask_group = p_model_state.m_MeshGroupMask();

		for (int i = 0; i < custom_model_groups.GetSize(); i++)
		{
			if (custom_model_groups.MeshBools[i])
				mesh_group |= 1ULL << i;
		}

		if (mesh_group != mesh_mask_group)
		{
			observer_target->m_scene_node()->set_mesh_group_mask(mesh_group);
		}
	}

	if (g_cfg->custom_model.m_model_path.empty() || g_cfg->agent_changer.m_enabled || !g_cfg->custom_model.m_enabled)
		return;

	const char* szPathModel = g_cfg->custom_model.m_model_path.c_str();

	bool should_update_model = p_model_state.get_model() ? (strcmp(observer_target->m_scene_node()->get_skeleton_instance()->m_model_state().m_ModelName().m_pString, szPathModel) != 0) : true;

	if (!g_cfg->custom_model.m_should_update && !current.state_changed(backup_engine_data) && !should_update_model /*simple workaraound, idk how to actually fix it rn. prob its possible to check the model name and compare it -> https://www.unknowncheats.me/forum/counter-strike-2-a/606375-reading-player-model-name.html*/)
		return;

	std::uint64_t uPlayerModelHash = fnv1a::hash_64(szPathModel);

	g_interfaces->m_resource_system->BlockingLoadResourceByName(szPathModel, 0);

	observer_target->set_model(szPathModel);

	c_model* p_model_handle = observer_target->m_scene_node()->get_skeleton_instance()->m_model_state().get_model();
	if(p_model_handle && uPlayerModelHash != saved_last_model)
	{
		custom_model_groups.Clear();

		const std::uint64_t active_mesh_mask_group = observer_target->m_scene_node()->get_skeleton_instance()->m_model_state().m_MeshGroupMask();

		for (int i = 0; i < p_model_handle->m_MeshGroups.m_size; i++)
		{
			bool is_active = (active_mesh_mask_group & (1ULL << i)) != 0;

			custom_model_groups.MeshNames.push_back(p_model_handle->m_MeshGroups.m_elements[i].m_pString);
			custom_model_groups.MeshBools[i] = is_active;
		}
	}

	saved_last_model = uPlayerModelHash;
	backup_engine_data = current;
	g_cfg->custom_model.m_should_update = false;
}

void c_agent_changer::agent_model(c_cs_player_pawn* observer_target, engine_data current)
{
	if (!g_cfg->agent_changer.m_enabled)
		return;

	auto* identity = observer_target->m_entity();
	if (!identity || !identity->is_valid())
		return;

	if (!identity->is_safe_to_modify())
		return;

	if (g_cfg->agent_changer.m_agent >= (int)g_item_schema->agents.size())
		return;

	const char* agent_model_path = g_item_schema->agents[g_cfg->agent_changer.m_agent].model_path;
	if (agent_model_path[0] == '\0')
		return;

	bool should_update_model = strcmp(observer_target->m_scene_node()->get_skeleton_instance()->m_model_state().m_ModelName().m_pString, agent_model_path) != 0;

	const bool config_changed = memcmp(&agent_backup_cfg, &g_cfg->agent_changer, sizeof(agent_backup_cfg)) != 0;

	if (current.state_changed(backup_engine_data) || config_changed || should_update_model /*current.m_demo_tick < general_engine_backup.m_demo_tick simple workaraound, idk how to actually fix it rn. prob its possible to check the model name and compare it -> https://www.unknowncheats.me/forum/counter-strike-2-a/606375-reading-player-model-name.html*/)
		should_update = true;

	if (!should_update)
		return;

	observer_target->set_model(agent_model_path);

	agent_backup_cfg = g_cfg->agent_changer;
	backup_engine_data = current;

	should_update = false;
}

void c_agent_changer::run(c_cs_player_pawn* observer_target, engine_data current) {
	this->custom_model(observer_target, current);
	this->agent_model(observer_target, current);
}
