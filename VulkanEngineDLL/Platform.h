// Platform.h  (only the part you need to change)

#pragma once
#include <vulkan/vulkan_core.h>
#include <spirv_reflect.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <assert.h>
#include <fstream>
#include <string>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <stb_image.h>
#include <mutex>
#include "Typedef.h"

#if defined(_WIN32)
    #define PLATFORM_WINDOWS
    #include <windows.h>
    #include <vulkan/vulkan_win32.h>
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
    DLL_EXPORT const char* Renderer_GetShaderReflectError(SpvReflectResult result);
#ifdef __cplusplus
}
#endif

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define IO_READ_CHUNK_SIZE 2097152
#define IO_READ_ERROR_GENERAL "Error reading file: %s. error: %d\n"
#define IO_READ_ERROR_MEMORY "Not enough free memory to read file: %s\n"
#define ERROR_EXIT(...) { fprintf(stderr, __VA_ARGS__); SLEEP(100); exit(1); }
#define ERROR_RETURN(R, ...) { fprintf(stderr, __VA_ARGS__); SLEEP(100); return (R); }
#define RENDERER_ERROR(msg) { \
    fprintf(stderr, "Error in %s:%d (%s): %s\n", __FILE__, __LINE__, __func__, msg); \
    SLEEP(100); \
}
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
                #call, __FILE__, __LINE__, __func__, Renderer_GetShaderReflectError(result)); \
    } \
}

#if defined(PLATFORM_WINDOWS) && defined(NOMINMAX)
#elif defined(PLATFORM_WINDOWS)
#undef max
#undef min
#define NOMINMAX
#endif