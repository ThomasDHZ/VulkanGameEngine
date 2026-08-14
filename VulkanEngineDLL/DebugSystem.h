#pragma once

#include <Platform.h>
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
		 bool IsRenderDocInjected();
#endif
		 void SetRootDirectory(const String& engineRoot);
		 VkResult                 DumpVMAStats() const;
};
extern  DebugSystem& debugSystem;
inline DebugSystem& DebugSystem::Get()
{
	static DebugSystem instance;
	return instance;
}


#ifdef __cplusplus
extern "C" {
#endif
	 void Debug_SetRootDirectory(const char* engineRoot);
#if defined(_WIN32)
	 bool	Debug_IsRenderDocInjected();
#endif
#ifdef __cplusplus
}
#endif