#pragma once
#include "Platform.h"
#include "ShaderSystem.h"

enum MemoryPoolTypes
{
	kMeshBuffer,
	kMaterialBuffer,
	kDirectionalLightBuffer,
	kPointLightBuffer,
	kEndofPool
};

struct MemoryPoolSubBufferHeader
{
	uint32					ActiveCount = UINT32_MAX;
	size_t					Offset = UINT32_MAX;
	uint32					Count = UINT32_MAX;
	uint32					Size = UINT32_MAX;
	Vector<byte>			IsActive;         // 0 = inactive, 1 = active
	Vector<uint32>			FreeIndices;
	bool					IsDirty = true;
};

struct MemoryPoolBufferHeader
{
	//CPU side: in bytes
	//GPU side: by index (bytes/4)
	//GPU data array: by index (bytes/4)
	uint MeshOffset = UINT32_MAX;
	uint MeshCount = UINT32_MAX;
	uint MeshSize = UINT32_MAX;
	uint MaterialOffset = UINT32_MAX;
	uint MaterialCount = UINT32_MAX;
	uint MaterialSize = UINT32_MAX;
	uint DirectionalLightOffset = UINT32_MAX;
	uint DirectionalLightCount = UINT32_MAX;
	uint DirectionalLightSize = UINT32_MAX;
	uint PointLightOffset = UINT32_MAX;
	uint PointLightCount = UINT32_MAX;
	uint PointLightSize = UINT32_MAX;
};

struct MeshPropertiesStruct;
struct GPUMaterial;
struct DirectionalLight;
struct PointLight;
class MemoryPoolSystem
{
public:
	static MemoryPoolSystem& Get();

private:
	MemoryPoolSystem() = default;
	~MemoryPoolSystem() = default;
	MemoryPoolSystem(const MemoryPoolSystem&) = delete;
	MemoryPoolSystem& operator=(const MemoryPoolSystem&) = delete;
	MemoryPoolSystem(MemoryPoolSystem&&) = delete;
	MemoryPoolSystem& operator=(MemoryPoolSystem&&) = delete;

	static constexpr size_t									 MeshInitialCapacity = 4096;
	static constexpr size_t									 MaterialInitialCapacity = 65536;
	static constexpr size_t									 DirectionalLightInitialCapacity = 4;
	static constexpr size_t									 PointLightInitialCapacity = 4096;

	UnorderedMap<MemoryPoolTypes, MemoryPoolSubBufferHeader> MemorySubPoolHeader;

	void													 ResizeMemoryPool(MemoryPoolTypes memoryPoolToUpdate, uint32 resizeCount);
	void													 UpdateMemoryPoolHeader(MemoryPoolTypes memoryPoolType, uint32 newPoolSize);

public:
	uint32													 GPUBufferIndex = UINT32_MAX;
	size_t													 GPUBufferMemoryPoolSize = UINT32_MAX;
	MemoryPoolBufferHeader									 GPUMemoryPoolHeader;
	Vector<byte>											 GPUBufferMemoryPool;
	bool													 IsHeaderDirty = true;
	bool													 IsBufferDirty = true;

	DLL_EXPORT void											 StartUp();
	DLL_EXPORT uint32										 AllocateObject(MemoryPoolTypes memoryPoolToUpdate);
	DLL_EXPORT void											 UpdateMemoryPool(uint32 descriptorBindingIndex, Vector<VulkanPipeline>& pipelineList);
	DLL_EXPORT const Vector<VkDescriptorBufferInfo>			 GetMemoryPoolBufferInfo() const;

	DLL_EXPORT MeshPropertiesStruct&						 UpdateMesh(uint32 index);
	DLL_EXPORT GPUMaterial&									 UpdateMaterial(uint32 index);
	DLL_EXPORT DirectionalLight&							 UpdateDirectionalLight(uint32 index);
	DLL_EXPORT PointLight&									 UpdatePointLight(uint32 index);

	DLL_EXPORT void											 MarkMeshBufferDirty();
	DLL_EXPORT void											 MarkMaterialBufferDirty();
	DLL_EXPORT void											 MarkDirectionalLightBufferDirty();
	DLL_EXPORT void											 MarkPointLightBufferDirty();

	DLL_EXPORT Vector<MeshPropertiesStruct>					 MeshBufferList();
	DLL_EXPORT Vector<GPUMaterial>							 MaterialBufferList();
	DLL_EXPORT Vector<DirectionalLight>						 DirectionalLightBufferList();
	DLL_EXPORT Vector<PointLight>							 PointLightBufferList();

	DLL_EXPORT const MemoryPoolSubBufferHeader				 MemoryPoolSubBufferInfo(MemoryPoolTypes memoryPoolType);
};
extern DLL_EXPORT MemoryPoolSystem& memoryPoolSystem;
inline MemoryPoolSystem& MemoryPoolSystem::Get()
{
	static MemoryPoolSystem instance;
	return instance;
}
