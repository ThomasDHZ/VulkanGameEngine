#pragma once
#include "Platform.h"
#include "ShaderSystem.h"

enum MemoryPoolTypes
{
	kMeshBuffer,
	kMaterialBuffer,
	kDirectionalLightBuffer,
	kPointLightBuffer,
	kTexture2DMetadataBuffer,
	kTexture3DMetadataBuffer,
	kTextureCubeMapMetadataBuffer,
	kEndofPool
};

struct MemoryPoolSubBufferHeader
{
	uint32					ActiveCount = UINT32_MAX;
	size_t					Offset		= UINT32_MAX;
	uint32					Count		= UINT32_MAX;
	uint32					Size		= UINT32_MAX;
	Vector<byte>			IsActive;         // 0 = inactive, 1 = active
	Vector<uint32>			FreeIndices;
	bool					IsDirty = true;
};

struct TextureMetadataHeader
{
	uint32 Width	  = 0;
	uint32 Height	  = 0;
	uint32 Depth	  = 0;
	uint32 MipLevels  = 1;
	uint32 LayerCount = 1; // 1 = 2D, 6 = Cubemap
	uint32 Format	  = 1;
	uint32 Type		  = 0; // 0 = 2D, 1 = Cube
	uint32 ArrayIndex = 0; // Index into the correct sampler array
};

struct MemoryPoolBufferHeader
{
	//CPU side: in bytes
	//GPU side: by index (bytes/4)
	//GPU data array: by index (bytes/4)
	uint64 MeshOffset = UINT64_MAX;
	uint32 MeshCount = UINT32_MAX;
	uint32 MeshSize = UINT32_MAX;
	uint64 MaterialOffset = UINT64_MAX;
	uint32 MaterialCount = UINT32_MAX;
	uint32 MaterialSize = UINT32_MAX;
	uint64 DirectionalLightOffset = UINT64_MAX;
	uint32 DirectionalLightCount = UINT32_MAX;
	uint32 DirectionalLightSize = UINT32_MAX;
	uint64 PointLightOffset = UINT64_MAX;
	uint32 PointLightCount = UINT32_MAX;
	uint32 PointLightSize = UINT32_MAX;
	uint64 Texture2DOffset = UINT64_MAX;
	uint32 Texture2DCount = UINT32_MAX;
	uint32 Texture2DSize = UINT32_MAX;
	uint64 Texture3DOffset = UINT64_MAX;
	uint32 Texture3DCount = UINT32_MAX;
	uint32 Texture3DSize = UINT32_MAX;
	uint64 TextureCubeMapOffset = UINT64_MAX;
	uint32 TextureCubeMapCount = UINT32_MAX;
	uint32 TextureCubeMapSize = UINT32_MAX;
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

	static constexpr uint									 BindlessDataDescriptorBinding = 9;

	static constexpr size_t									 MeshInitialCapacity = 4096;
	static constexpr size_t									 MaterialInitialCapacity = 65536;
	static constexpr size_t									 DirectionalLightInitialCapacity = 4;
	static constexpr size_t									 PointLightInitialCapacity = 4096;
	static constexpr size_t									 Texture2DInitialCapacity = 128;
	static constexpr size_t									 Texture3DInitialCapacity = 128;
	static constexpr size_t									 TextureCubeMapInitialCapacity = 128;

	UnorderedMap<MemoryPoolTypes, MemoryPoolSubBufferHeader> MemorySubPoolHeader;
	void													 ResizeMemoryPool(MemoryPoolTypes memoryPoolToUpdate, uint32 resizeCount);
	void													 UpdateMemoryPoolHeader(MemoryPoolTypes memoryPoolType, uint32 newPoolSize);

public:
	uint32													 GpuDataBufferIndex = UINT32_MAX;
	size_t													 GpuDataBufferMemoryPoolSize = UINT32_MAX;
	MemoryPoolBufferHeader									 GpuDataMemoryPoolHeader;
	Vector<byte>											 GpuDataBufferMemoryPool;
	void*													 MappedBufferPtr = nullptr;
	bool													 IsHeaderDirty = true;
	bool													 IsDescriptorSetDirty = true;

	DLL_EXPORT void											 StartUp();
	DLL_EXPORT uint32										 AllocateObject(MemoryPoolTypes memoryPoolToUpdate);
	DLL_EXPORT void											 UpdateMemoryPool(Vector<VulkanPipeline>& pipelineList);

	DLL_EXPORT MeshPropertiesStruct&						 UpdateMesh(uint32 index);
	DLL_EXPORT GPUMaterial&									 UpdateMaterial(uint32 index);
	DLL_EXPORT DirectionalLight&							 UpdateDirectionalLight(uint32 index);
	DLL_EXPORT PointLight&									 UpdatePointLight(uint32 index);
	DLL_EXPORT TextureMetadataHeader&						 UpdateTexture2DMetadataHeader(uint32 index);
	DLL_EXPORT TextureMetadataHeader&						 UpdateTexture3DMetadataHeader(uint32 index);
	DLL_EXPORT TextureMetadataHeader&						 UpdateTextureCubeMapMetadataHeader(uint32 index);

	DLL_EXPORT Vector<MeshPropertiesStruct>					 MeshBufferList();
	DLL_EXPORT Vector<GPUMaterial>							 MaterialBufferList();
	DLL_EXPORT Vector<DirectionalLight>						 DirectionalLightBufferList();
	DLL_EXPORT Vector<PointLight>							 PointLightBufferList();

	DLL_EXPORT const MemoryPoolSubBufferHeader				 MemoryPoolSubBufferInfo(MemoryPoolTypes memoryPoolType);
	DLL_EXPORT const Vector<VkDescriptorBufferInfo>			 GetBindlessDataBufferDescriptor() const;
};
extern DLL_EXPORT MemoryPoolSystem& memoryPoolSystem;
inline MemoryPoolSystem& MemoryPoolSystem::Get()
{
	static MemoryPoolSystem instance;
	return instance;
}
