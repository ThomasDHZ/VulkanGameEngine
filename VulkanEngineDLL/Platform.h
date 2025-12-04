#pragma once

#ifdef PLATFORM_ANDROID
#define GLFW_INCLUDE_NONE
#endif

#if defined(__linux__) && !defined(__ANDROID__)
    #define VK_ENABLE_BETA_EXTENSIONS
#endif
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan.h>
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
#include <mutex>

#include "DLL.h"
#include "Typedef.h"
#include "VulkanError.h"

#if defined(_WIN32)
    #define PLATFORM_WINDOWS
    #include <windows.h>
    #include <direct.h>
    #include <objbase.h>
    #include <combaseapi.h>
    #include <vulkan/vulkan_win32.h>
    #define SLEEP(ms) Sleep(ms)
    inline void GenerateGUID(GUID& guid) { CoCreateGuid(&guid); }

#elif defined(__linux__) && !defined(__ANDROID__)
    #define PLATFORM_LINUX
    #include <unistd.h>
    #include <uuid/uuid.h>
    #include <cctype>
    #include <cstdlib>
    #define SLEEP(ms) usleep((ms) * 1000)
    #if defined(__clang__) && defined(__linux__)
        #pragma clang diagnostic ignored "-Wfloat-conversion"
    #endif
    inline void GenerateGUID(uuid_t guid) { uuid_generate(guid); }

#elif defined(__ANDROID__)
    #define PLATFORM_ANDROID
    #include <vulkan/vulkan_android.h>
    #include <unistd.h>
    #include <random>
    #define SLEEP(ms) usleep((ms) * 1000)
    inline void GenerateGUID(uint8_t* guid) 
    {
        std::random_device rd; std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        for (int i = 0; i < 16; ++i) guid[i] = dis(gen);
    }

#elif defined(__APPLE__)
    #include <TargetConditionals.h>
#include <CoreFoundation/CoreFoundation.h>

#include <Endian.h>
    #if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
    #define PLATFORM_IOS
    #else
    #define PLATFORM_MACOS
    #endif
    #include <unistd.h>
    #include <uuid/uuid.h>
    #define SLEEP(ms) usleep((ms) * 1000)
    inline void GenerateGUID(uuid_t guid) { uuid_generate(guid); }
#endif

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define IO_READ_CHUNK_SIZE 2097152
#define IO_READ_ERROR_GENERAL "Error reading file: %s. error: %d\n"
#define IO_READ_ERROR_MEMORY "Not enough free memory to read file: %s\n"
#define ERROR_EXIT(...) { fprintf(stderr, __VA_ARGS__); SLEEP(100); exit(1); }
#define ERROR_RETURN(R, ...) { fprintf(stderr, __VA_ARGS__); SLEEP(100); return (R); }

#if defined(PLATFORM_WINDOWS) && defined(NOMINMAX)
#elif defined(PLATFORM_WINDOWS)
#undef max
#undef min
#define NOMINMAX
#endif