#pragma once
#include "Platform.h"
#include "GameObjectSystem.h"
#include <string>
#include <filesystem>
#include <cstdint>

#include "enum.h"
#include "nethost.h"
#include "hostfxr.h"
#include "coreclr_delegates.h"

using string_t = std::basic_string<char_t>;

class CSharpScriptSystem
{
public:
    static CSharpScriptSystem& Get();

private:
    CSharpScriptSystem() = default;
    ~CSharpScriptSystem() = default;
    CSharpScriptSystem(const CSharpScriptSystem&) = delete;
    CSharpScriptSystem& operator=(const CSharpScriptSystem&) = delete;
    CSharpScriptSystem(CSharpScriptSystem&&) = delete;
    CSharpScriptSystem& operator=(CSharpScriptSystem&&) = delete;

    static bool CSharpScriptSystemInitialized;

    hostfxr_initialize_for_runtime_config_fn  m_initFn = nullptr;
    hostfxr_get_runtime_delegate_fn           m_getDelegateFn = nullptr;
    hostfxr_close_fn                          m_closeFn = nullptr;

    load_assembly_and_get_function_pointer_fn m_loadAssemblyAndGetFnPtr = nullptr;

    std::wstring ToWString(const std::string& str);

    bool LoadHostFxr();
    bool InitializeRuntime(const string_t& runtimeConfigPath, const string_t& assemblyPath);

public:
    DLL_EXPORT bool Initialize(const string_t& runtimeConfigPath, const string_t& assemblyPath);
    DLL_EXPORT GameObjectBehavior LoadGameObjectScript(const String& assemblyPath, const String& typeNameString);
};
extern DLL_EXPORT CSharpScriptSystem& cSharpScriptSystem;
inline CSharpScriptSystem& CSharpScriptSystem::Get()
{
    static CSharpScriptSystem instance;
    return instance;
}