#pragma once
#include <DLL.h>
#include <Platform.h>
#include <JsonStruct.h>
#include <RenderSystem.h>
#include <VulkanSystem.h>
#include <TextureSystem.h>
#include <MaterialSystem.h>

class AssetCreatorSystem
{
public:
    static AssetCreatorSystem& Get();

private:
    AssetCreatorSystem() = default;
    ~AssetCreatorSystem() = default;
    AssetCreatorSystem(const AssetCreatorSystem&) = delete;
    AssetCreatorSystem& operator=(const AssetCreatorSystem&) = delete;
    AssetCreatorSystem(AssetCreatorSystem&&) = delete;
    AssetCreatorSystem& operator=(AssetCreatorSystem&&) = delete;
    

public:
     VulkanRenderPass vulkanRenderPass;
     VulkanPipeline   vulkanRenderPipeline;
    DLL_EXPORT void MaterialUpdate(Material& material);
    DLL_EXPORT VkDescriptorImageInfo GetTextureDescriptorbinding(Texture texture);
    DLL_EXPORT void BuildRenderPass(const String& materialPath);
    DLL_EXPORT void Run(String materialPath);
    DLL_EXPORT void Draw();
};
extern DLL_EXPORT AssetCreatorSystem& assetCreatorSystem;
inline AssetCreatorSystem& AssetCreatorSystem::Get()
{
    static AssetCreatorSystem instance;
    return instance;
}

