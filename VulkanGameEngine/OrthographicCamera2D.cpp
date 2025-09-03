#include "OrthographicCamera2D.h"
#include "SceneDataBuffer.h"
#include "ShaderSystem.h"

OrthographicCamera2D::OrthographicCamera2D()
{

}

OrthographicCamera2D::OrthographicCamera2D(float width, float height)
{
	Width = width;
	Height = height;
	AspectRatio = width / height;
	Zoom = 1.0f;

	Position = vec3(0.0f);
	ViewScreenSize = vec2(width, height);
	ProjectionMatrix = glm::ortho(0.0f, Width, Height, 0.0f);
	ViewMatrix = mat4(1.0f);
}

OrthographicCamera2D::OrthographicCamera2D(const vec2& viewScreenSize)
{
	Width = viewScreenSize.x;
	Height = viewScreenSize.y;
	AspectRatio = viewScreenSize.x / viewScreenSize.y;
	Zoom = 1.0f;

	Position = vec3(0.0f);
	ViewScreenSize = viewScreenSize;
	ProjectionMatrix = glm::ortho(0.0f, Width, Height, 0.0f);
	ViewMatrix = mat4(1.0f);
}

OrthographicCamera2D::OrthographicCamera2D(const vec2& viewScreenSize, const vec2& position)
{
	Width = viewScreenSize.x;
	Height = viewScreenSize.y;
	AspectRatio = viewScreenSize.x / viewScreenSize.y;
	Zoom = 1.0f;

	Position = glm::vec3(position, 0.0f);
	ViewScreenSize = viewScreenSize;
	ProjectionMatrix = glm::ortho(0.0f, Width, Height, 0.0f);
	ViewMatrix = mat4(1.0f);
}

OrthographicCamera2D::~OrthographicCamera2D()
{

}       

void OrthographicCamera2D::Update(ShaderPushConstant& sceneDataBuffer)
{
	mat4 view = mat4(1.0f);
	ProjectionMatrix = glm::ortho(0.0f, Width, Height, 0.0f);     
	memcpy(shaderSystem.SearchGlobalShaderConstantVar(&sceneDataBuffer, "Projection")->Value, &ProjectionMatrix, sizeof(ProjectionMatrix));
	memcpy(shaderSystem.SearchGlobalShaderConstantVar(&sceneDataBuffer, "View")->Value, &view, sizeof(view));
	memcpy(shaderSystem.SearchGlobalShaderConstantVar(&sceneDataBuffer, "CameraPosition")->Value, &Position, sizeof(Position));
}

void OrthographicCamera2D::UpdateKeyboard(float deltaTime)
{

}

void OrthographicCamera2D::UpdateMouse()
{

}