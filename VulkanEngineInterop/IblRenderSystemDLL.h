#include "DLL.h"
#include <IblRenderSystem.h>

#ifdef __cplusplus
extern "C" {
#endif
    DLL_EXPORT void IblRenderSystem_StartUp(const char* texturePath);
	DLL_EXPORT void IblRenderSystem_SetEnvironment(const char* texturePath);
#ifdef __cplusplus
}
#endif