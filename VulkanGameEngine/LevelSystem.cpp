#include "LevelSystem.h"
#include "MaterialSystem.h"
#include "TextureSystem.h"
#include "GameObjectSystem.h"
#include "MeshSystem.h"
#include "VRAM.h"
#include "SpriteSystem.h"
#include <VulkanShaderSystem.h>
#include "EngineConfigSystem.h"
#include <FileSystem.h>

LevelSystem levelSystem = LevelSystem();

LevelSystem::LevelSystem()
{

}

LevelSystem::~LevelSystem()
{

}

void LevelSystem::LoadLevel(const String& levelPath)
{
    OrthographicCamera = std::make_shared<OrthographicCamera2D>(OrthographicCamera2D(vec2((float)renderer.SwapChainResolution.width, (float)renderer.SwapChainResolution.height), vec3(0.0f, 0.0f, 0.0f)));

    VkGuid dummyGuid = VkGuid();
    VkGuid tileSetId = VkGuid();

    shaderSystem.CompileShaders(configSystem.ShaderSourceDirectory.c_str(), configSystem.CompiledShaderOutputDirectory.c_str());

    nlohmann::json json = fileSystem.LoadJsonFile(levelPath);
    nlohmann::json shaderJson = fileSystem.LoadJsonFile("../RenderPass/LevelShader2DRenderPass.json");
    nlohmann::json shaderWiredJson = fileSystem.LoadJsonFile("../RenderPass/LevelShader2DWireFrameRenderPass.json");
    spriteRenderPass2DId = VkGuid(shaderJson["RenderPassId"].get<String>().c_str());
    levelWireFrameRenderPass2DId = VkGuid(shaderWiredJson["RenderPassId"].get<String>().c_str());
    shaderSystem.LoadShaderPipelineStructPrototypes(json["LoadRenderPasses"]);

    for (size_t x = 0; x < json["LoadTextures"].size(); x++)
    {
        textureSystem.LoadTexture(json["LoadTextures"][x]);
    }

    for (size_t x = 0; x < json["LoadMaterials"].size(); x++)
    {
        materialSystem.LoadMaterial(json["LoadMaterials"][x]);
    }

    for (size_t x = 0; x < json["LoadSpriteVRAM"].size(); x++)
    {
        spriteSystem.LoadSpriteVRAM(json["LoadSpriteVRAM"][x]);
    }

    for (size_t x = 0; x < json["LoadTileSetVRAM"].size(); x++)
    {
        tileSetId = LoadTileSetVRAM(json["LoadTileSetVRAM"][x]);
    }

    for (size_t x = 0; x < json["GameObjectList"].size(); x++)
    {
        String objectJson = json["GameObjectList"][x]["GameObjectPath"];
        vec2 positionOverride(json["GameObjectList"][x]["GameObjectPositionOverride"][0], json["GameObjectList"][x]["GameObjectPositionOverride"][1]);
        gameObjectSystem.CreateGameObject(objectJson, positionOverride);
    }

    LoadLevelLayout(json["LoadLevelLayout"]);
    LoadLevelMesh(tileSetId);

    VkGuid LevelId = VkGuid(json["LevelID"].get<String>().c_str());
    spriteRenderPass2DId = renderSystem.LoadRenderPass(levelLayout.LevelLayoutId, "../RenderPass/LevelShader2DRenderPass.json", ivec2(renderer.SwapChainResolution.width, renderer.SwapChainResolution.height));
//    levelWireFrameRenderPass2DId = LoadRenderPass(levelLayout.LevelLayoutId, "../RenderPass/LevelShader2DWireFrameRenderPass.json", ivec2(renderer.SwapChainResolution.width, renderer.SwapChainResolution.height));
    gaussianBlurRenderPassId = renderSystem.LoadRenderPass(dummyGuid, "../RenderPass/GaussianBlurRenderPass.json", ivec2(renderer.SwapChainResolution.width, renderer.SwapChainResolution.height));
    frameBufferId = renderSystem.LoadRenderPass(dummyGuid, "../RenderPass/FrameBufferRenderPass.json", ivec2(renderer.SwapChainResolution.width, renderer.SwapChainResolution.height));
}


void LevelSystem::Update(const float& deltaTime)
{
    OrthographicCamera->Update(*shaderSystem.GetGlobalShaderPushConstant("sceneData"));
    spriteSystem.Update(deltaTime);
    shaderSystem.UpdateGlobalShaderBuffer("sceneData");
}

void LevelSystem::Draw(Vector<VkCommandBuffer>& commandBufferList, const float& deltaTime)
{
    commandBufferList.emplace_back(renderSystem.RenderLevel(spriteRenderPass2DId, levelLayout.LevelLayoutId, deltaTime));
    commandBufferList.emplace_back(renderSystem.RenderBloomPass(gaussianBlurRenderPassId));
    commandBufferList.emplace_back(renderSystem.RenderFrameBuffer(frameBufferId));
}

void LevelSystem::DestroyDeadGameObjects()
{
    // Optional logic for cleaning up dead game objects
    // (commented out, implement as needed)
    /*
    if (gameObjectSystem.GameObjectList.empty()) return;

    Vector<SharedPtr<GameObject>> deadGameObjects;
    for (auto& gameObject : gameObjectSystem.GameObjectList)
    {
        if (!gameObject->GameObjectAlive)
            deadGameObjects.push_back(gameObject);
    }
    for (auto& gameObject : deadGameObjects)
    {
        // Remove sprite components if any
        // gameObject->Destroy();
    }
    */
}

VkGuid LevelSystem::LoadTileSetVRAM(const String& tileSetPath)
{
    return Level_LoadTileSetVRAM(tileSetPath);
}

void LevelSystem::LoadLevelLayout(const String& levelLayoutPath)
{
    Level_LoadLevelLayout(levelLayoutPath);
}

void LevelSystem::LoadLevelMesh(VkGuid& tileSetId)
{
    Level_LoadLevelMesh(tileSetId);
}

void LevelSystem::DestroyLevel()
{
    Level_DestroyLevel();
}