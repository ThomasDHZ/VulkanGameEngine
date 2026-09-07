#include "MeshSystemDLL.h"

void MeshSystem_Update(const float& deltaTime)
{
	meshSystem.Update(deltaTime);
}

void MeshSystem_Destroy()
{
	meshSystem.Destroy();
}
