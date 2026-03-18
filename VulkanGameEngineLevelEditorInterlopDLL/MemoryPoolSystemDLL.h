#pragma once
#include <MemoryPoolSystem.h>

#ifdef __cplusplus
extern "C" {
#endif
	DLL_EXPORT void					 MemoryPoolSystem_StartUp();
	DLL_EXPORT uint32			     MemoryPoolSystem_AllocateObject(MemoryPoolTypes memoryPoolToUpdate);
	DLL_EXPORT MeshPropertiesStruct* MemoryPoolSystem_UpdateMesh(uint32 index);
	DLL_EXPORT GPUMaterial*			 MemoryPoolSystem_UpdateMaterial(uint32 index);
	DLL_EXPORT DirectionalLight*	 MemoryPoolSystem_UpdateDirectionalLight(uint32 index);
	DLL_EXPORT PointLight*			 MemoryPoolSystem_UpdatePointLight(uint32 index);
	DLL_EXPORT SceneDataBuffer*		 MemoryPoolSystem_UpdateSceneDataBuffer();
	DLL_EXPORT void					 MemoryPoolSystem_FreeObject(MemoryPoolTypes memoryPoolToUpdate, uint32 index);
#ifdef __cplusplus
}
#endif



