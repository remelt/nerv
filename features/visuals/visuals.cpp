#include "visuals.h"
//TODO: find alternative way for should_update -> ??? 
void c_visuals::skybox_changer()
{
	if (!g_interfaces->m_engine2_to_client->IsInGame() || !g_interfaces->m_engine2_to_client->IsConnected() || !g_ctx->m_local_pawn || !g_ctx->m_skybox_object)
		return;

	static int saved_index = 0;

	if ((saved_index != g_cfg->visuals.m_skybox_selected || this->should_update) && g_cfg->visuals.m_skybox_selected > 0)
	{
		const char* skybox_path = this->SkyBoxesArray.SkyBoxPath[g_cfg->visuals.m_skybox_selected];

		g_interfaces->m_resource_system->BlockingLoadResourceByName(skybox_path, 0);
		c_material2** pSkyBoxMat = g_interfaces->m_material_system2->FindMaterial(skybox_path);

		if (pSkyBoxMat && g_ctx->m_skybox_object && g_ctx->m_skybox_object->pSkyBoxDataMaterial != (c_scene_skybox_data*)pSkyBoxMat)
			g_ctx->m_skybox_object->pSkyBoxDataMaterial = (c_scene_skybox_data*)pSkyBoxMat;

		should_update = false;
	}

	saved_index = g_cfg->visuals.m_skybox_selected;
}

void c_visuals::skybox_parser()
{
	for (int i = 0; i < 18; ++i)
	{
		this->SkyBoxesArray.emplace(SkyBoxNames[i], SkyBoxPaths[i]);
	}

	//TODO: parse custom skyboxes

}
