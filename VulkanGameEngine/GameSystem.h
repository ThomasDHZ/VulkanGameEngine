#pragma once
#include "GameObjectSystem.h"
#include <RenderSystem.h>
#include "InputSystem.h"
#include "OrthographicCamera2D.h"
#include "Level2D.h"

class MeshSystem;
class GameSystem
{
private:
	Vector<VkCommandBuffer>				CommandBufferSubmitList;

public:
	GameSystem();
	~GameSystem();

	void StartUp(void* windowHandle);
	void Update(const float& deltaTime);
	void DebugUpdate(const float& deltaTime);
	void Draw(const float& deltaTime);
	void Destroy();
};
extern GameSystem gameSystem;
