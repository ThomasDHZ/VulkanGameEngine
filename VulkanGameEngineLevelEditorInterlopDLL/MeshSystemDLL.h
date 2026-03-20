#pragma once
#include <MeshSystem.h>

#ifdef __cplusplus
extern "C"
{
#endif
	DLL_EXPORT uint MeshSystem_CreateMesh(const char* key, MeshTypeEnum meshType, VertexLayout& vertexData, uint32* indexListPtr, size_t indexListCount, VkGuid materialId = VkGuid());
	DLL_EXPORT void MeshSystem_Update(const float& deltaTime);
	DLL_EXPORT void MeshSystem_Destroy(uint meshId);
	DLL_EXPORT void MeshSystem_DestroyAllGameObjects();
#ifdef __cplusplus
}
#endif
