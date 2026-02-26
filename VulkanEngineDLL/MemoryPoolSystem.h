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
	kSpriteInstanceBuffer,
	kEndofPool
};

struct SpriteInstance
{
	vec2  SpritePosition;
	vec4  UVOffset;
	vec2  SpriteSize;
	ivec2 FlipSprite;
	vec4  Color;
	mat4  InstanceTransform;
	uint  MaterialId;
	uint  Padding;
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

struct SceneDataBuffer
{
	mat4  Projection;
	mat4  View;
	mat4  InverseProjection;
	mat4  InverseView;
	vec3  CameraPosition;
	float _pad0;
	vec3  ViewDirection;
	float _pad1;
	vec2  InvertResolution;
	float Time;
	uint  FrameIndex;
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
	uint64 SpriteInstanceOffset = UINT64_MAX;
	uint32 SpriteInstanceCount = UINT32_MAX;
	uint32 SpriteInstanceSize = UINT32_MAX;
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

	static constexpr uint									 SceneDataDescriptorBinding = 0;
	static constexpr uint									 BindlessDataDescriptorBinding = 1;

	static constexpr size_t									 MeshInitialCapacity = 4;
	static constexpr size_t									 MaterialInitialCapacity = 4;
	static constexpr size_t									 DirectionalLightInitialCapacity = 4;
	static constexpr size_t									 PointLightInitialCapacity = 4;
	static constexpr size_t									 Texture2DInitialCapacity = 32;
	static constexpr size_t									 Texture3DInitialCapacity = 4;
	static constexpr size_t									 TextureCubeMapInitialCapacity = 4;
	static constexpr size_t									 SpriteInstanceInitialCapacity = 16384;

	UnorderedMap<MemoryPoolTypes, MemoryPoolSubBufferHeader> MemorySubPoolHeader;
	void													 UpdateMemoryPoolHeader(MemoryPoolTypes memoryPoolType, uint32 newPoolSize);
	void													 ResizeMemoryPool(MemoryPoolTypes memoryPoolToUpdate, uint32 resizeCount);

public:
	VkDescriptorPool										 GlobalBindlessPool = VK_NULL_HANDLE;
	VkDescriptorSet											 GlobalBindlessDescriptorSet = VK_NULL_HANDLE;
	VkDescriptorSetLayout									 GlobalBindlessDescriptorSetLayout = VK_NULL_HANDLE;

	uint32													 SceneDataBufferIndex = UINT32_MAX;
	void*													 SceneDataPtr = nullptr;
	bool													 IsSceneBufferDirty = true;

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
	DLL_EXPORT SpriteInstance&								 UpdateSpriteInstance(uint32 index);
	DLL_EXPORT SceneDataBuffer&								 UpdateSceneDataBuffer();

	DLL_EXPORT Vector<SpriteInstance*>						 GetActiveSpriteInstancePointers();

	DLL_EXPORT Vector<MeshPropertiesStruct>					 MeshBufferList();
	DLL_EXPORT Vector<GPUMaterial>							 MaterialBufferList();
	DLL_EXPORT Vector<DirectionalLight>						 DirectionalLightBufferList();
	DLL_EXPORT Vector<PointLight>							 PointLightBufferList();
	DLL_EXPORT Vector<SpriteInstance>						 SpriteInstanceBufferList();

	DLL_EXPORT void											 FreeObject(MemoryPoolTypes memoryPoolToUpdate, uint32 index);

	DLL_EXPORT const MemoryPoolSubBufferHeader				 MemoryPoolSubBufferInfo(MemoryPoolTypes memoryPoolType);
	DLL_EXPORT const Vector<VkDescriptorBufferInfo>			 GetSceneDataBufferDescriptor() const;
	DLL_EXPORT const Vector<VkDescriptorBufferInfo>			 GetBindlessDataBufferDescriptor() const;
};
extern DLL_EXPORT MemoryPoolSystem& memoryPoolSystem;
inline MemoryPoolSystem& MemoryPoolSystem::Get()
{
	static MemoryPoolSystem instance;
	return instance;
}
