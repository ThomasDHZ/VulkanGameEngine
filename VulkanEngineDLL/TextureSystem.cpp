#include "TextureSystem.h"
#include "RenderSystem.h"
#include "FileSystem.h"
#include "BufferSystem.h"
#include "JsonStruct.h"
#include "from_json.h"
#include <algorithm>
#include <cmath>
#include <stb/stb_image.h> 
#include <stb/stb_image_write.h>
#include "JsonStruct.h"
#include <imgui/backends/imgui_impl_vulkan.h>
#include "MemoryPoolSystem.h"
#include "MeshSystem.h"
#include <lodepng.h>
#include "RenderSystem.h"

TextureSystem& textureSystem = TextureSystem::Get();
Texture TextureSystem::LoadTexture(const String& texturePath)
{
	return LoadTexture(fileSystem.LoadJsonFile(texturePath.c_str()).get<TextureLoader>());
}

Texture TextureSystem::LoadTexture(const TextureLoader& textureLoader)
{
	if (TextureExists(textureLoader.TextureId)) return FindTexture(textureLoader.TextureId);

	String path = textureLoader.TextureFilePath.front();
	String ext = fileSystem.GetFileExtention(path.c_str());

	TextureReturnFileData texData;
	if (ext == "ktx" || ext == "ktx2") texData = LoadKtxTexture(textureLoader);
	else if (ext == "png") texData = LoadPngTexture(textureLoader);
	else texData = LoadGeneralTexture(textureLoader);
	if (texData.TextureData.empty())
	{
		std::cerr << "[TextureSystem] Failed to load: " << path << std::endl;
		return {};
	}

	VulkanTextureLoader vulkanLoader
	{
		.TextureData = texData.TextureData,
		.TextureDimensions = texData.TextureDimensions,
		.SamplerCreateInfo = textureLoader.SamplerCreateInfo,
		.MipMapCount = texData.MipMapCount,
		.ColorChannels = ColorChannelEnum::ChannelRGBA,
		.TextureImageLayout = texData.TextureImageLayout,
		.SampleCount = VK_SAMPLE_COUNT_1_BIT,
		.TextureByteFormat = texData.TextureByteFormat,
		.TextureType = textureLoader.TextureType,
		.IsRenderPassAttachment = false,
		.IsCubeMap = texData.IsCubeMap
	};

	Texture texture
	{
		.textureGuid = textureLoader.TextureId,
		.texture = VulkanTexture(vulkanLoader),
		.textureType = textureLoader.TextureType,
		.textureUsageType = textureLoader.TextureUsageType,
		.imGuiDescriptorSet = nullptr
	};

	SceneDataBuffer& sceneData = memoryPoolSystem.UpdateSceneDataBuffer();
	switch (textureLoader.TextureUsageType)
	{
	case kUsageType_CubeMap:            sceneData.CubeMapId = texture.textureId.id; break;
	case kUsageType_IrradianceTexture:  sceneData.IrradianceMapId = texture.textureId.id; break;
	case kUsageType_PrefilterTexture:   sceneData.PrefilterMapId = texture.textureId.id; break;
	case kUsageType_BRDFTexture:        sceneData.BRDFMapId = texture.textureId.id; break;
	default: break;
	}

	AddToMemoryPool(texture);
	return texture;
}

Texture TextureSystem::CreateRenderPassTexture(VulkanRenderPass& vulkanRenderPass, RenderPassAttachmentLoader& attachment)
{
	VkImageLayout textureImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	switch (attachment.TextureUsageType)
	{
		case kUsageType_DepthBufferTexture:     textureImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;  break;
		case kUsageType_GBufferTexture:         textureImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;         break;
		case kUsageType_IrradianceTexture:      textureImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;         break;
		case kUsageType_PrefilterTexture:       textureImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;         break;
		case kUsageType_OffscreenColorTexture:  textureImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;         break;
		case kUsageType_SwapChainTexture:       textureImageLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;                  break;
		case kUsageType_CubeMap:				textureImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;         break;
		case kUsageType_BRDFTexture:			textureImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;         break;
	}

	VulkanTextureLoader vulkanTextureLoader =
	{
		.TextureData = Vector<byte>(),
		.TextureDimensions = ivec3(vulkanRenderPass.RenderPassResolution().x, vulkanRenderPass.RenderPassResolution().y, 1),
		.SamplerCreateInfo = attachment.SamplerCreateInfo,
		.MipMapCount = attachment.MipMapCount,
		.ColorChannels = ColorChannelEnum::ChannelRGBA,
		.TextureImageLayout = textureImageLayout,
		.SampleCount = vulkanRenderPass.SampleCount(),
		.TextureByteFormat = attachment.TextureByteFormat,
		.TextureType = attachment.TextureType,
		.IsRenderPassAttachment = true,
	};

	Texture texture = Texture
	{
		.textureGuid = attachment.RenderedTextureId,
		.texture = VulkanTexture(vulkanTextureLoader),
		.textureType = attachment.TextureType,
		.textureUsageType = attachment.TextureUsageType,
		.imGuiDescriptorSet = nullptr
	};

	TextureReturnFileData textureReturnFileData = TextureReturnFileData
	{
		.TextureByteFormat = attachment.TextureByteFormat,
		.IsCubeMap = false
	};
	AddToMemoryPool(texture);
	return texture;
}

TextureReturnFileData TextureSystem::LoadKtxTexture(const TextureLoader& textureLoader)
{
	ktxTexture* ktex = nullptr;
	const String& path = textureLoader.TextureFilePath.front();
	
	KTX_error_code result = ktxTexture_CreateFromNamedFile(path.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktex);
	if (result != KTX_SUCCESS || !ktex)
	{
		std::cerr << "Failed to load KTX: " << path << " - " << ktxErrorString(result) << std::endl;
		return TextureReturnFileData();
	}

	ktxTexture2* ktex2 = reinterpret_cast<ktxTexture2*>(ktex);
	if (ktxTexture2_NeedsTranscoding(ktex2))
	{
		struct Candidate { VkFormat vkFmt; ktx_transcode_fmt_e ktxFmt; };
		Vector<Candidate> candidates;
		if (textureLoader.UsingSRGBFormat)
		{
			candidates.push_back({ VK_FORMAT_BC7_SRGB_BLOCK,    KTX_TTF_BC7_RGBA });
			candidates.push_back({ VK_FORMAT_ASTC_4x4_SRGB_BLOCK, KTX_TTF_ASTC_4x4_RGBA });
			candidates.push_back({ VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK, KTX_TTF_ETC2_RGBA });
		}
		else
		{
			candidates.push_back({ VK_FORMAT_BC7_UNORM_BLOCK,    KTX_TTF_BC7_RGBA });
			candidates.push_back({ VK_FORMAT_ASTC_4x4_UNORM_BLOCK, KTX_TTF_ASTC_4x4_RGBA });
			candidates.push_back({ VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK, KTX_TTF_ETC2_RGBA });
		}

		ktx_transcode_fmt_e targetFmt = KTX_TTF_RGBA32;
		for (const auto& candidate : candidates)
		{
			VkFormatProperties props{};
			vkGetPhysicalDeviceFormatProperties(vulkan.PhysicalDevice(), candidate.vkFmt, &props);
			if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)
			{
				targetFmt = candidate.ktxFmt;
				break;
			}
		}

		result = ktxTexture2_TranscodeBasis(ktex2, targetFmt, 0);
		if (result != KTX_SUCCESS)
		{
			std::cerr << "Transcode failed: " << ktxErrorString(result) << std::endl;
			ktxTexture_Destroy(ktex);
			return TextureReturnFileData();
		}
	}

	VkFormat textureByteFormat = ktxTexture2_GetVkFormat(ktex2);
	bool isDepthFormat = (textureByteFormat >= VK_FORMAT_D16_UNORM && textureByteFormat <= VK_FORMAT_D32_SFLOAT_S8_UINT) || (textureByteFormat == VK_FORMAT_X8_D24_UNORM_PACK32);
	bool hasStencil = (textureByteFormat == VK_FORMAT_D32_SFLOAT_S8_UINT || textureByteFormat == VK_FORMAT_D24_UNORM_S8_UINT);
	VkImageAspectFlags	  aspectMask = isDepthFormat ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
	if (hasStencil)		  aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	
	size_t dataSize = ktxTexture_GetDataSize(ktex);
	if (dataSize == 0)
	{
		std::cerr << "KTX data size is zero!" << std::endl;
		ktxTexture_Destroy(ktex);
		return TextureReturnFileData();
	}

	Vector<byte> ownedData(dataSize);
	memcpy(ownedData.data(), ktxTexture_GetData(ktex), dataSize);
	
	uint32 numComponents = 0;
	uint32 componentByteLength = 0;
	ktxTexture2_GetComponentInfo(ktex2, &numComponents, &componentByteLength);

	TextureReturnFileData out
	{
		.TextureData = std::move(ownedData),
		.MipMapCount = ktex2->numLevels,
		.ArrayLayers = ktex2->numLayers,
		.BytesPerChannel = componentByteLength,
		.TextureDimensions = {ktex2->baseWidth, ktex2->baseHeight, ktex2->baseDepth},
		.TextureByteFormat = ktxTexture2_GetVkFormat(ktex2),
		.TextureAspectFlags = aspectMask,
		.TextureImageLayout = isDepthFormat ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		.IsCubeMap = ktex2->isCubemap && ktex2->numLayers == 6,
		.IsDepthFormat = isDepthFormat,
		.IsStencil = hasStencil
	};

	ktxTexture_Destroy(ktex);
	return out;
}

TextureReturnFileData TextureSystem::LoadGeneralTexture(const TextureLoader& textureLoader)
{
	int width = 0;
	int height = 0;
	int channels = 0;
	uint forceChannel = 4;
	Vector<byte> textureData;
	for (auto& textureLayerPath : textureLoader.TextureFilePath)
	{
		byte* data = nullptr;
#ifdef PLATFORM_ANDROID
		AAsset* asset = AAssetManager_open(g_AssetManager, filePath.c_str(), AASSET_MODE_BUFFER);
		if (!asset) {
			return {};
		}

		size_t size = AAsset_getLength(asset);
		const void* buffer = AAsset_getBuffer(asset);
		data = stbi_load_from_memory((const stbi_uc*)buffer, (int)size, &w, &h, &comp, forceChannel);
		AAsset_close(asset);

		/* if (!data) {
			 __android_log_print(ANDROID_LOG_ERROR, "FileSystem", "STB failed: %s", stbi_failure_reason());
			 return {};
		 }*/
#else
		data = stbi_load(textureLayerPath.c_str(), &width, &height, &channels, forceChannel);
#endif
		Vector<byte> layerData(data, data + (width * height * forceChannel));
		stbi_image_free(data);

		textureData.insert(textureData.end(), layerData.begin(), layerData.end());
	}

	bool isDepthFormat = (textureLoader.TextureByteFormat >= VK_FORMAT_D16_UNORM && textureLoader.TextureByteFormat <= VK_FORMAT_D32_SFLOAT_S8_UINT) || (textureLoader.TextureByteFormat == VK_FORMAT_X8_D24_UNORM_PACK32);
	bool hasStencil = (textureLoader.TextureByteFormat == VK_FORMAT_D32_SFLOAT_S8_UINT || textureLoader.TextureByteFormat == VK_FORMAT_D24_UNORM_S8_UINT);
	VkImageAspectFlags	  aspectMask = isDepthFormat ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
	if (hasStencil)		  aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

	return TextureReturnFileData
	{
		.TextureData = textureData,
		.MipMapCount = textureLoader.MipMapCount,
		.ArrayLayers = static_cast<uint32>(textureLoader.TextureFilePath.size()),
		.BytesPerChannel = 1,
		.TextureDimensions = ivec3(width, height, 0),
		.TextureByteFormat = textureLoader.TextureByteFormat,
		.TextureAspectFlags = aspectMask,
		.TextureImageLayout = isDepthFormat ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		.IsCubeMap = static_cast<uint32>(textureLoader.TextureFilePath.size()) == 6,
		.IsDepthFormat = isDepthFormat,
		.IsStencil = hasStencil
	};
}

TextureReturnFileData TextureSystem::LoadPngTexture(const TextureLoader& textureLoader)
{
	String path = textureLoader.TextureFilePath.front();
	std::vector<unsigned char> pngData;
	unsigned error = lodepng::load_file(pngData, path.c_str());

	if (error)
	{
		std::cerr << "LodePNG load failed: " << lodepng_error_text(error) << std::endl;
		return {};
	}

	LodePNGState state;
	lodepng_state_init(&state);

	unsigned width = 0, height = 0;
	error = lodepng_inspect(&width, &height, &state, pngData.data(), pngData.size());
	if (error)
	{
		std::cerr << "LodePNG inspect failed: " << lodepng_error_text(error) << std::endl;
		lodepng_state_cleanup(&state);
		return {};
	}

	state.info_raw.colortype = LCT_RGBA;
	state.info_raw.bitdepth = 8;

	unsigned char* rawImage = nullptr;
	error = lodepng_decode(&rawImage, &width, &height, &state, pngData.data(), pngData.size());
	lodepng_state_cleanup(&state);

	if (error)
	{
		std::cerr << "LodePNG decode failed: " << lodepng_error_text(error) << std::endl;
		if (rawImage) free(rawImage);
		return {};
	}

	uint32 forceChannel = 4;
	size_t totalBytes = static_cast<size_t>(width) * height * forceChannel;
	Vector<byte> textureData(rawImage, rawImage + totalBytes);
	free(rawImage);

	return TextureReturnFileData
	{
		.TextureData = textureData,
		.MipMapCount = textureLoader.MipMapCount,
		.ArrayLayers = static_cast<uint32>(textureLoader.TextureFilePath.size()),
		.BytesPerChannel = 1,
		.TextureDimensions = ivec3(width, height, 1),
		.TextureByteFormat = textureLoader.TextureByteFormat,
		.TextureAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT,
		.TextureImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		.IsCubeMap = false
	};
}

void TextureSystem::AddToMemoryPool(Texture& texture)
{
	if (texture.texture.IsCubeMap())
	{
		texture.textureId.id = memoryPoolSystem.AllocateObject(kTextureCubeMapMetadataBuffer);
		TextureMetadataHeader& textureMetaDataHeader = memoryPoolSystem.UpdateTexture2DMetadataHeader(texture.textureId.id);
		textureMetaDataHeader.Width = texture.texture.TextureSize().x;
		textureMetaDataHeader.Height = texture.texture.TextureSize().y;
		textureMetaDataHeader.Depth = texture.texture.TextureSize().z;
		textureMetaDataHeader.MipLevels = texture.texture.MipMapLevels();
		textureMetaDataHeader.LayerCount = texture.texture.TextureArrayLayers();
		textureMetaDataHeader.Format = (uint32)texture.texture.TextureImageLayout();
		textureMetaDataHeader.Type = 1;

		CubeMapTextureList.emplace_back(texture);
		memoryPoolSystem.UpdateTextureDescriptorSet(texture, memoryPoolSystem.CubeMapDescriptorBinding);
	}
	else
	{
		texture.textureId.id = memoryPoolSystem.AllocateObject(kTexture2DMetadataBuffer);
		SceneDataBuffer& sceneDataBuffer = memoryPoolSystem.UpdateSceneDataBuffer();
		TextureMetadataHeader& textureMetaDataHeader = memoryPoolSystem.UpdateTexture2DMetadataHeader(texture.textureId.id);
		textureMetaDataHeader.Width = texture.texture.TextureSize().x;
		textureMetaDataHeader.Height = texture.texture.TextureSize().y;
		textureMetaDataHeader.Depth = texture.texture.TextureSize().z;
		textureMetaDataHeader.MipLevels = texture.texture.MipMapLevels();
		textureMetaDataHeader.LayerCount = texture.texture.TextureArrayLayers();
		textureMetaDataHeader.Format = (uint32)texture.texture.TextureImageLayout();
		textureMetaDataHeader.Type = 0;
		TextureList.emplace_back(texture);
		memoryPoolSystem.UpdateTextureDescriptorSet(texture, memoryPoolSystem.Texture2DBinding);
	}
}

Texture TextureSystem::FindTexture(const VkGuid& textureId)
{
	for (auto& pair : RenderedTextureListMap)
	{
		for (auto& texture : pair.second)
		{
			if (texture.textureGuid == textureId)
			{
				return texture;
			}
		}
	}

	for (auto& pair : DepthTextureMap)
	{
		if (pair.second.textureGuid == textureId)
		{
			return pair.second;
		}
	}

	for (auto& texture : TextureList)
	{
		if (texture.textureGuid == textureId)
		{
			return texture;
		}
	}
	throw std::out_of_range("Texture not found: TextureId: " + textureId.ToString());
}

void TextureSystem::AddRenderedTexture(RenderPassGuid renderPassGuid, Vector<Texture>& renderedTextureList)
{
	RenderedTextureListMap[renderPassGuid] = renderedTextureList;
}

void TextureSystem::AddDepthTexture(RenderPassGuid renderPassGuid, Texture& depthTexture)
{
	DepthTextureMap[renderPassGuid] = depthTexture;
}

Texture& TextureSystem::FindDepthTexture(const RenderPassGuid& renderPassGuid)
{
	return DepthTextureMap.at(renderPassGuid);
}

Texture& TextureSystem::FindRenderedTexture(const TextureGuid& textureGuid)
{
	for (auto& pair : RenderedTextureListMap)
	{
		auto& textureList = pair.second;
		auto it = std::find_if(textureList.begin(), textureList.end(),
			[&textureGuid](const Texture& texture)
			{
				return texture.textureGuid == textureGuid;
			});
		if (it != textureList.end())
			return *it;
	}
	throw std::out_of_range("Texture with Id: " + textureGuid.ToString() + " not found");
}

Vector<Texture>& TextureSystem::FindRenderedTextureList(const RenderPassGuid& renderPassGuid)
{
	return RenderedTextureListMap.at(renderPassGuid);
}

const bool TextureSystem::DepthTextureExists(const RenderPassGuid& renderPassGuid) const
{
	return DepthTextureMap.contains(renderPassGuid);
}

const bool TextureSystem::TextureExists(const TextureGuid& textureGuid) const
{
	auto it = std::find_if(TextureList.begin(), TextureList.end(),
		[&textureGuid](const Texture& texture)
		{
			return texture.textureGuid == textureGuid;
		});
	return it != TextureList.end();
}

const bool TextureSystem::RenderedTextureExists(const RenderPassGuid& renderPassGuid, const TextureGuid& textureGuid) const
{
	auto it = RenderedTextureListMap.find(renderPassGuid);
	if (it != RenderedTextureListMap.end())
	{
		return std::any_of(it->second.begin(), it->second.end(),
			[&textureGuid](const Texture& texture) { return texture.textureGuid == textureGuid; });
	}
	return RenderedTextureListMap.contains(textureGuid);
}

const bool TextureSystem::RenderedTextureListExists(const RenderPassGuid& renderPassGuid) const
{
	return RenderedTextureListMap.find(renderPassGuid) != RenderedTextureListMap.end();
}

void TextureSystem::GenerateTexture(VkGuid& renderPassId)
{
	const VulkanRenderPass renderPass = renderSystem.FindRenderPass(renderPassId);
	const VulkanSubPass subPass = renderPass.SubPassList().front().front();
	if (renderPass.SubPassList().empty() || renderPass.SubPassList().front().empty())
	{
		std::cerr << "[TextureSystem] GenerateTexture: No subpasses defined for render pass!\n";
		return;
	}

	uint32 maxMipLevelCount = 1;
	Vector<Texture> textureList = textureSystem.FindRenderedTextureList(renderPassId);
	for (auto& inputTexture : textureList)
	{
		if (maxMipLevelCount < inputTexture.texture.MipMapLevels()) maxMipLevelCount = inputTexture.texture.MipMapLevels() - 1;
	}

	Vector<VulkanDrawMessage> subPassDrawList;
	subPassDrawList.emplace_back(VulkanDrawMessage
		{
			.RenderPassGuid = renderPass.RenderPassId(),
			.PipelineGuid = subPass.PipelineGuid,
			.PushConstant = subPass.ShaderPushConstant,
			.DrawMeshList = subPass.MeshType == MeshTypeEnum::kMesh_StaticMesh ? meshSystem.DrawMesh(subPass.MeshType) : Vector<MeshDrawMessage>(),
			.RenderPassInputs = subPass.InputTextureList,
			.RenderPassOutputs = subPass.OutputTextureList,
			.OffScreenRenderPass = subPass.OffScreenFrameBuffer
		});

	Vector<RenderPassNode> node
	{
		RenderPassNode
		{
			.RenderPassGuid = renderPass.RenderPassId(),
			.SubPassDrawMessage = { subPassDrawList },
			.MipCount = maxMipLevelCount - 1
		}
	};

	VkCommandBuffer commandBuffer = vulkan.CommandBuffer().BeginSingleUseCommand();
	renderSystem.Draw(commandBuffer, node);
	vulkan.CommandBuffer().EndSingleUseCommand(commandBuffer);
}