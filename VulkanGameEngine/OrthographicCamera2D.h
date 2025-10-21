#pragma once
#include "Camera.h"
#include <VulkanShaderSystem.h>

class OrthographicCamera2D : public Camera
{
public:
	OrthographicCamera2D();
	OrthographicCamera2D(float width, float height);
	OrthographicCamera2D(const vec2& viewScreenSize);
	OrthographicCamera2D(const vec2& viewScreenSize, const vec2& position);
	virtual ~OrthographicCamera2D();

	virtual void Update(ShaderPushConstant& sceneDataBuffer);
	virtual void UpdateKeyboard(float deltaTime) override;
	virtual void UpdateMouse() override;
};