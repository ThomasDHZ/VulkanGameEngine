#include "GameSystem.h"
#include <imgui_impl_glfw.h>
#include "TextureSystem.h"
#include "ImGuiRenderer.h"
#include "GameObjectSystem.h"
#include "LevelSystem.h"
#include "MeshSystem.h"
#include "Mouse.h"
#include "GameController.h"
#include <MaterialBakerSystem.h>
#include <LevelSystem.h>
#include <VkGuid.h>
#ifdef PLATFORM_ANDROID
#include <android/native_window.h>
#endif

#ifndef __ANDROID__
GameSystem gameSystem = GameSystem();
#endif

void GameSystem::InitPrecomputedMaps() {
    static bool s_mapsGenerated = false;
    if (s_mapsGenerated) {
        printf("[InitPrecomputedMaps] Already generated — skipping\n");
        return;
    }
    s_mapsGenerated = true;
    printf("[InitPrecomputedMaps] Starting one-time BRDF LUT + default cubemap generation\n");

    SceneDataBuffer& sceneDataBuffer = memoryPoolSystem.UpdateSceneDataBuffer();
    VkGuid dummyGuid = VkGuid();

    // 1. Full sync + cleanup old resources
    printf("Waiting for device idle before cleanup...\n");
    vkDeviceWaitIdle(vulkanSystem.Device);
    if (sceneDataBuffer.BRDFMapId != UINT32_MAX) {
        printf("Destroying old BRDF texture index %u\n", sceneDataBuffer.BRDFMapId);
        textureSystem.DestroyTexture(sceneDataBuffer.BRDFMapId);
    }
    if (sceneDataBuffer.CubeMapId != UINT32_MAX) {
        printf("Destroying old cubemap index %u\n", sceneDataBuffer.CubeMapId);
        textureSystem.DestroyCubeTexture(sceneDataBuffer.CubeMapId);
    }
    sceneDataBuffer.BRDFMapId = UINT32_MAX;
    sceneDataBuffer.CubeMapId = UINT32_MAX;

    // Clear stale list entries
    while (!textureSystem.TextureList.empty() &&
        textureSystem.TextureList.back().textureImage == VK_NULL_HANDLE) {
        textureSystem.TextureList.pop_back();
    }
    while (!textureSystem.CubeMapTextureList.empty() &&
        textureSystem.CubeMapTextureList.back().textureImage == VK_NULL_HANDLE) {
        textureSystem.CubeMapTextureList.pop_back();
    }
    printf("Cleanup complete. TextureList size = %zu | CubeMapList size = %zu\n",
        textureSystem.TextureList.size(), textureSystem.CubeMapTextureList.size());

    // 2. Generate BRDF LUT
    printf("Loading & generating BRDF LUT...\n");
    levelSystem.brdfRenderPassId = renderSystem.LoadRenderPass(dummyGuid, "RenderPass/BRDFRenderPass.json", false);
    vkQueueWaitIdle(vulkanSystem.GraphicsQueue);

    renderSystem.GenerateTexture(levelSystem.brdfRenderPassId);

    sceneDataBuffer.BRDFMapId = textureSystem.TextureList.size();
    textureSystem.TextureList.emplace_back(textureSystem.FindRenderedTextureList(levelSystem.brdfRenderPassId).back());

    printf("BRDF generated — new image handle = %p\n",
        (void*)textureSystem.TextureList.back().textureImage);

    // Reset barrier for BRDF (after creation!)
    VkCommandBuffer cmdReset = vulkanSystem.BeginSingleUseCommand();
    VkImageMemoryBarrier resetBRDF{};
    resetBRDF.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    resetBRDF.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    resetBRDF.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    resetBRDF.srcAccessMask = 0;
    resetBRDF.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    resetBRDF.image = textureSystem.TextureList.back().textureImage;
    resetBRDF.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    vkCmdPipelineBarrier(cmdReset,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        0, 0, nullptr, 0, nullptr, 1, &resetBRDF);
    vulkanSystem.EndSingleUseCommand(cmdReset);

    vkQueueWaitIdle(vulkanSystem.GraphicsQueue);

    // 3. Generate default cubemap
    printf("Loading & generating default environment cubemap...\n");
    levelSystem.environmentToCubeMapRenderPassId = renderSystem.LoadRenderPass(
        dummyGuid, "RenderPass/EnvironmentToCubeMapRenderPass.json", false);
    vkQueueWaitIdle(vulkanSystem.GraphicsQueue);

    renderSystem.GenerateCubeMapTexture(levelSystem.environmentToCubeMapRenderPassId);

    sceneDataBuffer.CubeMapId = textureSystem.CubeMapTextureList.size();
    textureSystem.CubeMapTextureList.emplace_back(
        textureSystem.FindRenderedTextureList(levelSystem.environmentToCubeMapRenderPassId).back());

    printf("Cubemap generated — new image handle = %p\n",
        (void*)textureSystem.CubeMapTextureList.back().textureImage);

    // Reset barrier for cubemap
    cmdReset = vulkanSystem.BeginSingleUseCommand();
    VkImageMemoryBarrier resetCube{};
    resetCube.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    resetCube.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    resetCube.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    resetCube.srcAccessMask = 0;
    resetCube.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    resetCube.image = textureSystem.CubeMapTextureList.back().textureImage;
    resetCube.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, 6 };
    vkCmdPipelineBarrier(cmdReset,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        0, 0, nullptr, 0, nullptr, 1, &resetCube);
    vulkanSystem.EndSingleUseCommand(cmdReset);

    vkQueueWaitIdle(vulkanSystem.GraphicsQueue);

    // 4. Final transition to SHADER_READ_ONLY_OPTIMAL
    printf("Transitioning precomputed maps to shader-readable layout...\n");
    VkCommandBuffer cmd = VK_NULL_HANDLE;
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = vulkanSystem.CommandPool;
    allocInfo.commandBufferCount = 1;
    vkAllocateCommandBuffers(vulkanSystem.Device, &allocInfo, &cmd);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd, &beginInfo);

    VkImageMemoryBarrier barriers[2]{};
    // BRDF
    barriers[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barriers[0].oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barriers[0].newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barriers[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barriers[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barriers[0].image = textureSystem.TextureList.back().textureImage;  // still BRDF (last 2D)
    barriers[0].subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    // Cubemap (now last in list)
    barriers[1].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barriers[1].oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barriers[1].newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barriers[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barriers[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barriers[1].image = textureSystem.CubeMapTextureList.back().textureImage;
    barriers[1].subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, 6 };

    vkCmdPipelineBarrier(cmd,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0, 0, nullptr, 0, nullptr, 2, barriers);

    vkEndCommandBuffer(cmd);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;
    vkQueueSubmit(vulkanSystem.GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(vulkanSystem.GraphicsQueue);
    vkFreeCommandBuffers(vulkanSystem.Device, vulkanSystem.CommandPool, 1, &cmd);

    printf("Precomputed maps transitioned and ready\n");

    vkDeviceWaitIdle(vulkanSystem.Device);
}

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
    meshSystem.StartUp();
    materialSystem.StartUp();
    lightSystem.StartUp();
    memoryPoolSystem.StartUp();

#if defined(_WIN32)
    shaderSystem.CompileShaders(configSystem.ShaderSourceDirectory.c_str(), configSystem.CompiledShaderOutputDirectory.c_str());
    // materialBakerSystem.Run();  // Uncomment if needed
#endif
    InitPrecomputedMaps();
    levelSystem.LoadLevel("Levels/TestLevel.json");
}

#ifndef __ANDROID__
void Update(float deltaTime);
void GameSystem::Update(float deltaTime)
{
    inputSystem.Update(deltaTime);
    gameObjectSystem.Update(deltaTime);
    levelSystem.Update(deltaTime);
    spriteSystem.Update(deltaTime);
    meshSystem.Update(deltaTime);
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


        ImGui::SliderInt("UseHeightMap ", &levelSystem.UseHeightMap, 0, 1);
        ImGui::SliderFloat("HeightScale ", &levelSystem.HeightScale, 0.0f, 1.0f);
        ImGui::SliderFloat3("ViewDirection ", &levelSystem.ViewDirection.x, -1.0f, 1.0f);


    ImGui::Separator();

    //for (int x = 0; x < memoryPoolSystem.MemoryPoolSubBufferInfo(kDirectionalLightBuffer).ActiveCount; x++)
    //{
    //    DirectionalLight& directionalLight = memoryPoolSystem.UpdateDirectionalLight(x);
    //    if (ImGui::SliderFloat3("DLightColor ", &directionalLight.LightColor.x, 0.0f, 1.0f)) memoryPoolSystem.MarkMemoryPoolBufferDirty();
    //    if (ImGui::SliderFloat3("DLightDirection ", &directionalLight.LightDirection.x, -1.0f, 1.0f)) memoryPoolSystem.MarkMemoryPoolBufferDirty();
    //    if (ImGui::SliderFloat("DLightIntensity ", &directionalLight.LightIntensity, 0.0f, 10.0f)) memoryPoolSystem.MarkMemoryPoolBufferDirty();
    //    if (ImGui::SliderFloat("ShadowBias ", &directionalLight.ShadowBias, 0.0f, 10.0f)) memoryPoolSystem.MarkMemoryPoolBufferDirty();
    //    if (ImGui::SliderFloat("ShadowSoftness ", &directionalLight.ShadowSoftness, 0.0f, 10.0f)) memoryPoolSystem.MarkMemoryPoolBufferDirty();
    //    if (ImGui::SliderFloat("ShadowStrength ", &directionalLight.ShadowStrength, 0.0f, 10.0f)) memoryPoolSystem.MarkMemoryPoolBufferDirty();
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


    ImGui::Separator();

    //ImGui::Image((ImTextureID)textureSystem.FindDepthTexture(levelSystem.sdfShaderRenderPassId).ImGuiDescriptorSet, ImVec2(400, 300));


    ImGui_EndFrame();
}

void GameSystem::Draw(float deltaTime)
{
    vulkanSystem.StartFrame();
    VkCommandBuffer commandBuffer = vulkanSystem.CommandBuffers[vulkanSystem.CommandIndex];
    //materialBakerSystem.Draw(commandBuffer);
    levelSystem.Draw(commandBuffer, deltaTime);
    //ImGui_Draw(commandBuffer, imGuiRenderer);
    vulkanSystem.EndFrame(commandBuffer);
}

void GameSystem::Destroy()
{
   

}