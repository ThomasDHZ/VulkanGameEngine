#pragma once
#include <stdlib.h>

#if defined(_WIN32)
#ifdef VULKAN_ENGINE_S
#define  __declspec(dllexport)
#else
#define  __declspec(dllimport)
#endif
#elif defined(__linux__) && !defined(__ANDROID__)
#define  __attribute__((visibility("default")))
#elif defined(__ANDROID__)
#define  __attribute__((visibility("default")))
#elif defined(__APPLE__)
#define  __attribute__((visibility("default")))
#endif