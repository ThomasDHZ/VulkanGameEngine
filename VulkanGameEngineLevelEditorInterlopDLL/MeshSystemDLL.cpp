#include "pch.h"
#include "MeshSystemDLL.h"

uint MeshSystem_CreateMesh(MeshTypeEnum meshType, Vertex2D* vertexListPtr, uint32* indexListPtr, size_t vertexListCount, size_t indexListCount)
{
    Vector<Vertex2D> vertexList = Vector<Vertex2D>(vertexListPtr, vertexListPtr + vertexListCount);
    Vector<uint32> indexList = Vector<uint32>(indexListPtr, indexListPtr + indexListCount);
    return meshSystem.CreateMesh(meshType, vertexList, indexList);
}

void MeshSystem_Update(const float& deltaTime)
{
    meshSystem.Update(deltaTime);
}

void MeshSystem_Destroy(uint meshId)
{
    meshSystem.Destroy(meshId);
}

void MeshSystem_DestroyAllGameObjects()
{
    meshSystem.DestroyAllGameObjects();
}

void* MeshSystem_FindMesh(const uint& meshId)
{
    struct TempDLL
    {
        int Age = 34;
        int Score = 32;
    } temp;

    TempDLL* tempPtr = memorySystem.AddPtrBuffer<TempDLL>(temp, __FILE__, __LINE__, __func__);
    return static_cast<void*>(tempPtr);
}