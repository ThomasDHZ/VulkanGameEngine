#pragma once
#include <vulkan//vulkan_core.h>
#include "Macro.h"
#include "Typedef.h"

#if defined(_WIN32)
#define PLATFORM_WINDOWS
#include <windows.h>
#include <Vulkan/vulkan_win32.h>
#include "DLL.h"
#define SLEEP(ms) Sleep(ms)
inline void GenerateGUID(GUID& guid) { CoCreateGuid(&guid); }

#elif defined(__linux__) && !defined(__ANDROID__)
#define PLATFORM_LINUX
#include <unistd.h>
#include <uuid/uuid.h>
#define DLL_EXPORT extern "C" __attribute__((visibility("default")))
#define SLEEP(ms) usleep((ms) * 1000)
inline void GenerateGUID(uuid_t guid) { uuid_generate(guid); }

#elif defined(__ANDROID__)
#define PLATFORM_ANDROID
#include <unistd.h>
#include <random>
#define DLL_EXPORT extern "C" __attribute__((visibility("default")))
#define SLEEP(ms) usleep((ms) * 1000)
inline void GenerateGUID(uint8_t* guid) {
    std::random_device rd; std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    for (int i = 0; i < 16; ++i) guid[i] = dis(gen);
}

#elif defined(__APPLE__)
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
#define PLATFORM_IOS
#else
#define PLATFORM_MACOS
#endif
#include <unistd.h>
#include <uuid/uuid.h>
#define DLL_EXPORT extern "C"
#define SLEEP(ms) usleep((ms) * 1000)
inline void GenerateGUID(uuid_t guid) { uuid_generate(guid); }
#endif

#ifdef __cplusplus
extern "C" {
#endif
    DLL_EXPORT const char* Renderer_GetError(VkResult result);
#ifdef __cplusplus
}
#endif

#define VULKAN_RESULT(call) { \
    VkResult result = (call); \
    if (result != VK_SUCCESS) { \
        fprintf(stderr, "Error in %s at %s:%d (%s): %s\n", \
                #call, __FILE__, __LINE__, __func__, Renderer_GetError(result)); \
    } \
}

#define SPV_VULKAN_RESULT(call) { \
    SpvReflectResult result = (call); \
    if (result != SPV_REFLECT_RESULT_SUCCESS) { \
        fprintf(stderr, "Error in %s at %s:%d (%s): %s\n", \
                #call, __FILE__, __LINE__, __func__, result); \
    } \
}