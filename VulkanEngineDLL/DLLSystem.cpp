#include "DLLSystem.h"
#include <minwindef.h>
#include <libloaderapi.h>
#include <iostream>
#include "FileSystem.h"

DLLSystem& dllSystem = DLLSystem::Get();

bool DLLSystem::LoadHostFxr()
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

bool DLLSystem::InitializeDLLRuntime(const String& assemblyPath)
{
    std::filesystem::path path = std::filesystem::path(assemblyPath);
    path.replace_extension(".runtimeconfig.json");
    string_t runtimeConfigPathT = ToStringT(path.string());

    if (m_runtimeInitialized) return true;
    if (!LoadHostFxr())
    {
        LoadHostFxr();
    }
    if (!std::filesystem::exists(path))
    {
        std::cerr << "[DLLSystem] runtimeconfig.json not found: " << path << "\n";
        return false;
    }

    hostfxr_handle context = nullptr;
    int rc = m_initFn(path.c_str(), nullptr, &context);
    if (rc != 0 || !context)
    {
        std::cerr << "[DLLSystem] hostfxr_initialize failed. Code: " << rc << "\n";
        return false;
    }

    rc = m_getDelegateFn(context, hdt_load_assembly_and_get_function_pointer, (void**)&m_loadAssemblyAndGetFnPtr);
    if (rc != 0 || !m_loadAssemblyAndGetFnPtr)
    {
        std::cerr << "[DLLSystem] Failed to get load_assembly delegate. Code: " << rc << "\n";
        return false;
    }
    m_closeFn(context);

    m_runtimeInitialized = true;
    std::cout << "[DLLSystem] .NET Runtime initialized successfully.\n";
    return true;
}

void DLLSystem::GetDLLFunctionPtr(const String& assemblyPath, const String& typeNameString, const char_t* functionName, void** outFunctionPtr)
{
    int rc = m_loadAssemblyAndGetFnPtr(ToWString(assemblyPath).c_str(), ToWString(typeNameString).c_str(), functionName, UNMANAGEDCALLERSONLY_METHOD, nullptr, outFunctionPtr);
    if (rc != 0)
    {
        std::wcerr << L"[DLLSystem] FAILED '" << assemblyPath.c_str() << " " << functionName << L"' → 0x" << std::hex << rc << std::dec << std::endl;
    }
}

std::wstring DLLSystem::ToWString(const std::string& str)
{
    if (str.empty()) return L"";

    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    std::wstring wstr(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wstr.data(), size);

    return wstr;
}

string_t DLLSystem::ToStringT(const std::string& str)
{
    if (str.empty()) return string_t();
#ifdef _WIN32
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    if (size <= 0) return L"";

    string_t result(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, result.data(), size);
    return result;
#else
    return str;
#endif
}
