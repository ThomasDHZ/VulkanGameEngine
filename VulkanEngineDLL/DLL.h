#pragma once
#include <stdlib.h>

#if defined(_WIN32)
	#ifdef VulkanEngineDLL_EXPORTS
		#define DLL_EXPORT __declspec(dllexport)
	#else
		#define DLL_EXPORT __declspec(dllimport)
	#endif
#elif defined(__linux__) && !defined(__ANDROID__)
	#define DLL_EXPORT __attribute__((visibility("default")))
#elif defined(__ANDROID__)
	#define DLL_EXPORT __attribute__((visibility("default")))
#elif defined(__APPLE__)
	#define DLL_EXPORT __attribute__((visibility("default")))
#endif