#pragma once
#include <MaterialSystem.h>

#ifdef __cplusplus
extern "C" {
#endif
    DLL_EXPORT VkGuid MaterialSystem_LoadMaterial(const char* materialPath);
    DLL_EXPORT bool MaterialSystem_MaterialExists(VkGuid materialGuid);
    DLL_EXPORT Material* MaterialSystem_FindMaterial(VkGuid materialGuid);
    DLL_EXPORT uint MaterialSystem_FindMaterialPoolIndex(VkGuid materialGuid);
    DLL_EXPORT void MaterialSystem_Destroy(VkGuid materialGuid);
    DLL_EXPORT void MaterialSystem_DestroyAllMaterials();
#ifdef __cplusplus
}
#endif