#include "LuaScriptingSystem.h"
#include "Transform2DComponent.h"

LuaScriptingSystem& luaScriptingSystem = LuaScriptingSystem::Get();

void LuaScriptingSystem::BindCoreAPI()
{
    //lua.set_function("print", [](const std::string& msg) {
    //    std::cout << "[Lua] " << msg << std::endl;
    //    });
}

void LuaScriptingSystem::BindEnTTAPI()
{
    //lua.new_usertype<entt::entity>("Entity",
    //    sol::meta_function::construct, sol::no_constructor,
    //    "GetID", [](entt::entity e) { return static_cast<uint32_t>(e); }
    //);

    // Bind Transform (example)
    //lua.new_usertype<Transform2DComponent>("Transform",
    //    "Position", sol::property(&Transform2DComponent::GetPosition, &Transform2DComponent::SetPosition),
    //    "Move", &Transform2DComponent::Move
    //);

    //// Helper to get components
    //lua.set_function("GetTransform", [&](entt::entity e) -> TransformComponent& {
    //    return registry.get<TransformComponent>(e);
    //    });
}

void LuaScriptingSystem::BindInputAPI()
{
}

void LuaScriptingSystem::StartUp()
{
    //lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::table);

    //BindCoreAPI();
    //BindEnTTAPI();

    //std::cout << "[Lua] Scripting system initialized.\n";
}

void LuaScriptingSystem::Update(float deltaTime)
{
}

void LuaScriptingSystem::ShutDown()
{
}

entt::entity LuaScriptingSystem::CreateEntityFromScript(const String& scriptPath, const String& entityName)
{
    return entt::entity();
}
