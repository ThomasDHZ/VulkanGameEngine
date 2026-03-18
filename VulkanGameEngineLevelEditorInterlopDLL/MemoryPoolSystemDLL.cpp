#include "MemoryPoolSystemDLL.h"

void MemoryPoolSystem_StartUp()
{
    memoryPoolSystem.StartUp();
}

uint32 MemoryPoolSystem_AllocateObject(MemoryPoolTypes memoryPoolToUpdate)
{
    return memoryPoolSystem.AllocateObject(memoryPoolToUpdate);
}

 MeshPropertiesStruct* MemoryPoolSystem_UpdateMesh(uint32 index)
 {
     return  &memoryPoolSystem.UpdateMesh(index);
 }

 GPUMaterial* MemoryPoolSystem_UpdateMaterial(uint32 index)
 {
     return  &memoryPoolSystem.UpdateMaterial(index);
 }

 DirectionalLight* MemoryPoolSystem_UpdateDirectionalLight(uint32 index)
 {
     return  &memoryPoolSystem.UpdateDirectionalLight(index);
 }

 PointLight* MemoryPoolSystem_UpdatePointLight(uint32 index)
 {
     return  &memoryPoolSystem.UpdatePointLight(index);
 }

 SceneDataBuffer* MemoryPoolSystem_UpdateSceneDataBuffer()
 {
     return  &memoryPoolSystem.UpdateSceneDataBuffer();
 }

 void MemoryPoolSystem_FreeObject(MemoryPoolTypes memoryPoolToUpdate, uint32 index)
 {
     memoryPoolSystem.FreeObject(memoryPoolToUpdate, index);
 }
