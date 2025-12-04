#include "DebugSystem.h"

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#include <io.h>
#else
#include <limits.h>
#include <dlfcn.h>
#include <unistd.h>
#include <libgen.h>
#endif

DebugSystem debugSystem = DebugSystem();

DebugSystem::DebugSystem()
{
}

DebugSystem::~DebugSystem()
{
}

#if defined(_WIN32)
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

bool Debug_IsRenderDocInjected()
{
    return debugSystem.IsRenderDocInjected();
}
#endif

void DebugSystem::SetRootDirectory(const String& engineRoot)
{
#ifdef _WIN32
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
#else
    if (chdir(engineRoot.c_str()) != 0)
    {
        std::cerr << "Failed to set CWD to: " << engineRoot << std::endl;
        return;
    }

    char cwd[PATH_MAX] = {};
    if (getcwd(cwd, PATH_MAX))
#endif
    {
        std::cout << "C++ CWD SET TO: " << cwd << std::endl;
    }
}

void Debug_SetRootDirectory(const char* engineRoot)
{
    debugSystem.SetRootDirectory(String(engineRoot));
}
