#include "MaterialSystemDLL.h"

VkGuid MaterialSystem_LoadMaterial(const char* materialPath)
{
	return materialSystem.LoadMaterial(materialPath);
}

bool MaterialSystem_MaterialExists(VkGuid materialGuid)
{
	return materialSystem.MaterialExists(materialGuid);
}

Material* MaterialSystem_FindMaterial(VkGuid materialGuid)
{
	return &materialSystem.FindMaterial(materialGuid);
}

uint MaterialSystem_FindMaterialPoolIndex(VkGuid materialGuid)
{
	return materialSystem.FindMaterialPoolIndex(materialGuid);
}

void MaterialSystem_Destroy(VkGuid materialGuid)
{
	return materialSystem.Destroy(materialGuid);
}

void MaterialSystem_DestroyAllMaterials()
{
	return materialSystem.DestroyAllMaterials();
}
