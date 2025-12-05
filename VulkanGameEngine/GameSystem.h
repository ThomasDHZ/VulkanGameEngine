#pragma once
#include <Platform.h>
#include "GameObjectSystem.h"
#include <RenderSystem.h>
#include "InputSystem.h"
#include <Camera.h>
#include <LevelSystem.h>

class MeshSystem;
class GameSystem
{
private:
    Vector<VkCommandBuffer> CommandBufferSubmitList;

    GameSystem() = default;
    ~GameSystem() = default;

public:
    // Delete copy/move
    GameSystem(const GameSystem&) = delete;
    GameSystem& operator=(const GameSystem&) = delete;

    // Singleton access
    static GameSystem& Get()
    {
        static GameSystem instance;
        return instance;
    }

    void StartUp(void* windowHandle);

#ifndef __ANDROID__
    void Update(float deltaTime);
#else
    void Update(void* windowHandle, float deltaTime);
#endif
    void DebugUpdate(float deltaTime);
    void Draw(float deltaTime);
    void Destroy();
};

// Keep old global for desktop compatibility (optional, but safe)
#ifndef __ANDROID__
extern GameSystem gameSystem;
inline GameSystem& GameSystem::Get() { return gameSystem; }
#endif
