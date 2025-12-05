#include "VulkanWindow.h"
#include "SystemClock.h"
#include <iostream>
#include "FrameTimer.h"
#include "GameSystem.h"
#include "EngineConfigSystem.h"
#include "ImGuiRenderer.h"
#include <DebugSystem.h>

int main()
{
    SystemClock systemClock = SystemClock();
    FrameTimer deltaTime = FrameTimer();

    std::cout << "Base Directory: " << std::filesystem::current_path() << std::endl;
#if defined(_WIN32)
    if(!debugSystem.IsRenderDocInjected())
    {
       debugSystem.SetRootDirectory("../Assets");
    }
#else
    debugSystem.SetRootDirectory("Assets");
#endif

    try
    {
        vulkanWindow = new GameEngineWindow();
        vulkanWindow->CreateGraphicsWindow(vulkanWindow, "Game", configSystem.WindowResolution.x, configSystem.WindowResolution.y);
        GameSystem::Get().StartUp(vulkanWindow);

        // imGuiRenderer = ImGui_StartUp(renderer);
        while (!vulkanWindow->WindowShouldClose(vulkanWindow))
        {
            const float frameTime = deltaTime.GetFrameTime();
            vulkanWindow->PollEventHandler(vulkanWindow);

          // GameSystem::Get().Update(frameTime);
            GameSystem::Get().DebugUpdate(frameTime);
            GameSystem::Get().Draw(frameTime);
            deltaTime.EndFrameTime();
        }
        vkDeviceWaitIdle(renderer.Device);
        GameSystem::Get().Destroy();
        vulkanWindow->DestroyWindow(vulkanWindow);
    }
    catch (const VulkanError& e)
    {
        fprintf(stderr, "%s\n", e.what());
        return -1;
    }
    catch (const std::exception& e)
    {
        fprintf(stderr, "STD EXCEPTION: %s\n", e.what());
        return -1;
    }
    return 0;
}

#ifdef __ANDROID__
#include <android/native_activity.h>
#include <android/asset_manager.h>
#include <android_native_app_glue.h>
#include <android/log.h>
#include <android/input.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_android.h>

// Global pointer so handle_cmd can access the window
static GameEngineWindow* g_vulkanWindow = nullptr;

void handle_cmd(android_app* app, int32_t cmd)
{
    switch (cmd)
    {
        case APP_CMD_INIT_WINDOW:
            if (app->window && g_vulkanWindow)
            {
                int32_t w = ANativeWindow_getWidth(app->window);
                int32_t h = ANativeWindow_getHeight(app->window);
                g_vulkanWindow->Width = w;
                g_vulkanWindow->Height = h;
                g_vulkanWindow->FrameBufferResized = true;
                __android_log_print(ANDROID_LOG_INFO, "VulkanEngine", "Window initialized: %dx%d", w, h);
            }
            break;

        case APP_CMD_TERM_WINDOW:
            __android_log_print(ANDROID_LOG_INFO, "VulkanEngine", "Window terminated");
            break;
    }
}

// Optional input handler (you can leave empty for now)
int32_t handle_input(android_app* app, AInputEvent* event)
{
    return 0; // 0 = not consumed
}

void android_main(struct android_app* app)
{
    app->userData = nullptr;
    app->onAppCmd = handle_cmd;
    app->onInputEvent = handle_input;

    // Wait for window
    while (!app->window) {
        int events;
        android_poll_source* source;
        if (ALooper_pollOnce(-1, nullptr, &events, (void**)&source) >= 0) {
            if (source) source->process(app, source);
            if (app->destroyRequested) return;
        }
    }

    FrameTimer deltaTime = FrameTimer();
    __android_log_print(ANDROID_LOG_INFO, "VulkanEngine", "android_main: Window ready");

    // Load assets
    fileSystem.LoadAndroidAssetManager(app->activity->assetManager);

    int32_t width  = ANativeWindow_getWidth(app->window);
    int32_t height = ANativeWindow_getHeight(app->window);
    __android_log_print(ANDROID_LOG_INFO, "VulkanEngine", "WINDOW READY: %dx%d", width, height);

    // Create window and initialize engine
    g_vulkanWindow = new GameEngineWindow();
    g_vulkanWindow->CreateGraphicsWindow(g_vulkanWindow, "Vulkan Game Engine", width, height);
    g_vulkanWindow->WindowHandle = (void*)app->window;

    configSystem.LoadConfig("EngineConfig.json");
    __android_log_print(ANDROID_LOG_INFO, "VulkanEngine", "ENGINE FULLY INITIALIZED — STARTING MAIN LOOP");

    // Start game system with the ANativeWindow*
    GameSystem::Get().StartUp(g_vulkanWindow->WindowHandle);

    // Main loop
    while (true) {
        int events;
        android_poll_source* source;

        // Non-blocking poll
        while (ALooper_pollOnce(0, nullptr, &events, (void**)&source) >= 0) {
            if (source) source->process(app, source);
            if (app->destroyRequested) {
                GameSystem::Get().Destroy();
                delete g_vulkanWindow;
                return;
            }
        }

        if (!app->window) continue;

        float frameTime = deltaTime.GetFrameTime();
        g_vulkanWindow->PollEventHandler(g_vulkanWindow);

        GameSystem::Get().Update(g_vulkanWindow->WindowHandle, frameTime);
        GameSystem::Get().DebugUpdate(frameTime);
        GameSystem::Get().Draw(frameTime);
        deltaTime.EndFrameTime();
    }
}
#endif