#pragma once
#include <Platform.h>
#include "GameObjectSystem.h"
#include <RenderSystem.h>
#include "InputSystem.h"
#include <Camera.h>
#include <LevelSystem.h>

class MeshSystem;
#ifndef __ANDROID__
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
#else
	class GameSystem
	{
	private:
		static Vector<VkCommandBuffer>				CommandBufferSubmitList;

	public:
		GameSystem();
		~GameSystem();

		static void StartUp(void* windowHandle);
		static void Update(const float& deltaTime);
		static void DebugUpdate(const float& deltaTime);
		static void Draw(const float& deltaTime);
		static void Destroy();
	};
#endif 
extern GameSystem gameSystem;
