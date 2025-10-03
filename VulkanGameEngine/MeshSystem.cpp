#include "MeshSystem.h"

MeshSystem meshSystem = MeshSystem();

MeshSystem::MeshSystem()
{

}

MeshSystem::~MeshSystem()
{

}

uint MeshSystem::CreateSpriteLayerMesh(Vector<Vertex2D>& vertexList, Vector<uint32>& indexList)
{
    return Mesh_CreateSpriteLayerMesh(renderSystem.renderer, vertexList, indexList);
}

uint MeshSystem::CreateLevelLayerMesh(const VkGuid& levelId, Vector<Vertex2D>& vertexList, Vector<uint32>& indexList)
{
    return Mesh_CreateLevelLayerMesh(renderSystem.renderer, levelId, vertexList, indexList);
}

void MeshSystem::Update(const float& deltaTime)
{
    Mesh_Update(renderSystem.renderer, deltaTime);
}

void MeshSystem::Destroy(uint meshId)
{
    Mesh_Destroy(renderSystem.renderer, meshId);
}

void MeshSystem::DestroyAllGameObjects()
{
    Mesh_DestroyAllGameObjects(renderSystem.renderer);
}

const Mesh& MeshSystem::FindMesh(const uint& id)
{
    return Mesh_FindMesh(id);
}

const Mesh& MeshSystem::FindSpriteMesh(const uint& id)
{
    return Mesh_FindSpriteMesh(id);
}

const Vector<Mesh>& MeshSystem::FindLevelLayerMeshList(const LevelGuid& guid)
{
    return Mesh_FindLevelLayerMeshList(guid);
}

const Vector<Vertex2D>& MeshSystem::FindVertex2DList(const uint& id)
{
    return Mesh_FindVertex2DList(id);
}

const Vector<uint>& MeshSystem::FindIndexList(const uint& id)
{
    return Mesh_FindIndexList(id);
}

const Vector<Mesh> MeshSystem::MeshList()
{
    return Mesh_MeshList();
}

const Vector<Mesh> MeshSystem::SpriteMeshList()
{
    return Mesh_SpriteMeshList();
}