#pragma once
#include <vulkan/vulkan.h>
#include <SPIRV-Reflect/spirv_reflect.h>
#include <string>
#include <stdexcept>
#include <cstdio>
#include <cstdlib> 

class VulkanError : public std::runtime_error
{
public:
    VulkanError(VkResult result, const char* call, const char* file, int line, const char* func)
        : std::runtime_error(BuildMessage(result, call, file, line, func))
        , result(result)
    {
    }

    const VkResult result;

    // INLINE + STATIC = Works on MSVC, Clang, GCC, everywhere
    static inline const char* GetVulkanError(VkResult r)
    {
        switch (r) {
#define C(x) case VK_##x: return "VK_" #x
            C(NOT_READY); C(TIMEOUT); C(EVENT_SET); C(EVENT_RESET); C(INCOMPLETE);
            C(ERROR_OUT_OF_HOST_MEMORY); C(ERROR_OUT_OF_DEVICE_MEMORY);
            C(ERROR_INITIALIZATION_FAILED); C(ERROR_DEVICE_LOST);
            C(ERROR_MEMORY_MAP_FAILED); C(ERROR_LAYER_NOT_PRESENT);
            C(ERROR_EXTENSION_NOT_PRESENT); C(ERROR_FEATURE_NOT_PRESENT);
            C(ERROR_INCOMPATIBLE_DRIVER); C(ERROR_TOO_MANY_OBJECTS);
            C(ERROR_FORMAT_NOT_SUPPORTED); C(ERROR_FRAGMENTED_POOL);
            C(ERROR_UNKNOWN); C(ERROR_OUT_OF_POOL_MEMORY);
            C(ERROR_INVALID_EXTERNAL_HANDLE); C(ERROR_FRAGMENTATION);
            C(ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS); C(PIPELINE_COMPILE_REQUIRED);
            C(ERROR_SURFACE_LOST_KHR); C(ERROR_NATIVE_WINDOW_IN_USE_KHR);
            C(SUBOPTIMAL_KHR); C(ERROR_OUT_OF_DATE_KHR);
            C(ERROR_INCOMPATIBLE_DISPLAY_KHR); C(ERROR_VALIDATION_FAILED_EXT);
            C(ERROR_INVALID_SHADER_NV); C(ERROR_COMPRESSION_EXHAUSTED_EXT);
#if defined(VK_INCOMPATIBLE_SHADER_BINARY_EXT)
            C(INCOMPATIBLE_SHADER_BINARY_EXT);
#endif
#undef C
        default: return "UNKNOWN_VK_RESULT";
        }
    }

    static inline const char* GetShaderReflectError(SpvReflectResult r)
    {
        switch (r) {
#define C(x) case SPV_REFLECT_##x: return "SPV_REFLECT_" #x
            C(RESULT_SUCCESS);
            C(RESULT_NOT_READY);
            C(RESULT_ERROR_PARSE_FAILED);
            C(RESULT_ERROR_ALLOC_FAILED);
            C(RESULT_ERROR_RANGE_EXCEEDED);
            C(RESULT_ERROR_NULL_POINTER);
            C(RESULT_ERROR_INTERNAL_ERROR);
            C(RESULT_ERROR_COUNT_MISMATCH);
            C(RESULT_ERROR_ELEMENT_NOT_FOUND);
            C(RESULT_ERROR_SPIRV_INVALID_CODE_SIZE);
            C(RESULT_ERROR_SPIRV_INVALID_MAGIC_NUMBER);
            C(RESULT_ERROR_SPIRV_UNEXPECTED_EOF);
            C(RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE);
            C(RESULT_ERROR_SPIRV_SET_NUMBER_OVERFLOW);
            C(RESULT_ERROR_SPIRV_INVALID_STORAGE_CLASS);
            C(RESULT_ERROR_SPIRV_RECURSION);
            C(RESULT_ERROR_SPIRV_INVALID_INSTRUCTION);
            C(RESULT_ERROR_SPIRV_UNEXPECTED_BLOCK_DATA);
            C(RESULT_ERROR_SPIRV_INVALID_BLOCK_MEMBER_REFERENCE);
            C(RESULT_ERROR_SPIRV_INVALID_ENTRY_POINT);
            C(RESULT_ERROR_SPIRV_INVALID_EXECUTION_MODE);
            C(RESULT_ERROR_SPIRV_MAX_RECURSIVE_EXCEEDED);
#undef C
        default: return "UNKNOWN_SPIRV_REFLECT_RESULT";
        }
    }

private:
    static std::string BuildMessage(VkResult r, const char* call, const char* file, int line, const char* func)
    {
        char buf[2048];
        snprintf(buf, sizeof(buf),
            "\n"
            "VULKAN FATAL ERROR\n"
            "────────────────────────────────────\n"
            " Call   : %s\n"
            " Result : %s (%d)\n"
            " File   : %s:%d\n"
            " Func   : %s\n"
            "────────────────────────────────────\n",
            call, GetVulkanError(r), (int)r, file, line, func);
        return std::string(buf);
    }
};

#define VULKAN_THROW_IF_FAIL(call) \
    do { \
        VkResult vk_res = (call); \
        if (vk_res != VK_SUCCESS) [[unlikely]] { \
            throw VulkanError(vk_res, #call, __FILE__, __LINE__, __func__); \
        } \
    } while (0)

#define VULKAN_CHECK(call) \
    do { \
        VkResult vk_res = (call); \
        if (vk_res != VK_SUCCESS) [[unlikely]] { \
            fprintf(stderr, \
                "\033[1;31m\n" \
                "VULKAN CRITICAL FAILURE\n" \
                "────────────────────────────────────\n" \
                " Call  : %s\n" \
                " Error : %s (%d)\n" \
                " File  : %s:%d\n" \
                " Func  : %s\n" \
                "────────────────────────────────────\n\033[0m", \
                #call, VulkanError::GetVulkanError(vk_res), (int)vk_res, \
                __FILE__, __LINE__, __func__); \
            std::abort(); \
        } \
    } while (0)

#define SPV_VULKAN_RESULT(call) \
    do { \
        SpvReflectResult res = (call); \
        if (res != SPV_REFLECT_RESULT_SUCCESS) { \
            fprintf(stderr, "\033[1;33m[SPIRV-REFLECT WARNING] %s → %s\033[0m at %s:%d\n", \
                #call, VulkanError::GetShaderReflectError(res), __FILE__, __LINE__); \
        } \
    } while (0)