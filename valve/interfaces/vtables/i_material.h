#pragma once
#include <sdk/vfunc/vfunc.hpp>
#include <cstdint>

class c_material2
{
public:
	virtual const char* GetName() = 0;
	virtual const char* GetShareName() = 0;
};

class i_material_system2
{
public:
	c_material2** FindOrCreateFromResource(c_material2** pOutMaterial, const char* szMaterialName)
	{
		return VFUNC(this, c_material2 **, 14, pOutMaterial, szMaterialName);
	}

	//c_material2** CreateMaterial(c_material2*** pOutMaterial, const char* szMaterialName, CMeshData* pData)
	//{
	//	return VFUNC(this, c_material2 **, 29, pOutMaterial, szMaterialName, pData);
	//}

	//void SetCreateDataByMaterial(const void* pData, c_material2*** const pInMaterial)
	//{
	//	return VFUNC(this, voids, 37, pData, pInMaterial);
	//}

	c_material2** FindMaterial(const char* szMaterialName) {
		c_material2* pTemporary = nullptr;
		c_material2** ppResult = this->FindOrCreateFromResource(&pTemporary, szMaterialName);
		/*if (ppResult != nullptr && *ppResult != nullptr && **ppResult != nullptr)
		{
			CMaterial2* pMaterial = **ppResult;
			const char* pMaterialName = pMaterial->GetName();

			if (stricmp(pMaterialName, "materials/error.vmat"))
				return pMaterial;
		}*/

		return *(c_material2***)ppResult;
	}

};

//class CDrawMesh
//{
//public:
//
//	char pad_0000[24]; //0x0000
//	void* m_pSceneAnimatableObject; //0x0018
//	c_material2* m_pMaterial;  //0x0020
//	c_material2* m_pMaterial2; //0x0028
//	char pad_0030[32]; //0x0030
//	Color_B m_color; //0x0050
//	char pad_0054[28]; //0x0054
//};

class c_aggregate_object_data
{
public:
	char pad_0000[4]; //0x0000
	int count; //0x0004
	char pad_0008[48]; //0x0008
	int index; //0x0044
}; //Size: 0x0034

class c_aggregate_object_array
{
public:
	void* object; //0x0000
	c_aggregate_object_data* data; //0x0008
};
class c_scene_skybox_data
{
public:
	c_material2* pSkyMaterial;
	void* pUnk;
};
class c_scene_skybox_object
{
public:
	char pad_0000[208]; //0x0000
	c_scene_skybox_data* pSkyBoxDataMaterial; //0x00D0
	char pad_00D8[16]; //0x00D8
	vec3_t SkyBoxColor;
};