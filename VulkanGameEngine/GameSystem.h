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

public:
    GameSystem();
    ~GameSystem();
    GameSystem(const GameSystem&) = delete;
    GameSystem& operator=(const GameSystem&) = delete;

#if __ANDROID__
    static GameSystem& Get()
    {
        static GameSystem instance;
        return instance;
    }
#endif

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
#ifndef __ANDROID__
extern GameSystem gameSystem;
#endif
