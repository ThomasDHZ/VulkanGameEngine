#include "GameSystem.h"
#include <imgui/backends/imgui_impl_glfw.h>
#include "json.h"
#include "TextureSystem.h"
#include "ImGuiRenderer.h"
#include "GameObjectSystem.h"
#include "LevelSystem.h"
#include "MeshSystem.h"
#include <GPUSystem.h>
#include "Mouse.h"


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
    gpuSystem.StartUp(renderSystem.renderer);
    levelSystem.LoadLevel("../Levels/TestLevel.json");
}

void GameSystem::Update(const float& deltaTime)
{
    inputSystem.Update(deltaTime);
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
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::Text("Mouse Position: (%.1f, %.1f)", mouse.X, mouse.Y);
    ImGui::Text("Mouse Wheel Offset: (%.1f)", mouse.WheelOffset);
    ImGui::Text("Left Button: %s", mouse.MouseButtonState[0] ? "Pressed" : "Released");
    ImGui::Text("Right Button: %s", mouse.MouseButtonState[1] ? "Pressed" : "Released");
    ImGui::Text("Middle Button: %s", mouse.MouseButtonState[2] ? "Pressed" : "Released");
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
    meshSystem.DestroyAllGameObjects();
    materialSystem.DestroyAllMaterials(); 
    textureSystem.DestroyAllTextures(); 
    bufferSystem.DestroyAllBuffers(); 
    shaderSystem.Destroy();
    renderSystem.Destroy(); 
    levelSystem.DestroyLevel(); 
    memorySystem.ReportLeaks();
}