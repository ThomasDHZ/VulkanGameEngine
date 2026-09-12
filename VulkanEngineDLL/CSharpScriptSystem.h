#pragma once
#include "DLL.h"
#include <Platform.h>
#include "DLLSystem.h"
#include "GameObjectSystem.h"
#include <string>
#include <filesystem>
#include <cstdint>

#include "enum.h"
class  ENGINE_DLL_EXPORT CSharpScriptSystem
{
public:
    static CSharpScriptSystem& Get();
   // static bool CSharpScriptSystemInitialized;

private:
    CSharpScriptSystem() = default;
    ~CSharpScriptSystem() = default;
    CSharpScriptSystem(const CSharpScriptSystem&) = delete;
    CSharpScriptSystem& operator=(const CSharpScriptSystem&) = delete;
    CSharpScriptSystem(CSharpScriptSystem&&) = delete;
    CSharpScriptSystem& operator=(CSharpScriptSystem&&) = delete;

public:
     bool Initialize();
     GameObjectBehavior LoadGameObjectScript(const String& assemblyPath, const String& typeNameString);
};
ENGINE_DLL_EXPORT extern  CSharpScriptSystem& cSharpScriptSystem;
inline CSharpScriptSystem& CSharpScriptSystem::Get()
{
    static CSharpScriptSystem instance;
    return instance;
}