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
//     void StartUp();
//     void Update(float deltaTime);
//     void ShutDown();
//
//     entt::entity CreateEntityFromScript(const std::string& scriptPath, const std::string& entityName);
//     sol::state& GetLuaState() { return lua; }
//};
//extern  LuaScriptingSystem& luaScriptingSystem;
//inline LuaScriptingSystem& LuaScriptingSystem::Get()
//{
//    static LuaScriptingSystem instance;
//    return instance;
//}
//
//
