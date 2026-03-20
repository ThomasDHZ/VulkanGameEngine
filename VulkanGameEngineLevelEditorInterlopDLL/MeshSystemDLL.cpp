#include "MeshSystemDLL.h"

 uint MeshSystem_CreateMesh(const char* key, MeshTypeEnum meshType, VertexLayout& vertexData, uint32* indexListPtr, size_t indexListCount, VkGuid materialId)
{
     Vector<uint32> indexList = Vector<uint32>(indexListPtr, indexListPtr + indexListCount);
    return meshSystem.CreateMesh(key, meshType, vertexData, indexList, materialId);
}

 void MeshSystem_Update(const float& deltaTime)
{
    return  void();
}

 void MeshSystem_Destroy(uint meshId)
{
    return  void();
}

 void MeshSystem_DestroyAllGameObjects()
{
    return  void();
}
