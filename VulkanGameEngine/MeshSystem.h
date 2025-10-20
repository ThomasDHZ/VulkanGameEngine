#pragma once

#include "Vertex.h"
#include <Mesh.h>
#include <BufferSystem.h>
#include "GameObjectSystem.h"
#include "MaterialSystem.h"
#include "LevelSystem.h"
#include "GameSystem.h"

class MeshSystem
{
private:
public:
    MeshSystem();
    ~MeshSystem();

    uint CreateSpriteLayerMesh(MeshTypeEnum meshtype, Vector<Vertex2D>& vertexList, Vector<uint32>& indexList);
    
    void Update(const float& deltaTime);

    void Destroy(uint meshId);
    void DestroyAllGameObjects();

    const Vector<uint>& FindIndexList(const uint& id);
    const Vector<Mesh> FindMeshByMeshType(MeshTypeEnum meshType);
    const Vector<Mesh>& FindMeshByVertexType(VertexTypeEnum vertexType);
};
extern MeshSystem meshSystem;