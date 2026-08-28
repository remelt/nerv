#pragma once
#include <sdk/typedefs/vec_t.hpp>


class CAggregateSceneObject
{
public:
	unsigned char pad_0[0xE4]; // 0x0 - 0xE3
	vec3_t m_cLightColor;
};

class CSceneLightObject
{
public:
	char pad_0000[388]; //0x0000		
	vec3_t m_vDirectionNormalized; //0x0184
};

class CSceneData
{
public:
	char pad_0000[40]; //0x0000
	CSceneLightObject* m_pSceneLightObject; //0x0028
};

class CSceneObjectInfo
{
public:
	CSceneData* m_pScene;
};