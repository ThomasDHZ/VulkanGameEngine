#include "GameSystem.h"
#include <imgui/backends/imgui_impl_glfw.h>
#include "TextureSystem.h"
#include "ImGuiRenderer.h"
#include "GameObjectSystem.h"
#include "LevelSystem.h"
#include "MeshSystem.h"
#include <GPUSystem.h>
#include "Mouse.h"
#include "GameController.h"
#include <LevelSystem.h>


GameSystem gameSystem = GameSystem();

GameSystem::GameSystem()
{
}

GameSystem::~GameSystem()
{
}

void GameSystem::StartUp(void* windowHandle, VkInstance& instance, VkSurfaceKHR& surface, VkDebugUtilsMessengerEXT& debugMessenger)
{
    renderSystem.StartUp(windowHandle, instance, surface, debugMessenger);
    gpuSystem.StartUp();
    levelSystem.LoadLevel("Levels/TestLevel.json");
}

void GameSystem::Update(const float& deltaTime)
{
    inputSystem.Update(deltaTime);
    gameObjectSystem.Update(deltaTime);
    levelSystem.Update(deltaTime);
    textureSystem.Update(deltaTime);
    materialSystem.Update(deltaTime);
    renderSystem.Update(vulkanWindow->WindowHandle, levelSystem.spriteRenderPass2DId, levelSystem.levelLayout.LevelLayoutId, deltaTime);

    VkCommandBuffer commandBuffer = renderSystem.BeginSingleTimeCommands();
    meshSystem.Update(deltaTime);
    renderSystem.EndSingleTimeCommands(commandBuffer);
    GameObjectSystem_DestroyDeadGameObjects();
}

void GameSystem::DebugUpdate(const float& deltaTime)
{
    vec2 leftStick = gameController.LeftJoyStickMoved(GLFW_JOYSTICK_1);
    vec2 rightStick = gameController.RightJoyStickMoved(GLFW_JOYSTICK_1);
    vec2 r2L2 = gameController.R2L2Pressed(GLFW_JOYSTICK_1);

    ImGui_StartFrame();
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::Text("Mouse Position: (%.1f, %.1f)", mouse.X, mouse.Y);
    ImGui::Text("Mouse Wheel Offset: (%.1f)", mouse.WheelOffset);
    ImGui::Text("Left Button: %s", mouse.MouseButtonState[0] ? "Pressed" : "Released");
    ImGui::Text("Right Button: %s", mouse.MouseButtonState[1] ? "Pressed" : "Released");
    ImGui::Text("Middle Button: %s", mouse.MouseButtonState[2] ? "Pressed" : "Released");

    ImGui::Text("Left Stick: (%.03f, %.03f)", leftStick.x, leftStick.y);
    ImGui::Text("Right Stick: (%.03f, %.03f)", rightStick.x, rightStick.y);
    ImGui::Text("Up DPad: %s", gameController.ButtonPressed(GLFW_JOYSTICK_1, GLFW_GAMEPAD_BUTTON_DPAD_UP) ? "Pressed" : "Released");
    ImGui::Text("Right DPad: %s", gameController.ButtonPressed(GLFW_JOYSTICK_1, GLFW_GAMEPAD_BUTTON_DPAD_RIGHT) ? "Pressed" : "Released");
    ImGui::Text("Down DPad: %s", gameController.ButtonPressed(GLFW_JOYSTICK_1, GLFW_GAMEPAD_BUTTON_DPAD_DOWN) ? "Pressed" : "Released");
    ImGui::Text("Left DPad: %s", gameController.ButtonPressed(GLFW_JOYSTICK_1, GLFW_GAMEPAD_BUTTON_DPAD_LEFT) ? "Pressed" : "Released");
    ImGui::Text("X button: %s", gameController.ButtonPressed(GLFW_JOYSTICK_1, GLFW_GAMEPAD_BUTTON_CROSS) ? "Pressed" : "Released");
    ImGui::Text("O button: %s", gameController.ButtonPressed(GLFW_JOYSTICK_1, GLFW_GAMEPAD_BUTTON_CIRCLE) ? "Pressed" : "Released");
    ImGui::Text("Square button: %s", gameController.ButtonPressed(GLFW_JOYSTICK_1, GLFW_GAMEPAD_BUTTON_SQUARE) ? "Pressed" : "Released");
    ImGui::Text("Triangle: %s", gameController.ButtonPressed(GLFW_JOYSTICK_1, GLFW_GAMEPAD_BUTTON_TRIANGLE) ? "Pressed" : "Released");
    ImGui::Text("L1 button: %s", gameController.ButtonPressed(GLFW_JOYSTICK_1, GLFW_GAMEPAD_BUTTON_LEFT_BUMPER) ? "Pressed" : "Released");
    ImGui::Text("R1 button: %s", gameController.ButtonPressed(GLFW_JOYSTICK_1, GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER) ? "Pressed" : "Released");
    ImGui::Text("L3 button: %s", gameController.ButtonPressed(GLFW_JOYSTICK_1, GLFW_GAMEPAD_BUTTON_LEFT_THUMB) ? "Pressed" : "Released");
    ImGui::Text("R3 button: %s", gameController.ButtonPressed(GLFW_JOYSTICK_1, GLFW_GAMEPAD_BUTTON_RIGHT_THUMB) ? "Pressed" : "Released");
    ImGui::Text("R2L2: (%.03f, %.03f)", r2L2.x, r2L2.y);
    ImGui_EndFrame();
}

void GameSystem::Draw(const float& deltaTime)
{
    renderSystem.StartFrame();
    levelSystem.Draw(CommandBufferSubmitList, deltaTime);
    CommandBufferSubmitList.emplace_back(ImGui_Draw(renderer, imGuiRenderer));
    renderSystem.EndFrame(CommandBufferSubmitList);
    CommandBufferSubmitList.clear();
}

void GameSystem::Destroy()
{
    ImGui_Destroy(renderer, imGuiRenderer);
    meshSystem.DestroyAllGameObjects();
    materialSystem.DestroyAllMaterials(); 
    textureSystem.DestroyAllTextures(); 
    bufferSystem.DestroyAllBuffers();
    shaderSystem.Destroy();
    Destroy(); 
    levelSystem.DestroyLevel(); 
    memorySystem.ReportLeaks();
}