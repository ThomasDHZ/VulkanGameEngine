#include "MeshSystem.h"

MeshSystem meshSystem = MeshSystem();

MeshSystem::MeshSystem()
{

}

MeshSystem::~MeshSystem()
{

}

uint MeshSystem::CreateSpriteLayerMesh(MeshTypeEnum meshtype, Vector<Vertex2D>& vertexList, Vector<uint32>& indexList)
{
    return Mesh_CreateMesh(renderer, meshtype, vertexList, indexList);
}

void MeshSystem::Update(const float& deltaTime)
{
    Mesh_Update(renderer, deltaTime);
}

void MeshSystem::Destroy(uint meshId)
{
    Mesh_Destroy(renderer, meshId);
}

void MeshSystem::DestroyAllGameObjects()
{
    Mesh_DestroyAllGameObjects(renderer);
}

const Vector<Mesh> MeshSystem::FindMeshByMeshType(MeshTypeEnum meshType)
{
    return Mesh_FindMeshByMeshType(meshType);
}

const Vector<Mesh>& MeshSystem::FindMeshByVertexType(VertexTypeEnum vertexType)
{
    return Mesh_FindMeshByVertexType(vertexType);
}
