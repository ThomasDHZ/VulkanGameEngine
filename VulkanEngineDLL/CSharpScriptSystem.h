#pragma once

#include <Platform.h>
#include "DLLSystem.h"
#include "GameObjectSystem.h"
#include <string>
#include <filesystem>
#include <cstdint>

#include "enum.h"
class CSharpScriptSystem
{
public:
    static CSharpScriptSystem& Get();
    static bool CSharpScriptSystemInitialized;

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
extern  CSharpScriptSystem& cSharpScriptSystem;
inline CSharpScriptSystem& CSharpScriptSystem::Get()
{
    static CSharpScriptSystem instance;
    return instance;
}