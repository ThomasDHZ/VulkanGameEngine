#include "CSharpScriptSystem.h"
#include "GameObjectSystem.h"
#include <windows.h>
#include <iostream>

CSharpScriptSystem& cSharpScriptSystem = CSharpScriptSystem::Get();
bool CSharpScriptSystem::CSharpScriptSystemInitialized = false;

bool CSharpScriptSystem::InitializeRuntime(const string_t& runtimeConfigPath, const string_t& assemblyPath)
{
    if (!LoadHostFxr()) return false;
    if (!std::filesystem::exists(runtimeConfigPath)) std::cerr << "[CSharpScriptSystem] runtimeconfig.json NOT FOUND!" << std::endl;
    if (!std::filesystem::exists(assemblyPath)) std::cerr << "[GameSystem] DLL NOT FOUND!" << std::endl;

    hostfxr_handle context = nullptr;
    int rc = m_initFn(runtimeConfigPath.c_str(), nullptr, &context);
    if (rc != 0 || !context)
    {
        std::cerr << "[CSharpScriptSystem] hostfxr_initialize failed with code: " << rc << std::endl;
        return false;
    }

    rc = m_getDelegateFn(context, hdt_load_assembly_and_get_function_pointer, (void**)&m_loadAssemblyAndGetFnPtr); 
    if (rc != 0 || !m_loadAssemblyAndGetFnPtr)
    {
        std::cerr << "[CSharpScriptSystem] Failed to get load_assembly delegate. Code: " << rc << std::endl;
        return false;
    }
    m_closeFn(context);
    return true;
}

bool CSharpScriptSystem::LoadHostFxr()
{
    char_t hostfxrPath[MAX_PATH] = {};
    size_t bufferSize = sizeof(hostfxrPath) / sizeof(char_t);
    if (get_hostfxr_path(hostfxrPath, &bufferSize, nullptr) != 0)
    {
        std::cerr << "[CSharpScriptSystem] Failed to locate hostfxr.dll" << std::endl;
        return false;
    }

    HMODULE hHostFxr = LoadLibraryW(hostfxrPath);
    if (!hHostFxr)
    {
        std::cerr << "[CSharpScriptSystem] Failed to load hostfxr.dll" << std::endl;
        return false;
    }

    m_initFn = (hostfxr_initialize_for_runtime_config_fn)GetProcAddress(hHostFxr, "hostfxr_initialize_for_runtime_config");
    m_getDelegateFn = (hostfxr_get_runtime_delegate_fn)GetProcAddress(hHostFxr, "hostfxr_get_runtime_delegate");
    m_closeFn = (hostfxr_close_fn)GetProcAddress(hHostFxr, "hostfxr_close");

    return m_initFn && m_getDelegateFn && m_closeFn;
}

bool CSharpScriptSystem::Initialize(const string_t& runtimeConfigPath, const string_t& assemblyPath)
{
    if (CSharpScriptSystemInitialized) return true;

    bool success = InitializeRuntime(runtimeConfigPath, assemblyPath);
    if (success) CSharpScriptSystemInitialized = true;
    return success;
}

GameObjectBehavior CSharpScriptSystem::LoadGameObjectScript(const String& assemblyPath, const String& typeNameString)
{
    std::wstring assemblyW = ToWString(assemblyPath);
    std::wstring typeNameW = ToWString(typeNameString);
    auto GetDLLFunction = [&](const char_t* method, void** outFn) -> bool
        {
            int rc = m_loadAssemblyAndGetFnPtr(assemblyW.c_str(), typeNameW.c_str(), method, UNMANAGEDCALLERSONLY_METHOD, nullptr, outFn);
            if (rc != 0)
            {
                std::wcerr << L"[CSharpScriptSystem] FAILED '" << method << L"' → 0x" << std::hex << rc << std::dec << std::endl;
                return false;
            }
            return true;
        };

    GameObjectBehavior gameObjectBehavior;
    GetDLLFunction(L"Create" , (void**)&gameObjectBehavior.CreateObject);
    GetDLLFunction(L"StartUp", (void**)&gameObjectBehavior.Startup);
    GetDLLFunction(L"Update" , (void**)&gameObjectBehavior.Update);
    GetDLLFunction(L"Destroy", (void**)&gameObjectBehavior.Destroy);
    return gameObjectBehavior;
}

std::wstring CSharpScriptSystem::ToWString(const std::string& str)
{
    if (str.empty()) return L"";

    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    std::wstring wstr(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wstr.data(), size);

    return wstr;
}