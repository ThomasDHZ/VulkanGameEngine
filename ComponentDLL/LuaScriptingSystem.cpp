#include "pch.h"
#include "LuaScriptingSystem.h"
#include "Transform2DComponent.h"
#include <entt/entt.hpp>
#include "LevelSystem.h"
#include <sol/sol.hpp>

LuaScriptingSystem& luaScriptingSystem = LuaScriptingSystem::Get();

void LuaScriptingSystem::BindCoreAPI()
{
    lua.set_function("print", [](const std::string& msg) {
        std::cout << "[Lua] " << msg << std::endl;
        });
}

void LuaScriptingSystem::BindEnTTAPI()
{
    lua.new_usertype<entt::entity>("Entity",
        sol::meta_function::construct, sol::no_constructor,  
        "GetID", [](entt::entity e) { return static_cast<uint32_t>(e); }
    );

    lua.new_usertype<Transform2DComponent>("Transform",
        sol::constructors<Transform2DComponent()>(),

        "Position", sol::property(
            [](const Transform2DComponent& t) -> glm::vec2 { return t.GameObjectPosition; },
            [](Transform2DComponent& t, const glm::vec2& v) { t.GameObjectPosition = v; t.Dirty = true; }
        ),

        "Rotation", sol::property(
            [](const Transform2DComponent& t) -> glm::vec2 { return t.GameObjectRotation; },
            [](Transform2DComponent& t, const glm::vec2& v) { t.GameObjectRotation = v; t.Dirty = true; }
        ),

        "Scale", sol::property(
            [](const Transform2DComponent& t) -> glm::vec2 { return t.GameObjectScale; },
            [](Transform2DComponent& t, const glm::vec2& v) { t.GameObjectScale = v; t.Dirty = true; }
        ),

        "Move", sol::overload(
            sol::resolve<void(float, float)>(&Transform2DComponent::Move),
            sol::resolve<void(const glm::vec2&)>(&Transform2DComponent::Move)
        ),

        "Rotate", sol::overload(
            sol::resolve<void(float, float)>(&Transform2DComponent::Rotate),
            sol::resolve<void(const glm::vec2&)>(&Transform2DComponent::Rotate)
        ),

        "SetPosition", sol::overload(
            sol::resolve<void(float, float)>(&Transform2DComponent::SetPosition),
            sol::resolve<void(const glm::vec2&)>(&Transform2DComponent::SetPosition)
        )
    );

    lua.set_function("GetTransform", [&](entt::entity e) -> Transform2DComponent& {
        return levelSystem.EntityRegistry.get<Transform2DComponent>(e);
        });

    std::cout << "[Lua] EnTT bindings registered.\n";
}

void LuaScriptingSystem::BindInputAPI()
{
}

void LuaScriptingSystem::StartUp()
{
    lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::table);

    lua.set_function("Print", [](const std::string& msg) {
        std::cout << "[Lua] " << msg << std::endl;
        });

    lua.set_function("print", [](const std::string& msg) { 
        std::cout << "[Lua] " << msg << std::endl;
        });

    BindCoreAPI();
    BindEnTTAPI();

    std::cout << "[Lua] Scripting system initialized.\n";
    lua.script(R"(
    print("[Lua] Test print from C++ - if you see this, printing works!")
)");
}

entt::entity LuaScriptingSystem::CreateEntityFromScript(
    const std::string& scriptPath,
    const std::string& entityName,
    glm::vec2 startPos,
    float startRot)
{
    try
    {
        std::cout << "[Lua] Loading script: " << scriptPath << std::endl;

        sol::table scriptTable = lua.script_file(scriptPath);
        std::cout << "[Lua] Script file loaded successfully, table valid = "
            << (scriptTable.valid() ? "true" : "false") << std::endl;

        entt::entity entity = levelSystem.EntityRegistry.create();

        auto& luaScript = levelSystem.EntityRegistry.emplace<LuaScriptComponent>(entity);
        luaScript.scriptTable = scriptTable;
        luaScript.entityName = entityName;

        std::cout << "[Lua] Table contents:" << std::endl;
        for (auto& kv : scriptTable) {
            std::cout << "   " << kv.first.as<std::string>() << std::endl;
        }

        sol::function onSpawn = scriptTable["OnSpawn"];
        std::cout << "[Lua] OnSpawn function valid = " << (onSpawn.valid() ? "YES" : "NO") << std::endl;

        if (onSpawn.valid())
        {
            std::cout << "[Lua] Calling OnSpawn..." << std::endl;
            onSpawn(entity, startPos, startRot);
            std::cout << "[Lua] OnSpawn call finished" << std::endl;
        }

        return entity;
    }
    catch (const sol::error& e)
    {
        std::cerr << "[Lua Error] " << e.what() << std::endl;
        return entt::null;
    }
}

void LuaScriptingSystem::Update(float deltaTime)
{
    auto view = levelSystem.EntityRegistry.view<LuaScriptComponent>();
    for (auto entity : view)
    {
        auto& script = view.get<LuaScriptComponent>(entity);

        sol::function onUpdate = script.scriptTable["OnUpdate"];
        if (onUpdate.valid())
        {
            if (onUpdate.is<bool>())
            {
                bool shouldDestroy = onUpdate.as<bool>();
                if (shouldDestroy)
                {
                    levelSystem.EntityRegistry.destroy(entity);
                    continue;
                }
            }
            else if (onUpdate.is<sol::table>())
            {
                sol::table events = onUpdate.as<sol::table>();
               // ProcessLuaEvents(entity, events);
            }
        }
    }
}

void LuaScriptingSystem::ShutDown()
{
}

