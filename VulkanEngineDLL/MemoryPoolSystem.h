#pragma once

#include <Platform.h>
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
	uint  SpriteId;
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

struct TextureMetadataHeader
{
	uint32 Width = 0;
	uint32 Height = 0;
	uint32 Depth = 0;
	uint32 MipLevels = 1;
	uint32 LayerCount = 1; // 1 = 2D, 6 = Cubemap
	uint32 Format = 1;
	uint32 Type = 0; // 0 = 2D, 1 = Cube
	uint32 ArrayIndex = 0; // Index into the correct sampler array
};

struct SceneDataBuffer
{
	uint HDRMapInputIndex = UINT32_MAX;
	uint FrameBufferIndex = UINT32_MAX;
	uint32 BRDFMapId = UINT32_MAX;
	uint32 CubeMapId = UINT32_MAX;
	uint32 IrradianceMapId = UINT32_MAX;
	uint32 PrefilterMapId = UINT32_MAX;
	ivec2  _pad2;
	mat4   Projection;
	mat4   View;
	mat4   InverseProjection;
	mat4   InverseView;
	vec3   CameraPosition;
	float  _pad0;
	vec3   ViewDirection;
	float  _pad1;
	vec2   InvertResolution;
	float  Time;
	uint   FrameIndex;
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
struct DirectionalLightComponent;
struct PointLightComponent;
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

	static constexpr size_t									 MeshInitialCapacity = 4;
	static constexpr size_t									 MaterialInitialCapacity = 4;
	static constexpr size_t									 DirectionalLightInitialCapacity = 4;
	static constexpr size_t									 PointLightInitialCapacity = 4;
	static constexpr size_t									 SpriteInstanceInitialCapacity = 1024;

	UnorderedMap<MemoryPoolTypes, MemoryPoolSubBufferHeader> MemorySubPoolHeader;
	void													 UpdateMemoryPoolHeader(MemoryPoolTypes memoryPoolType, uint32 newPoolSize);
	void													 ResizeMemoryPool(MemoryPoolTypes memoryPoolToUpdate, uint32 resizeCount);
	void													 CreateGlobalBindlessDescriptorSet();

public:

	static constexpr uint									 SceneDataDescriptorBinding = 0;
	static constexpr uint									 BindlessDataDescriptorBinding = 1;
	static constexpr uint									 CubeMapDescriptorBinding = 2;
	static constexpr uint									 Texture2DBinding = 3;
	static constexpr uint									 Texture3DBinding = 4;

	static constexpr size_t									 Texture2DInitialCapacity = 4096;
	static constexpr uint32									 Texture3DInitialCapacity = 128; 
	static constexpr uint32									 TextureCubeMapInitialCapacity = 128;

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

	 void													 StartUp();
	 uint32													 AllocateObject(MemoryPoolTypes memoryPoolToUpdate);
	 void													 UpdateMemoryPool();
	 void													 UpdateTextureDescriptorSet(uint32 textureGpuBufferIndex, VulkanTexture& texture, uint binding);
	 void													 UpdateDataBufferDescriptorSet(uint32 vulkanGpuBufferIndex, uint binding);
	 MeshPropertiesStruct&									 UpdateMesh(uint32 index);
	 GPUMaterial&											 UpdateMaterial(uint32 index);
	 DirectionalLightComponent&								 UpdateDirectionalLight(uint32 index);
	 PointLightComponent&									 UpdatePointLight(uint32 index);
	 TextureMetadataHeader&									 UpdateTexture2DMetadataHeader(uint32 index);
	 TextureMetadataHeader&									 UpdateTexture3DMetadataHeader(uint32 index);
	 TextureMetadataHeader&									 UpdateTextureCubeMapMetadataHeader(uint32 index);
	 SpriteInstance&										 UpdateSpriteInstance(uint32 index);
	 SceneDataBuffer&										 UpdateSceneDataBuffer();
	 uint32 AddToMemoryPool(VulkanTexture& texture);
	 Vector<SpriteInstance*>								 GetActiveSpriteInstancePointers();

	 Vector<MeshPropertiesStruct>							 MeshBufferList();
	 Vector<GPUMaterial>									 MaterialBufferList();
	 Vector<DirectionalLightComponent>						 DirectionalLightBufferList();
	 Vector<PointLightComponent>							 PointLightBufferList();
	 Vector<SpriteInstance>									 SpriteInstanceBufferList();

	 void													 FreeObject(MemoryPoolTypes memoryPoolToUpdate, uint32 index);

	 const MemoryPoolSubBufferHeader						 MemoryPoolSubBufferInfo(MemoryPoolTypes memoryPoolType);
	 const Vector<VkDescriptorBufferInfo>					 GetSceneDataBufferDescriptor() const;
	 const Vector<VkDescriptorBufferInfo>					 GetBindlessDataBufferDescriptor() const;
	 const Vector<VkDescriptorImageInfo>					 GetSubPassInputTextureDescriptor(VkGuid& renderPassId) const;
};
extern  MemoryPoolSystem& memoryPoolSystem;
inline MemoryPoolSystem& MemoryPoolSystem::Get()
{
	static MemoryPoolSystem instance;
	return instance;
}
