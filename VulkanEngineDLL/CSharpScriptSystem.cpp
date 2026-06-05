#include "CSharpScriptSystem.h"
#include "DLLSystem.h"
#include "GameObjectSystem.h"
#include "EngineConfigSystem.h"

CSharpScriptSystem& cSharpScriptSystem = CSharpScriptSystem::Get();
bool CSharpScriptSystem::CSharpScriptSystemInitialized = false;

bool CSharpScriptSystem::Initialize()
{
        if (CSharpScriptSystemInitialized) return true;
    
        bool success = dllSystem.InitializeDLLRuntime(configSystem.GameScriptLibraryDLL);
        if (success) CSharpScriptSystemInitialized = true;
        return success;
}

GameObjectBehavior CSharpScriptSystem::LoadGameObjectScript(const String& assemblyPath, const String& typeNameString)
{
    GameObjectBehavior gameObjectBehavior;
    dllSystem.GetDLLFunctionPtr(assemblyPath, typeNameString, L"Create" , (void**)&gameObjectBehavior.CreateObject);
    dllSystem.GetDLLFunctionPtr(assemblyPath, typeNameString, L"StartUp", (void**)&gameObjectBehavior.Startup);
    dllSystem.GetDLLFunctionPtr(assemblyPath, typeNameString, L"Update" , (void**)&gameObjectBehavior.Update);
    dllSystem.GetDLLFunctionPtr(assemblyPath, typeNameString, L"Destroy", (void**)&gameObjectBehavior.Destroy);
    return gameObjectBehavior;
}

