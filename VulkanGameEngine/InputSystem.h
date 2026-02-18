#pragma once
#include <MegaManObject.h>
#include <imgui_impl_vulkan.h>
#include "VulkanWindow.h"
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
public:

	void Update(const float& deltaTime);
};
extern DLL_EXPORT InputSystem& inputSystem;
inline InputSystem& InputSystem::Get()
{
	static InputSystem instance;
	return instance;
}
