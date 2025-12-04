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
        gameSystem.StartUp(vulkanWindow);

        // imGuiRenderer = ImGui_StartUp(renderer);
        while (!vulkanWindow->WindowShouldClose(vulkanWindow))
        {
            const float frameTime = deltaTime.GetFrameTime();
            vulkanWindow->PollEventHandler(vulkanWindow);

            gameSystem.Update(frameTime);
            gameSystem.DebugUpdate(frameTime);
            gameSystem.Draw(frameTime);
            deltaTime.EndFrameTime();
        }
        vkDeviceWaitIdle(renderer.Device);
        gameSystem.Destroy();
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
#include <android_native_app_glue.h>
#include <android/log.h>

void handle_cmd(android_app* app, int32_t cmd)
{
    switch (cmd)
    {
        case APP_CMD_INIT_WINDOW:
            if (app->window && vulkanWindow) {
                int32_t w = ANativeWindow_getWidth(app->window);
                int32_t h = ANativeWindow_getHeight(app->window);
                vulkanWindow->Width = w;
                vulkanWindow->Height = h;
                vulkanWindow->FrameBufferResized = true;
                __android_log_print(ANDROID_LOG_INFO, "VulkanEngine", "Window created/resized: %dx%d", w, h);
            }
            break;
        case APP_CMD_TERM_WINDOW:
            if (vulkanWindow)
            {
                __android_log_print(ANDROID_LOG_INFO, "VulkanEngine", "Window terminated");
            }
            break;
    }
}

void android_main(struct android_app* app)
{
    fileSystem.LoadAndroidAssetManager(app->activity->assetManager);
    while (!app->window)
    {
        int events;
        struct android_poll_source* source;
        if (ALooper_pollOnce(-1, nullptr, &events, (void**)&source) >= 0)
        {
            if (source) source->process(app, source);
            if (app->destroyRequested) return;
        }
    }

    uint width = (uint)ANativeWindow_getWidth(app->window);
    uint height = (uint)ANativeWindow_getHeight(app->window);

    vulkanWindow = new GameEngineWindow();
    vulkanWindow->WindowHandle = (void*)app->window;
    vulkanWindow->CreateGraphicsWindow(vulkanWindow, "Vulkan Game Engine", width, height);
    main();
}
#endif