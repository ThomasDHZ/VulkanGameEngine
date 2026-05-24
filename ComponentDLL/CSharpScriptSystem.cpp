#include "pch.h"
#include "CSharpScriptSystem.h"

#include <windows.h>
#include <iostream>

CSharpScriptSystem* CSharpScriptSystem::s_instance = nullptr;
bool CSharpScriptSystem::s_initialized = false;

CSharpScriptSystem& CSharpScriptSystem::GetInstance()
{
    if (!s_instance)
        s_instance = new CSharpScriptSystem();
    return *s_instance;
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

bool CSharpScriptSystem::InitializeRuntime(const string_t& runtimeConfigPath, const string_t& assemblyPath)
{
    if (!fs::exists(runtimeConfigPath))
    {
        std::cerr << "[GameSystem] ❌ runtimeconfig.json NOT FOUND!" << std::endl;
    }

    if (!fs::exists(assemblyPath))
    {
        std::cerr << "[GameSystem] ❌ DLL NOT FOUND!" << std::endl;
    }

    if (!LoadHostFxr())
    {
        return false;
    }

    hostfxr_handle context = nullptr;
    int rc = m_initFn(runtimeConfigPath.c_str(), nullptr, &context);
    if (rc != 0 || !context)
    {
        std::cerr << "[CSharpScriptSystem] ❌ hostfxr_initialize failed with code: " << rc << std::endl;
        return false;
    }

    rc = m_getDelegateFn(context, hdt_load_assembly_and_get_function_pointer, (void**)&m_loadAssemblyAndGetFnPtr);
    if (rc != 0 || !m_loadAssemblyAndGetFnPtr)
    {
        std::cerr << "[CSharpScriptSystem] ❌ Failed to get load_assembly delegate. Code: " << rc << std::endl;
        return false;
    }
    m_closeFn(context);

    const char_t* typeName = L"GameScriptLibraryDLL.Player, GameScriptLibraryDLL";
    auto getFn = [&](const char_t* method, void** outFn) -> bool 
        {
            int rc = m_loadAssemblyAndGetFnPtr(assemblyPath.c_str(), typeName, method, UNMANAGEDCALLERSONLY_METHOD, nullptr, outFn);
            if (rc != 0)
            {
                std::wcerr << L"[CSharpScriptSystem] FAILED '" << method << L"' → 0x" << std::hex << rc << std::dec << std::endl;
                return false;
            }
            return true;
        };


    auto& scriptSys = CSharpScriptSystem::GetInstance();

    PlayerCreateFn  create = scriptSys.GetCreateFn();
    PlayerStartUpFn startup = scriptSys.GetStartUpFn();
    PlayerUpdateFn  update = scriptSys.GetUpdateFn();
    PlayerDestroyFn destroy = scriptSys.GetDestroyFn();

    const char_t* playerType = L"Player";
    intptr_t handle = create();
    startup(handle);
    update(handle, 1.4f);
    update(handle, 1.7);

    return true;
}

bool CSharpScriptSystem::Initialize(const string_t& runtimeConfigPath, const string_t& assemblyPath)
{
    if (s_initialized) return true;

    bool success = GetInstance().InitializeRuntime(runtimeConfigPath, assemblyPath);
    if (success)
        s_initialized = true;

    return success;
}

ManagedPlayer::ManagedPlayer(const char_t* typeName)
{
    auto& runtime = CSharpScriptSystem::GetInstance();
    PlayerCreateFn createFn = runtime.GetCreateFn();

    if (createFn)
    {
        m_handle = createFn();
        if (m_handle != 0)
            std::cout << "[C++] Player created (handle = " << m_handle << ")" << std::endl;
        else
            std::cerr << "[C++] Failed to create C# Player instance" << std::endl;
    }
    else
    {
        std::cerr << "[C++] Runtime not initialized before creating ManagedPlayer!" << std::endl;
    }
}

ManagedPlayer::~ManagedPlayer()
{
    if (m_handle != 0)
    {
        auto& runtime = CSharpScriptSystem::GetInstance();
        PlayerDestroyFn destroyFn = runtime.GetDestroyFn();
        if (destroyFn)
            destroyFn(m_handle);

        m_handle = 0;
    }
}

ManagedPlayer::ManagedPlayer(ManagedPlayer&& other) noexcept
    : m_handle(other.m_handle)
{
    other.m_handle = 0;
}

ManagedPlayer& ManagedPlayer::operator=(ManagedPlayer&& other) noexcept
{
    if (this != &other)
    {
        if (m_handle != 0)
        {
            auto& runtime = CSharpScriptSystem::GetInstance();
            if (PlayerDestroyFn destroyFn = runtime.GetDestroyFn())
                destroyFn(m_handle);
        }

        m_handle = other.m_handle;
        other.m_handle = 0;
    }
    return *this;
}

void ManagedPlayer::StartUp(intptr_t entity, intptr_t startPos, intptr_t rotation)
{
    if (m_handle == 0) return;
    auto& runtime = CSharpScriptSystem::GetInstance();
    if (PlayerStartUpFn fn = runtime.GetStartUpFn())
        fn(m_handle);
}

void ManagedPlayer::Update(float dt)
{
    if (m_handle == 0) return;
    auto& runtime = CSharpScriptSystem::GetInstance();
    if (PlayerUpdateFn fn = runtime.GetUpdateFn())
        fn(m_handle, dt);
}