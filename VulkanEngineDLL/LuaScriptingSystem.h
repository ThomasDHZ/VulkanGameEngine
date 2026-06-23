//#pragma once
//#include <Platform.h>
//#include <sol/sol.hpp>
//
//struct LuaScriptComponent
//{
//    sol::table scriptTable;
//    std::string entityName;
//};
//
//class LuaScriptingSystem
//{
//public:
//    static LuaScriptingSystem& Get();
//
//private:
//    LuaScriptingSystem() = default;
//    ~LuaScriptingSystem() = default;
//    LuaScriptingSystem(const LuaScriptingSystem&) = delete;
//    LuaScriptingSystem& operator=(const LuaScriptingSystem&) = delete;
//    LuaScriptingSystem(LuaScriptingSystem&&) = delete;
//    LuaScriptingSystem& operator=(LuaScriptingSystem&&) = delete;
//
//    sol::state lua;
//
//    void BindCoreAPI();
//    void BindEnTTAPI();
//    void BindInputAPI();
//
//public:
//
//    DLL_EXPORT void StartUp();
//    DLL_EXPORT void Update(float deltaTime);
//    DLL_EXPORT void ShutDown();
//
//    DLL_EXPORT entt::entity CreateEntityFromScript(const std::string& scriptPath, const std::string& entityName);
//    DLL_EXPORT sol::state& GetLuaState() { return lua; }
//};
//extern DLL_EXPORT LuaScriptingSystem& luaScriptingSystem;
//inline LuaScriptingSystem& LuaScriptingSystem::Get()
//{
//    static LuaScriptingSystem instance;
//    return instance;
//}
//
//
