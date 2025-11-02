#pragma once
#include <vulkan/vulkan_core.h>
#include "DLL.h"
#include "Typedef.h"
#include <C:\Program Files\RenderDoc\renderdoc_app.h>

#ifdef __cplusplus
extern "C" {
#endif
	DLL_EXPORT void Debug_SetRootDirectory(const char* engineRoot);
	DLL_EXPORT bool	Debug_IsRenderDocInjected();
#ifdef __cplusplus
}
#endif
	bool Debug_TryLoadRenderDocAPI();

class DebugSystem
{
private:
public:
	RENDERDOC_API_1_6_0* RenderDocAPI = nullptr;
	bool UsingRenderDoc = false;

	DebugSystem()
	{

	}

	~DebugSystem()
	{

	}

	void SetRootDirectory(const String& engineRoot)
	{
		Debug_SetRootDirectory(engineRoot.c_str());
	}

	bool IsRenderDocInjected()
	{
		return Debug_IsRenderDocInjected();
	}
};
DLL_EXPORT DebugSystem debugSystem;

