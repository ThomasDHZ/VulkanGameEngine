#include "DebugSystem.h"
#include <windows.h>
#include <direct.h>
#include <iostream>

DebugSystem debugSystem = DebugSystem();

void Debug_SetRootDirectory(const char* engineRoot)
{
    if (_chdir(engineRoot) != 0)
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

bool Debug_TryLoadRenderDocAPI()
{
    HMODULE rd = GetModuleHandleA("renderdoc.dll"); 
    if (!rd) 
    { 
        debugSystem.UsingRenderDoc = false;
        return debugSystem.UsingRenderDoc;
    }

    auto GetAPI = (pRENDERDOC_GetAPI)GetProcAddress(rd, "RENDERDOC_GetAPI");
    if (!GetAPI)
    {
        debugSystem.UsingRenderDoc = false;
        return debugSystem.UsingRenderDoc;
    }

    RENDERDOC_API_1_6_0* api = nullptr;
    if (GetAPI(eRENDERDOC_API_Version_1_6_0, (void**)&api) == 1) 
    {
        debugSystem.RenderDocAPI = api;
        debugSystem.UsingRenderDoc = true;
        return debugSystem.UsingRenderDoc;
    }
    debugSystem.UsingRenderDoc = false;
    return debugSystem.UsingRenderDoc;
}

bool Debug_IsRenderDocInjected()
{
    if (Debug_TryLoadRenderDocAPI())
    {
        return debugSystem.UsingRenderDoc;
    }
    
    if (GetModuleHandleA("renderdoc.dll") != nullptr) 
    {
        debugSystem.UsingRenderDoc = true;
        return debugSystem.UsingRenderDoc;
    }

    debugSystem.UsingRenderDoc = false;
    debugSystem.RenderDocAPI = nullptr;
}
