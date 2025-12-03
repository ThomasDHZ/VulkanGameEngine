#include "VulkanWindow.h"
#include <stdio.h>
#include <nlohmann/json.hpp>
#include <implot.h>
#include "SystemClock.h"
#include <iostream>
#include "FrameTimer.h"
#include "GameSystem.h"
#include "MaterialSystem.h"
#include "EngineConfigSystem.h"
#include <RigidBody.h>
#include "ImGuiRenderer.h"
#include <DebugSystem.h>


#ifdef __ANDROID__
#include <jni.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android_native_app_glue.h>

extern "C" {

    __attribute__((visibility("default"))) JNIEXPORT void JNICALL Java_com_example_vulkangameengine_MainActivity_vulkanMain(JNIEnv* env, jobject, jobject assetManager)
    {
        __android_log_print(ANDROID_LOG_INFO, "VulkanGameEngine", "vulkanMain() called — starting engine");
        AAssetManager* nativeAssetManager = AAssetManager_fromJava(env, assetManager);
		fileSystem.LoadAndroidAssetManager(nativeAssetManager);
        EngineMain();
    }

}

void EngineMain()
#else
    int main(int argc, char** argv)
#endif
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
 //   debugSystem.SetRootDirectory("");
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
            vulkanWindow->SwapBuffer(vulkanWindow);

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