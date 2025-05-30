#pragma once
#include "Typedef.h"

struct SceneDataBuffer
{
	alignas(4)  uint32  MeshBufferIndex = 0;
	alignas(16) mat4 Projection = mat4(1.0f);
	alignas(16) mat4 View = mat4(1.0f);
	alignas(16) vec3 CameraPosition = vec3(0.0f);
};

enum ComponentTypeEnum
{
	kUndefined,
	kRenderMesh2DComponent,
	kTransform2DComponent,
	kInputComponent,
	kSpriteComponent,
	kTransform3DComponent
};
