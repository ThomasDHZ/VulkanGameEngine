#pragma once
#include <stdlib.h>

#if defined(_WIN32)
#ifdef VULKAN_ENGINE_DLL_EXPORTS
#define ENGINE_DLL_EXPORT __declspec(dllexport)
#else
#define ENGINE_DLL_EXPORT __declspec(dllimport)
#endif
#else
#define ENGINE_DLL_EXPORT __attribute__((visibility("default")))
#endif
