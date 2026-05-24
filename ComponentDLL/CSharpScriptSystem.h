#pragma once

#include <string>
#include <filesystem>
#include <cstdint>

#include "nethost.h"
#include "hostfxr.h"
#include "coreclr_delegates.h"

namespace fs = std::filesystem;
using string_t = std::basic_string<char_t>;

using PlayerCreateFn = intptr_t(*)();
using PlayerStartUpFn = void(*)(intptr_t instance);
using PlayerUpdateFn = void(*)(intptr_t instance, float deltaTime);
using PlayerDestroyFn = void(*)(intptr_t instance);

class CSharpScriptSystem
{
private:
    static CSharpScriptSystem* s_instance;
    static bool s_initialized;

    hostfxr_initialize_for_runtime_config_fn  m_initFn = nullptr;
    hostfxr_get_runtime_delegate_fn           m_getDelegateFn = nullptr;
    hostfxr_close_fn                          m_closeFn = nullptr;

    load_assembly_and_get_function_pointer_fn m_loadAssemblyAndGetFnPtr = nullptr;

    PlayerCreateFn  m_createFn = nullptr;
    PlayerStartUpFn m_startupFn = nullptr;
    PlayerUpdateFn  m_updateFn = nullptr;
    PlayerDestroyFn m_destroyFn = nullptr;

    bool LoadHostFxr();
    bool InitializeRuntime(const string_t& runtimeConfigPath, const string_t& assemblyPath);

    CSharpScriptSystem() = default;
    CSharpScriptSystem(const CSharpScriptSystem&) = delete;
    CSharpScriptSystem& operator=(const CSharpScriptSystem&) = delete;

public:
   DLL_EXPORT static CSharpScriptSystem& GetInstance();

   DLL_EXPORT static bool Initialize(const string_t& runtimeConfigPath, const string_t& assemblyPath);

    PlayerCreateFn  GetCreateFn()  const { return m_createFn; }
     PlayerStartUpFn GetStartUpFn() const { return m_startupFn; }
    PlayerUpdateFn  GetUpdateFn()  const { return m_updateFn; }
     PlayerDestroyFn GetDestroyFn() const { return m_destroyFn; }
};

class ManagedPlayer
{
private:
    intptr_t m_handle = 0;

public:
    explicit ManagedPlayer(const char_t* typeName = L"Player");
    ~ManagedPlayer();

    ManagedPlayer(const ManagedPlayer&) = delete;
    ManagedPlayer& operator=(const ManagedPlayer&) = delete;

    ManagedPlayer(ManagedPlayer&& other) noexcept;
    ManagedPlayer& operator=(ManagedPlayer&& other) noexcept;

    DLL_EXPORT void StartUp(intptr_t entity = 0, intptr_t startPos = 0, intptr_t rotation = 0);
    DLL_EXPORT void Update(float dt);
};