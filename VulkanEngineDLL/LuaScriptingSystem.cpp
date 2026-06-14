//#include "LuaScriptingSystem.h"
//#include "LevelSystem.h"
//
//LuaScriptingSystem& luaScriptingSystem = LuaScriptingSystem::Get();
//
//void LuaScriptingSystem::BindCoreAPI()
//{
//    lua.set_function("print", [](const std::string& msg) {
//        std::cout << "[Lua] " << msg << std::endl;
//        });
//
//    lua.set_function("Print", [](const std::string& msg) {
//        std::cout << "[Lua] " << msg << std::endl;
//        });
//}
//
//void LuaScriptingSystem::BindEnTTAPI()
//{
//    lua.new_usertype<entt::entity>("Entity",
//        sol::meta_function::construct, sol::no_constructor,
//        "GetID", [](entt::entity e) { return static_cast<uint32_t>(e); }
//    );
//
//    lua.new_usertype<Transform2DComponent>("Transform",
//        sol::constructors<Transform2DComponent()>(),
//        "Position", sol::property(
//            [](const Transform2DComponent& t) { return t.GameObjectPosition; },
//            [](Transform2DComponent& t, const glm::vec2& v) { t.GameObjectPosition = v; t.Dirty = true; }
//        ),
//        "Rotation", sol::property(
//            [](const Transform2DComponent& t) { return t.GameObjectRotation; },
//            [](Transform2DComponent& t, const glm::vec2& v) { t.GameObjectRotation = v; t.Dirty = true; }
//        ),
//        "Scale", sol::property(
//            [](const Transform2DComponent& t) { return t.GameObjectScale; },
//            [](Transform2DComponent& t, const glm::vec2& v) { t.GameObjectScale = v; t.Dirty = true; }
//        ),
//        "Move", sol::overload(
//            sol::resolve<void(float, float)>(&Transform2DComponent::Move),
//            sol::resolve<void(const glm::vec2&)>(&Transform2DComponent::Move)
//        )
//    );
//
//    lua.set_function("GetTransform", [&](entt::entity e) -> Transform2DComponent& {
//        return gameObjectSystem.EntityRegistry.get<Transform2DComponent>(e);
//        });
//}
//
//void LuaScriptingSystem::StartUp()
//{
//    lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::table);
//
//    BindCoreAPI();
//    BindEnTTAPI();
//
//    std::cout << "[Lua] Scripting system initialized.\n";
//}
//
//entt::entity LuaScriptingSystem::CreateEntityFromScript(const std::string& scriptPath, const std::string& entityName)
//{
//    try
//    {
//        sol::table scriptTable = lua.script_file(scriptPath);
//
//        if (!scriptTable.valid())
//        {
//            std::cerr << "[Lua Error] Failed to load script: " << scriptPath << std::endl;
//            return entt::null;
//        }
//
//        entt::entity entity = gameObjectSystem.EntityRegistry.create();
//        gameObjectSystem.EntityRegistry.emplace<LuaScriptComponent>(entity, scriptTable, scriptPath);
//
//        // Call OnSpawn if it exists
//        sol::function onSpawn = scriptTable["OnSpawn"];
//        if (onSpawn.valid())
//        {
//            onSpawn(entity, glm::vec2(0.0f, 0.0f));
//            std::cout << "[Lua] OnSpawn called for " << entityName << std::endl;
//        }
//
//        return entity;
//    }
//    catch (const sol::error& e)
//    {
//        std::cerr << "[Lua Error] " << e.what() << std::endl;
//        return entt::null;
//    }
//}
//
//void LuaScriptingSystem::Update(float deltaTime)
//{
//    auto view = gameObjectSystem.EntityRegistry.view<LuaScriptComponent>();
//    for (auto entity : view)
//    {
//        auto& script = view.get<LuaScriptComponent>(entity);
//        sol::function onUpdate = script.scriptTable["OnUpdate"];
//
//        if (onUpdate.valid())
//        {
//            onUpdate(deltaTime);
//        }
//    }
//}