#pragma once
#include <vulkan/vulkan.h>
#include "Macro.h"
#include "DLL.h"

#ifdef __cplusplus
extern "C" {
#endif

    DLL_EXPORT const char* Renderer_GetError(VkResult result);

#define VULKAN_RESULT(call) { \
    VkResult result = (call); \
    if (result != VK_SUCCESS) { \
        fprintf(stderr, "Error in %s at %s:%d (%s): %s\n", \
                #call, __FILE__, __LINE__, __func__, Renderer_GetError(result)); \
    } \
}

#ifdef __cplusplus
}
#endif