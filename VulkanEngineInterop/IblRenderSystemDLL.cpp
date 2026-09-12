#include "IblRenderSystemDLL.h"

void IblRenderSystem_StartUp(const char* texturePath)
{
	iblRenderSystem.StartUp(texturePath);
}

void IblRenderSystem_SetEnvironmentMap(const char* texturePath)
{
	iblRenderSystem.SetEnvironmentMap(texturePath);
}
