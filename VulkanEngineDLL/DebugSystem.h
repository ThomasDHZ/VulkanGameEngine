#pragma once
#include "Platform.h"
#if defined(_WIN32)
#include <C:\Program Files\RenderDoc\renderdoc_app.h>
#endif

class DebugSystem
{
	private:
#if defined(_WIN32)
		bool TryLoadRenderDocAPI();
#endif

	public:
		DebugSystem();
		~DebugSystem();

#if defined(_WIN32)
		RENDERDOC_API_1_6_0* RenderDocAPI = nullptr;
		bool UsingRenderDoc = false;
		DLL_EXPORT bool IsRenderDocInjected();
#endif

		DLL_EXPORT void SetRootDirectory(const String& engineRoot);
};
extern DLL_EXPORT DebugSystem debugSystem;


#ifdef __cplusplus
extern "C" {
#endif
	DLL_EXPORT void Debug_SetRootDirectory(const char* engineRoot);
#if defined(_WIN32)
	DLL_EXPORT bool	Debug_IsRenderDocInjected();
#endif
#ifdef __cplusplus
}
#endif