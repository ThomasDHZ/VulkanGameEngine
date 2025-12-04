#pragma once
#include <MeshSystem.h>

#ifdef __cplusplus
extern "C"
{
#endif
	DLL_EXPORT uint MeshSystem_CreateMesh(MeshTypeEnum meshType, Vertex2D* vertexListPtr, uint32* indexListPtr, size_t vertexListCount, size_t indexListCount);
	DLL_EXPORT void MeshSystem_Update(const float& deltaTime);
	DLL_EXPORT void MeshSystem_Destroy(uint meshId);
	DLL_EXPORT void MeshSystem_DestroyMesh(Mesh& mesh, VulkanBuffer& vertexBuffer, VulkanBuffer& indexBuffer, VulkanBuffer& transformBuffer, VulkanBuffer& propertiesBuffer);
	DLL_EXPORT void MeshSystem_DestroyAllGameObjects();
	DLL_EXPORT void* MeshSystem_FindMesh(const uint& meshId);
#ifdef __cplusplus
}
#endif
