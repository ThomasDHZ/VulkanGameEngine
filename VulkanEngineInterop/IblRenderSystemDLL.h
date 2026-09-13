#include "DLL.h"
#include <IblRenderSystem.h>
#include "ToDLL.h"

#ifdef __cplusplus
extern "C" {
#endif
    DLL_EXPORT void IblRenderSystem_StartUp(const char* texturePath);
	DLL_EXPORT RenderPassNodeDLL* IblRenderSystem_CreateDrawCommands(VkCommandBuffer& commandBuffer, const float& deltaTime, size_t* renderPassNodeCount);
	DLL_EXPORT void IblRenderSystem_SetEnvironmentMap(const char* texturePath);
#ifdef __cplusplus
}
#endif