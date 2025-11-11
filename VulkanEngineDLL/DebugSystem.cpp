#include "DebugSystem.h"
#include <windows.h>
#include <direct.h>
#include <iostream>

DebugSystem debugSystem = DebugSystem();

DebugSystem::DebugSystem()
{
}

DebugSystem::~DebugSystem()
{
}

bool DebugSystem::TryLoadRenderDocAPI()
{
    HMODULE rd = GetModuleHandleA("renderdoc.dll");
    if (!rd)
    {
        UsingRenderDoc = false;
        return UsingRenderDoc;
    }

    auto GetAPI = (pRENDERDOC_GetAPI)GetProcAddress(rd, "RENDERDOC_GetAPI");
    if (!GetAPI)
    {
        UsingRenderDoc = false;
        return UsingRenderDoc;
    }

    RENDERDOC_API_1_6_0* api = nullptr;
    if (GetAPI(eRENDERDOC_API_Version_1_6_0, (void**)&api) == 1)
    {
        RenderDocAPI = api;
        UsingRenderDoc = true;
        return UsingRenderDoc;
    }
    UsingRenderDoc = false;
    return UsingRenderDoc;
}

void DebugSystem::SetRootDirectory(const String& engineRoot)
{
    if (_chdir(engineRoot.c_str()) != 0)
    {
        std::cerr << "Failed to set CWD to: " << engineRoot << std::endl;
        return;
    }

    char cwd[MAX_PATH];
    if (_getcwd(cwd, MAX_PATH))
    {
        std::cout << "C++ CWD SET TO: " << cwd << std::endl;
    }
}

bool DebugSystem::IsRenderDocInjected()
{
    if (TryLoadRenderDocAPI())
    {
        return UsingRenderDoc;
    }

    if (GetModuleHandleA("renderdoc.dll") != nullptr)
    {
        UsingRenderDoc = true;
        return UsingRenderDoc;
    }

    UsingRenderDoc = false;
    RenderDocAPI = nullptr;
    return UsingRenderDoc;
}

void Debug_SetRootDirectory(const char* engineRoot)
{
    debugSystem.SetRootDirectory(String(engineRoot));
}

bool Debug_IsRenderDocInjected()
{
    return debugSystem.IsRenderDocInjected();
}