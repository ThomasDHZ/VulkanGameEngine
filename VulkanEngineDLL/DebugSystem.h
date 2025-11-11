#pragma once
#include "Platform.h"
#include <C:\Program Files\RenderDoc\renderdoc_app.h>


class DebugSystem
{
	private:
		bool TryLoadRenderDocAPI();

	public:
		RENDERDOC_API_1_6_0* RenderDocAPI = nullptr;
		bool UsingRenderDoc = false;

		DebugSystem();
		~DebugSystem();

		DLL_EXPORT void SetRootDirectory(const String& engineRoot);
		DLL_EXPORT bool IsRenderDocInjected();
};
DLL_EXPORT DebugSystem debugSystem;


#ifdef __cplusplus
extern "C" {
#endif
	DLL_EXPORT void Debug_SetRootDirectory(const char* engineRoot);
	DLL_EXPORT bool	Debug_IsRenderDocInjected();
#ifdef __cplusplus
}
#endif