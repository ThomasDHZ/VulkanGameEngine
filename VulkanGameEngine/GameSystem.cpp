#include "GameSystem.h"
#include <imgui_impl_glfw.h>
#include "TextureSystem.h"
#include "ImGuiRenderer.h"
#include "GameObjectSystem.h"
#include "LevelSystem.h"
#include "MeshSystem.h"
#include "Mouse.h"
#include "GameController.h"
#include <LevelSystem.h>
#ifdef PLATFORM_ANDROID
#include <android/native_window.h>
#endif

#ifndef __ANDROID__
    GameSystem gameSystem = GameSystem();
#endif

GameSystem::GameSystem()
{

}

GameSystem::~GameSystem()
{

}

void GameSystem::StartUp(void* windowHandle)
{
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkInstance instance = vulkanSystem.CreateVulkanInstance();
#ifdef PLATFORM_ANDROID
    ANativeWindow* nativeWindow = (ANativeWindow*)windowHandle;

    VkAndroidSurfaceCreateInfoKHR surfaceInfo = { VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR };
    surfaceInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    surfaceInfo.pNext = nullptr;
    surfaceInfo.flags = 0;
    surfaceInfo.window = nativeWindow;

    VkResult result = vkCreateAndroidSurfaceKHR(instance, &surfaceInfo, nullptr, &surface);
    if (result != VK_SUCCESS || surface == VK_NULL_HANDLE)
    {
        __android_log_print(ANDROID_LOG_ERROR, "VulkanEngine", "FATAL: vkCreateAndroidSurfaceKHR failed! Result: %d", result);
        return;
    }

    __android_log_print(ANDROID_LOG_INFO, "VulkanEngine", "Android surface created successfully: %p", surface);
#else
    glfwCreateWindowSurface(instance, (GLFWwindow*)vulkanWindow->WindowHandle, NULL, &surface);
#endif
    renderSystem.StartUp(windowHandle, instance, surface);
    levelSystem.LoadLevel("Levels/TestLevel.json");
}

#ifndef __ANDROID__
void Update(float deltaTime);
void GameSystem::Update(float deltaTime)
{
    inputSystem.Update(deltaTime);
    gameObjectSystem.Update(deltaTime);
    levelSystem.Update(deltaTime);
    textureSystem.Update(deltaTime);
    materialSystem.Update(deltaTime);
    renderSystem.Update(vulkanWindow->WindowHandle, levelSystem.levelLayout.LevelLayoutId, deltaTime);

    gameObjectSystem.DestroyDeadGameObjects();
}

#else
void GameSystem::Update(void* windowHandle, float deltaTime)
{
    inputSystem.Update(deltaTime);
    gameObjectSystem.Update(deltaTime);
    levelSystem.Update(deltaTime);
    textureSystem.Update(deltaTime);
    materialSystem.Update(deltaTime);

    renderSystem.Update(windowHandle, levelSystem.spriteRenderPass2DId, levelSystem.levelLayout.LevelLayoutId, deltaTime);

    VkCommandBuffer commandBuffer = renderSystem.BeginSingleUseCommand();
    meshSystem.Update(deltaTime);
    renderSystem.EndSingleUseCommand(commandBuffer);
    gameObjectSystem.DestroyDeadGameObjects();
}
#endif

void GameSystem::DebugUpdate(float deltaTime)
{
    vec2 leftStick = gameController.LeftJoyStickMoved(GLFW_JOYSTICK_1);
    vec2 rightStick = gameController.RightJoyStickMoved(GLFW_JOYSTICK_1);
    vec2 r2L2 = gameController.R2L2Pressed(GLFW_JOYSTICK_1);

    ImGui_StartFrame();
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    
    ImGui::Separator();

    for (auto& directionalLight : lightSystem.DirectionalLightList)
    {
        ImGui::SliderFloat3("DLightColor ", &directionalLight.LightColor.x, 0.0f, 1.0f);
        ImGui::SliderFloat3("DLightDirection ", &directionalLight.LightDirection.x, -1.0f, 1.0f);
        ImGui::SliderFloat("DLightIntensity ", &directionalLight.LightIntensity, 0.0f, 10.0f);
    }

    ImGui::Separator();

    for (auto& pointLight : lightSystem.PointLightList)
    {
        ImGui::SliderFloat("PLightPosition X", &pointLight.LightPosition.x, 0.0f, static_cast<float>(vulkanSystem.SwapChainResolution.width));
        ImGui::SliderFloat("PLightPosition Y", &pointLight.LightPosition.y, 0.0f, static_cast<float>(vulkanSystem.SwapChainResolution.height));
        ImGui::SliderFloat3("PLightColor ", &pointLight.LightColor.x, 0.0f, 1.0f);
        ImGui::SliderFloat("PLightRadius ", &pointLight.LightRadius, 0.0f, 500.0f);
        ImGui::SliderFloat("PLightIntensity ", &pointLight.LightIntensity, 0.0f, 50.0f);
    }

    ImGui::Separator();

    ImGui::Text("Mouse Position: (%.1f, %.1f)", mouse.X, mouse.Y);
    ImGui::Text("Mouse Wheel Offset: (%.1f)", mouse.WheelOffset);
    ImGui::Text("Left Button: %s", mouse.MouseButtonState[0] ? "Pressed" : "Released");
    ImGui::Text("Right Button: %s", mouse.MouseButtonState[1] ? "Pressed" : "Released");
    ImGui::Text("Middle Button: %s", mouse.MouseButtonState[2] ? "Pressed" : "Released");

    ImGui::Separator();

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

void GameSystem::Draw(float deltaTime)
{
    vulkanSystem.StartFrame();
    VkCommandBuffer commandBuffer = vulkanSystem.CommandBuffers[vulkanSystem.CommandIndex];
    levelSystem.Draw(commandBuffer, deltaTime);
    ImGui_Draw(commandBuffer, imGuiRenderer);
    vulkanSystem.EndFrame(commandBuffer);
}

void GameSystem::Destroy()
{
    ImGui_Destroy(imGuiRenderer);
    meshSystem.DestroyAllGameObjects();
    materialSystem.DestroyAllMaterials(); 
    textureSystem.DestroyAllTextures(); 
    bufferSystem.DestroyAllBuffers();
    levelSystem.DestroyLevel(); 
    renderSystem.Destroy();
    memorySystem.ReportLeaks();
}