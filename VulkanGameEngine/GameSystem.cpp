#include "GameSystem.h"
#include <imgui/backends/imgui_impl_glfw.h>
#include "json.h"
#include "TextureSystem.h"
#include <ImGuiRenderer.h>
#include "GameObjectSystem.h"
#include "LevelSystem.h"
#include "MeshSystem.h"

GameSystem gameSystem = GameSystem();

GameSystem::GameSystem()
{
}

GameSystem::~GameSystem()
{
}

void GameSystem::StartUp(WindowType windowType, void* windowHandle)
{
    renderSystem.StartUp(windowType, windowHandle);
    levelSystem.LoadLevel("../Levels/TestLevel.json");
}

void GameSystem::Input(const float& deltaTime)
{
    //for (auto& input : gameObjectSystem.InputComponentList())
    //{
    //    Sprite& sprite = levelSystem.SpriteList[input.GameObjectId];
    //    Transform2DComponent& transform = gameObjectSystem.FindTransform2DComponent(input.GameObjectId);
    //    if (vulkanWindow->keyboard.KeyPressed[KEY_A] == KS_PRESSED ||
    //        vulkanWindow->keyboard.KeyPressed[KEY_A] == KS_HELD)
    //    {
    //        transform.GameObjectPosition.x -= 200.0f * deltaTime;
    //        sprite.SetSpriteAnimation(Sprite::SpriteAnimationEnum::kWalking);
    //        sprite.FlipSprite.x = 0;
    //    }
    //    else if (vulkanWindow->keyboard.KeyPressed[KEY_D] == KS_PRESSED ||
    //             vulkanWindow->keyboard.KeyPressed[KEY_D] == KS_HELD)
    //    {
    //        transform.GameObjectPosition.x += 200.0f * deltaTime;
    //        sprite.SetSpriteAnimation(Sprite::SpriteAnimationEnum::kWalking);
    //        sprite.FlipSprite.x = 1;
    //    }
    //    else
    //    {
    //        sprite.SetSpriteAnimation(Sprite::SpriteAnimationEnum::kStanding);
    //    }
    //}
}

void GameSystem::Update(const float& deltaTime)
{
    levelSystem.Update(deltaTime);
    textureSystem.Update(deltaTime);
    materialSystem.Update(deltaTime);
    renderSystem.Update(levelSystem.spriteRenderPass2DId, levelSystem.levelLayout.LevelLayoutId, deltaTime);

    VkCommandBuffer commandBuffer = renderSystem.BeginSingleTimeCommands();
    meshSystem.Update(deltaTime);
    renderSystem.EndSingleTimeCommands(commandBuffer);
}

void GameSystem::DebugUpdate(const float& deltaTime)
{
    ImGui_StartFrame();
    //ImGui::Begin("Button Window");
   // ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ////texture2.get()->ImGuiShowTexture(ImVec2(256, 128));
    ImGui_EndFrame();
}

void GameSystem::Draw(const float& deltaTime)
{
    renderSystem.StartFrame();
    levelSystem.Draw(CommandBufferSubmitList, deltaTime);
    CommandBufferSubmitList.emplace_back(ImGui_Draw(renderSystem.renderer, imGuiRenderer));
    renderSystem.EndFrame(CommandBufferSubmitList);
    CommandBufferSubmitList.clear();
}

void GameSystem::Destroy()
{
    shaderSystem.Destroy();
    textureSystem.DestroyAllTextures();
    materialSystem.DestroyAllMaterials();
    levelSystem.DestoryLevel();
    meshSystem.DestroyAllGameObjects();
    bufferSystem.DestroyAllBuffers();
    renderSystem.Destroy();
    memorySystem.ReportLeaks();
}