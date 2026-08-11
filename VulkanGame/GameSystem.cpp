#include "GameSystem.h"
#include "TextureSystem.h"
#include <ImGuiSystem.h>
#include "GameObjectSystem.h"
#include "LevelSystem.h"
#include "MeshSystem.h"
#include "Mouse.h"
#include "GameController.h"
#include <LevelSystem.h>
#include <CSharpScriptSystem.h>
#include <ChatSystem.h>
#include <GameController.h>

#ifdef PLATFORM_ANDROID
#include <android/native_window.h>
#endif
#include <LuaScriptingSystem.h>

#ifndef __ANDROID__
GameSystem gameSystem = GameSystem();
#endif
#include <CollisionSystem.h>
#include <EngineConfigSystem.h>
#include <NetworkSystem.h>
#include <InputSystem.h>
#include <TextSystem.h>

GameSystem::GameSystem()
{

}

GameSystem::~GameSystem()
{

}

void GameSystem::StartUp()
{
    vulkan.VulkanSetUp(configSystem.WindowResolution, configSystem.RenderResolution);
    bufferSystem.SetUpVmaAllocation();
    textSystem.StartUp();
    textSystem.SetFont("fonts/arial.ttf");
    memoryPoolSystem.StartUp();
    networkSystem.StartAsServer(7777);
    //luaScriptingSystem.StartUp();
    cSharpScriptSystem.Initialize();
#if defined(_WIN32)
    shaderSystem.CompileShaders(configSystem.ShaderSourceDirectory.c_str(), configSystem.CompiledShaderOutputDirectory.c_str());
    // materialBakerSystem.Run();
#endif
    levelSystem.LoadLevel("Levels/TestLevel.json");
    //String text = String("asdfasdf");
   // textSystem.RenderText(text, vec2(200.0f), 32, vec3(1.0f));
}

#ifndef __ANDROID__
void GameSystem::Update(void* windowHandle, float deltaTime)
{

    //luaScriptingSystem.Update(deltaTime);
    gameObjectSystem.Update(deltaTime);
    levelSystem.Update(deltaTime);
    collisionSystem.Update();
    spriteSystem.Update(deltaTime);
    meshSystem.Update(deltaTime);
    memoryPoolSystem.UpdateMemoryPool();
    renderSystem.Update(windowHandle, deltaTime);
    inputSystem.Update(deltaTime);
    networkSystem.Update(deltaTime);

    //cSharpScriptSystem.Update(deltaTime);
    auto a = VkGuid("7047804f-d32e-4cb5-ba95-90783b28d1df");
  //  renderSystem.SampleRenderPassPixel(a, ivec2(mouse.X, mouse.Y));
   // renderSystem.Update(vulkanWindow->WindowHandle, levelSystem.levelLayout.LevelLayoutId, deltaTime);
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

    imGuiSystem.StartFrame();
    imGuiSystem.FpsDisplay();
    imGuiSystem.SliderInt("UseHeightMap ", &levelSystem.UseHeightMap, 0, 1);
    imGuiSystem.SliderFloat("HeightScale ", &levelSystem.HeightScale, 0.0f, 1.0f);
    imGuiSystem.SliderFloat3("ViewDirection ", &levelSystem.ViewDirection.x, -1.0f, 1.0f);
    imGuiSystem.Separator();

    // Current status
    const char* modeNames[] = { "None", "Server", "Client" };
    imGuiSystem.Text("Current Mode: %s", modeNames[(int)networkSystem.GetNetworkMode()]);
    imGuiSystem.Text("Connected: %s", networkSystem.IsConnected() ? "Yes" : "No");

    imGuiSystem.Separator();

    // --- Server ---
    if (imGuiSystem.Button("Start Server"))
    {
        networkSystem.StartAsServer(7777);
    }

    // --- Client ---
    static char ipBuffer[64] = "127.0.0.1";
    std::array<char, 64> ip = { 0 };
    std::snprintf(ip.data(), ip.size(), "%s", ipBuffer);
    static int port = 7777;

    imGuiSystem.InputText("Server IP", ipBuffer, sizeof(ipBuffer));
    imGuiSystem.InputInt("Port", &port);

    if (imGuiSystem.Button("Start Client"))
    {
        if (networkSystem.StartAsClient())
        {
            networkSystem.ConnectToServer(ip, static_cast<uint16_t>(port));
        }
    }

    imGuiSystem.SameLine();
    if (imGuiSystem.Button("Stop / Disconnect"))
    {
        networkSystem.Stop();
    }

    imGuiSystem.Separator();

    //for (int x = 0; x < memoryPoolSystem.MemoryPoolSubBufferInfo(kDirectionalLightBuffer).ActiveCount; x++)
    //{
    //    DirectionalLight& directionalLight = memoryPoolSystem.UpdateDirectionalLight(x);
    //    if (ImGui::SliderFloat3("DLightColor ", &directionalLight.LightColor.x, 0.0f, 1.0f));
    //    if (ImGui::SliderFloat3("DLightDirection ", &directionalLight.LightDirection.x, -1.0f, 1.0f));
    //    if (ImGui::SliderFloat("DLightIntensity ", &directionalLight.LightIntensity, 0.0f, 10.0f));
    //    if (ImGui::SliderFloat("ShadowBias ", &directionalLight.ShadowBias, 0.0f, 10.0f));
    //    if (ImGui::SliderFloat("ShadowSoftness ", &directionalLight.ShadowSoftness, 0.0f, 10.0f));
    //    if (ImGui::SliderFloat("ShadowStrength ", &directionalLight.ShadowStrength, 0.0f, 10.0f));
    //}

    //ImGui::Separator();

    //for (int x = 0; x < memoryPoolSystem.MemoryPoolSubBufferInfo(kDirectionalLightBuffer).ActiveCount; x++)
    //{
    //    PointLight& pointLight = memoryPoolSystem.UpdatePointLight(x);
    //    if (ImGui::SliderFloat3("PLightPosition", &pointLight.LightPosition.x, -static_cast<float>(vulkanSystem.SwapChainResolution.width), static_cast<float>(vulkanSystem.SwapChainResolution.width))) memoryPoolSystem.MarkMemoryPoolBufferDirty();
    //    if (ImGui::SliderFloat3("PLightColor ", &pointLight.LightColor.x, 0.0f, 1.0f)) memoryPoolSystem.MarkMemoryPoolBufferDirty();
    //    if (ImGui::SliderFloat("PLightRadius ", &pointLight.LightRadius, 0.0f, 500.0f)) memoryPoolSystem.MarkMemoryPoolBufferDirty();
    //    if (ImGui::SliderFloat("PLightIntensity ", &pointLight.LightIntensity, 0.0f, 50.0f)) memoryPoolSystem.MarkMemoryPoolBufferDirty();
    //}


  //  imGuiSystem.Separator();
  ////  uint32_t hoverId = renderSystem.SampleRenderPassPixel(VkGuid("7047804f-d32e-4cb5-ba95-90783b28d1df"), ivec2(mouse.X, mouse.Y));
  //  imGuiSystem.Checkbox("Show Wireframe View", &renderSystem.WireFrameFlag);

  //  imGuiSystem.Text("Mouse Position: (%.1f, %.1f)", mouse.X, mouse.Y);
  //  imGuiSystem.Text("Mouse Wheel Offset: (%.1f)", mouse.WheelOffset);
  //  //imGuiSystem.Text("Mouse Hover Id: %u", hoverId);
  //  imGuiSystem.Text("Left Button: %s", mouse.MouseButtonState[0] ? "Pressed" : "Released");
  //  imGuiSystem.Text("Right Button: %s", mouse.MouseButtonState[1] ? "Pressed" : "Released");
  //  imGuiSystem.Text("Middle Button: %s", mouse.MouseButtonState[2] ? "Pressed" : "Released");

  //  imGuiSystem.Separator();

  //  imGuiSystem.Text("Left Stick: (%.03f, %.03f)", leftStick.x, leftStick.y);
  //  imGuiSystem.Text("Right Stick: (%.03f, %.03f)", rightStick.x, rightStick.y);
  //  imGuiSystem.Text("Up DPad: %s", gameController.ButtonPressed(GLFW_JOYSTICK_1, GLFW_GAMEPAD_BUTTON_DPAD_UP) ? "Pressed" : "Released");
  //  imGuiSystem.Text("Right DPad: %s", gameController.ButtonPressed(GLFW_JOYSTICK_1, GLFW_GAMEPAD_BUTTON_DPAD_RIGHT) ? "Pressed" : "Released");
  //  imGuiSystem.Text("Down DPad: %s", gameController.ButtonPressed(GLFW_JOYSTICK_1, GLFW_GAMEPAD_BUTTON_DPAD_DOWN) ? "Pressed" : "Released");
  //  imGuiSystem.Text("Left DPad: %s", gameController.ButtonPressed(GLFW_JOYSTICK_1, GLFW_GAMEPAD_BUTTON_DPAD_LEFT) ? "Pressed" : "Released");
  //  imGuiSystem.Text("X button: %s", gameController.ButtonPressed(GLFW_JOYSTICK_1, GLFW_GAMEPAD_BUTTON_CROSS) ? "Pressed" : "Released");
  //  imGuiSystem.Text("O button: %s", gameController.ButtonPressed(GLFW_JOYSTICK_1, GLFW_GAMEPAD_BUTTON_CIRCLE) ? "Pressed" : "Released");
  //  imGuiSystem.Text("Square button: %s", gameController.ButtonPressed(GLFW_JOYSTICK_1, GLFW_GAMEPAD_BUTTON_SQUARE) ? "Pressed" : "Released");
  //  imGuiSystem.Text("Triangle: %s", gameController.ButtonPressed(GLFW_JOYSTICK_1, GLFW_GAMEPAD_BUTTON_TRIANGLE) ? "Pressed" : "Released");
  //  imGuiSystem.Text("L1 button: %s", gameController.ButtonPressed(GLFW_JOYSTICK_1, GLFW_GAMEPAD_BUTTON_LEFT_BUMPER) ? "Pressed" : "Released");
  //  imGuiSystem.Text("R1 button: %s", gameController.ButtonPressed(GLFW_JOYSTICK_1, GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER) ? "Pressed" : "Released");
  //  imGuiSystem.Text("L3 button: %s", gameController.ButtonPressed(GLFW_JOYSTICK_1, GLFW_GAMEPAD_BUTTON_LEFT_THUMB) ? "Pressed" : "Released");
  //  imGuiSystem.Text("R3 button: %s", gameController.ButtonPressed(GLFW_JOYSTICK_1, GLFW_GAMEPAD_BUTTON_RIGHT_THUMB) ? "Pressed" : "Released");
  //  imGuiSystem.Text("R2L2: (%.03f, %.03f)", r2L2.x, r2L2.y);

  //  imGuiSystem.Separator();

    //ImGui::Image((ImTextureID)textureSystem.FindDepthTexture(levelSystem.ShaderRenderPassId).ImGuiDescriptorSet, ImVec2(400, 300));

  //  chatSystem.DrawChatWindow();
    imGuiSystem.EndFrame();
}

void GameSystem::Draw(float deltaTime)
{
    vulkan.Swapchain().StartFrame();
    commandBuffer = vulkan.CommandBufferList()[vulkan.Swapchain().CommandIndex()];
   // materialBakerSystem.Draw(commandBuffer);
    Vector<RenderPassNode> renderNodes = levelSystem.Draw(commandBuffer, deltaTime);
    renderSystem.Draw(commandBuffer, renderNodes);
    levelSystem.RenderFrameBuffer(commandBuffer, levelSystem.frameBufferId);
    imGuiSystem.Draw(commandBuffer);
    vulkan.Swapchain().EndFrame(commandBuffer);
}

void GameSystem::Destroy()
{
   

}