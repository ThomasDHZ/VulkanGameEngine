#pragma once
#include "Platform.h"
#if defined(_WIN32)
#include <C:\Program Files\RenderDoc\renderdoc_app.h>
#endif

class DebugSystem
{
public:
	static DebugSystem& Get();

private:
	DebugSystem() = default;
	~DebugSystem() = default;
	DebugSystem(const DebugSystem&) = delete;
	DebugSystem& operator=(const DebugSystem&) = delete;
	DebugSystem(DebugSystem&&) = delete;
	DebugSystem& operator=(DebugSystem&&) = delete;

#if defined(_WIN32)
		bool TryLoadRenderDocAPI();
#endif

	public:
#if defined(_WIN32)
		RENDERDOC_API_1_6_0* RenderDocAPI = nullptr;
		bool UsingRenderDoc = false;
		DLL_EXPORT bool IsRenderDocInjected();
#endif
		DLL_EXPORT void SetRootDirectory(const String& engineRoot);
		DLL_EXPORT VkResult                 DumpVMAStats() const;
};
extern DLL_EXPORT DebugSystem& debugSystem;
inline DebugSystem& DebugSystem::Get()
{
	static DebugSystem instance;
	return instance;
}


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