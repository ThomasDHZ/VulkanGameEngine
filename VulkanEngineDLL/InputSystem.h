#pragma once
#include "DLL.h"
#include <Platform.h>
#include <InputEnum.h>

class ENGINE_DLL_EXPORT InputSystem
{
public:
    static InputSystem& Get();

private:
    InputSystem() = default;
    ~InputSystem() = default;
    InputSystem(const InputSystem&) = delete;
    InputSystem& operator=(const InputSystem&) = delete;
    InputSystem(InputSystem&&) = delete;
    InputSystem& operator=(InputSystem&&) = delete;

    KeyState m_previousKeys[MAXKEYBOARDKEY]{};

public:

     void Update(const float& deltaTime);
     bool IsKeyDown(int key) const;
     bool IsKeyPressed(int key) const;
     bool IsKeyReleased(int key) const;
};
ENGINE_DLL_EXPORT extern  InputSystem& inputSystem;
inline InputSystem& InputSystem::Get()
{
    static InputSystem instance;
    return instance;
}
