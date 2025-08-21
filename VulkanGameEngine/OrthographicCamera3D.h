#include "Camera.h"

class OrthographicCamera3D : public Camera
{
public:
	OrthographicCamera3D();
	OrthographicCamera3D(float width, float height);
	OrthographicCamera3D(const vec2& viewScreenSize);
	OrthographicCamera3D(const vec2& viewScreenSize, const vec2& position);
	virtual ~OrthographicCamera3D();

	virtual void Update(ShaderPushConstant& sceneDataBuffer);
	virtual void UpdateKeyboard(float deltaTime) override;
	virtual void UpdateMouse() override;
};