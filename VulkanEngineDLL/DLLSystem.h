#pragma once
#include "DLL.h"
#include "Typedef.h"
#include "nethost.h"
#include "hostfxr.h"
#include "coreclr_delegates.h"
#include <string>

using string_t = std::basic_string<char_t>;
class DLLSystem
{
    public:
        static DLLSystem& Get();

    private:
        DLLSystem() = default;
        ~DLLSystem() = default;
        DLLSystem(const DLLSystem&) = delete;
        DLLSystem& operator=(const DLLSystem&) = delete;
        DLLSystem(DLLSystem&&) = delete;
        DLLSystem& operator=(DLLSystem&&) = delete;

        bool m_runtimeInitialized = false;
        hostfxr_initialize_for_runtime_config_fn  m_initFn = nullptr;
        hostfxr_get_runtime_delegate_fn           m_getDelegateFn = nullptr;
        hostfxr_close_fn                          m_closeFn = nullptr;
        load_assembly_and_get_function_pointer_fn m_loadAssemblyAndGetFnPtr = nullptr;

        bool LoadHostFxr();
        std::wstring  ToWString(const std::string& str);
        string_t ToStringT(const std::string& str);

    public:
        DLL_EXPORT bool InitializeDLLRuntime(const String& assemblyPath);
        DLL_EXPORT void GetDLLFunctionPtr(const String& assemblyPath, const String& typeNameString, const char_t* functionName, void** outFunctionPtr);
};
extern DLL_EXPORT DLLSystem& dllSystem;
inline DLLSystem& DLLSystem::Get()
{
    static DLLSystem instance;
    return instance;
}