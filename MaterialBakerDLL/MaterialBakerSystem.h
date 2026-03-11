#pragma once
#include <DLL.h>
#include <Platform.h>
#include <JsonStruct.h>
#include <RenderSystem.h>
#include <VulkanSystem.h>
#include <TextureSystem.h>
#include <MaterialSystem.h>
#include "TextureBakerSystem.h"

class MaterialBakerSystem
{
public:
    static MaterialBakerSystem& Get();

private:
    MaterialBakerSystem() = default;
    ~MaterialBakerSystem() = default;
    MaterialBakerSystem(const MaterialBakerSystem&) = delete;
    MaterialBakerSystem& operator=(const MaterialBakerSystem&) = delete;
    MaterialBakerSystem(MaterialBakerSystem&&) = delete;
    MaterialBakerSystem& operator=(MaterialBakerSystem&&) = delete;

    Vector<Texture>                         TextureList;
    VulkanRenderPass                        vulkanRenderPass;
    VulkanPipeline                          vulkanRenderPipeline;

    bool                                    UpdateNeeded(const String& materialPath);
    void                                    TransitionImageLayout(Texture& texture, VkImageLayout newLayout, uint32 baseMipLevel = 0, uint32 levelCount = VK_REMAINING_MIP_LEVELS);
    void                                    CreateTextureImage(Texture& texture, VkImageCreateInfo& imageCreateInfo, Vector<byte>& textureData, uint layerCount);
    void                                    CreateTextureView(Texture& texture, VkImageAspectFlags imageAspectFlags);
    Texture                                 LoadTexture(TextureLoader textureLoader, VkSampler sampler);
    void                                    LoadMaterial(const String& materialPath);
    void                                    CleanInputResources();
    void                                    CleanRenderPass();
    void                                    BuildRenderPass(ivec2 renderPassResolution);

public:

    DLL_EXPORT void Run();
    DLL_EXPORT void Draw(VkCommandBuffer& commandBuffer);
};

extern DLL_EXPORT MaterialBakerSystem& materialBakerSystem;
inline MaterialBakerSystem& MaterialBakerSystem::Get()
{
    static MaterialBakerSystem instance;
    return instance;
}