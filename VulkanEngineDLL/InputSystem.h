#pragma once
#include <Platform.h>
#include <InputEnum.h>

class InputSystem
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

    DLL_EXPORT void Update(const float& deltaTime);
    DLL_EXPORT bool IsKeyDown(int key) const;
    DLL_EXPORT bool IsKeyPressed(int key) const;
    DLL_EXPORT bool IsKeyReleased(int key) const;
};
extern DLL_EXPORT InputSystem& inputSystem;
inline InputSystem& InputSystem::Get()
{
    static InputSystem instance;
    return instance;
}
