#include "VulkanInstance.h"
#include "VulkanSystem2.h"
#include <GLFW/glfw3.h>
#include "VulkanWindow.h"

VulkanInstance::VulkanInstance()
{
}

VulkanInstance::~VulkanInstance()
{
}

void VulkanInstance::Initialize()
{
    SetUpVulkanInstance();
    SetUpVulkanSurface();
}

void VulkanInstance::SetUpVulkanInstance()
{
#if defined(__linux__)
    unsetenv("VK_INSTANCE_LAYERS");
    unsetenv("VK_LAYER_PATH");
#endif

    VkInstance instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerCreateInfoEXT debugInfo;
    Vector<VkValidationFeatureEnableEXT> enabledList;
    Vector<VkValidationFeatureDisableEXT> disabledList;

#ifndef NDEBUG
    enabledList =
    {
        VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT,
        VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT,
        VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT,
        VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT
    };

    disabledList =
    {
        VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT,
        VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT
    };

    debugInfo =
    {
       .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
       .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                          VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
       .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                      VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
       .pfnUserCallback = VulkanDebugger::DebugCallBack
    };
#endif

    VkValidationFeaturesEXT validationFeatures = {
          .sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
          .pNext = nullptr,
          .enabledValidationFeatureCount = static_cast<uint32_t>(enabledList.size()),
          .pEnabledValidationFeatures = enabledList.data(),
          .disabledValidationFeatureCount = static_cast<uint32_t>(disabledList.size()),
          .pDisabledValidationFeatures = disabledList.data()
    };

#ifndef NDEBUG
    validationFeatures.pNext = &debugInfo;
#endif

    Vector<const char*> extensionNames = GetRequiredInstanceExtensions();
    VkApplicationInfo applicationInfo =
    {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Vulkan Application",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        #if defined(__ANDROID__)
            .apiVersion = VK_API_VERSION_1_3
        #else
            .apiVersion = VK_API_VERSION_1_4
        #endif
    };

    Vector<const char*> validationLayers = GetValidationLayerProperties();
    VkInstanceCreateInfo createInfo =
    {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = &validationFeatures,
        .pApplicationInfo = &applicationInfo,
        .enabledLayerCount = static_cast<uint32_t>(validationLayers.size()),
        .ppEnabledLayerNames = validationLayers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(extensionNames.size()),
        .ppEnabledExtensionNames = extensionNames.data()
    };
    VULKAN_THROW_IF_FAIL(vkCreateInstance(&createInfo, nullptr, &m_instance));

#ifndef NDEBUG
    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT");
    if (func)
    {
        //auto debug = vulkan.Debug().DebugMessengerHandle();
        //func(instance, &debugInfo, nullptr, &debug);
    }
#endif
}

void VulkanInstance::SetUpVulkanSurface()
{
    if (!vulkan.WindowHandle() || m_instance == VK_NULL_HANDLE)
    {
        fprintf(stderr, "Invalid window handle (%p) or instance (%p)\n", vulkan.WindowHandle(), (void*)m_instance);
    }

#if defined(_WIN32)
    if (vulkan.CustomSurface())
    {
        VkWin32SurfaceCreateInfoKHR surfaceCreateInfo =
        {
            .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
            .hinstance = GetModuleHandle(nullptr),
            .hwnd = (HWND)vulkan.WindowHandle()
        };
        VkResult result = vkCreateWin32SurfaceKHR(m_instance, &surfaceCreateInfo, nullptr, &m_surface);
    }
    else
    {
        vulkanWindow.CreateSurface(m_instance, m_surface);
    }

#elif defined(__linux__) && !defined(__ANDROID__)
    GLFWwindow* window = (GLFWwindow*)windowHandle;
    glfwCreateWindowSurface(instance, window, nullptr, &surface);

#elif defined(__ANDROID__)
    ANativeWindow* nativeWindow = (ANativeWindow*)windowHandle;

    VkAndroidSurfaceCreateInfoKHR surfaceInfo = { VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR };
    surfaceInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    surfaceInfo.pNext = nullptr;
    surfaceInfo.flags = 0;
    surfaceInfo.window = nativeWindow;

    VkResult result = vkCreateAndroidSurfaceKHR(instance, &surfaceInfo, nullptr, &surface);
    if (result != VK_SUCCESS || surface == VK_NULL_HANDLE)
    {
        __android_log_print(ANDROID_LOG_ERROR, "VulkanEngine", "FATAL: vkCreateAndroidSurfaceKHR failed! Result: %d", result);
        return;
    }
    __android_log_print(ANDROID_LOG_INFO, "VulkanEngine", "Android surface created successfully: %p", surface);
#endif
}

uint32 VulkanInstance::GetMaxApiVersion(VkPhysicalDevice physicalDevice)
{
    return 0;
//    uint32 version = vulkan.Device().GetPhysicalDeviceProperties(physicalDevice).apiVersion;
//#ifndef __ANDROID__
//    if ((VK_VERSION_MAJOR(version) == 1 && VK_VERSION_MINOR(version) == 4)) return VK_API_VERSION_1_4;
//#endif
//    if ((VK_VERSION_MAJOR(version) == 1 && VK_VERSION_MINOR(version) == 3)) return VK_API_VERSION_1_3;
//    if ((VK_VERSION_MAJOR(version) == 1 && VK_VERSION_MINOR(version) == 2)) return VK_API_VERSION_1_2;
//    if ((VK_VERSION_MAJOR(version) == 1 && VK_VERSION_MINOR(version) == 1)) return VK_API_VERSION_1_1;
}

Vector<const char*> VulkanInstance::GetRequiredInstanceExtensions()
{
    uint32 count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    Vector<VkExtensionProperties> availableExtensionList(count);
    vkEnumerateInstanceExtensionProperties(nullptr, &count, availableExtensionList.data());

    Vector<const char*> extensions;
    auto AddExtensionIfSupported = [&](const char* ext)
        {
            for (const auto& extension : availableExtensionList)
                if (strcmp(extension.extensionName, ext) == 0)
                {
                    extensions.push_back(ext);
                    std::cout << "Enabling instance extension: " << ext << '\n';
                    return;
                }
            std::cout << "Extension not supported: " << ext << '\n';
        };

    AddExtensionIfSupported(VK_KHR_SURFACE_EXTENSION_NAME);
    AddExtensionIfSupported(VK_EXT_COLOR_WRITE_ENABLE_EXTENSION_NAME);
#if defined(_WIN32)
    AddExtensionIfSupported(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(__linux__) && !defined(__ANDROID__)
    AddExtensionIfSupported("VK_KHR_xcb_surface");
    AddExtensionIfSupported("VK_KHR_wayland_surface");
#elif defined(__ANDROID__)
    AddExtensionIfSupported(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#endif

#if !defined(NDEBUG) && !defined(__ANDROID__)
    AddExtensionIfSupported(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    if (extensions.empty() ||
        (extensions.size() == 1 && extensions[0] == VK_KHR_SURFACE_EXTENSION_NAME))
    {
        throw std::runtime_error("No platform surface extension available — cannot create window.");
    }
    return extensions;
}

Vector<const char*> VulkanInstance::GetValidationLayerProperties()
{
    uint32 layerCount = UINT32_MAX;
    Vector<const char*> validationLayers;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    Vector<const char*> extensions;
    auto AddExtensionIfSupported = [&](const char* ext)
        {
            for (const auto& layer : availableLayers)
            {
                if (strcmp(layer.layerName, ext) == 0)
                {
                    extensions.push_back(ext);
                    std::cout << "Enabling instance extension: " << ext << '\n';
                    return;
                }
            }
            std::cout << "Extension not supported: " << ext << '\n';
            //   __android_log_print(ANDROID_LOG_WARN, "Vulkan", "Validation layers not available - running without validation");
        };
    AddExtensionIfSupported("VK_LAYER_KHRONOS_validation");

    return extensions;
}

uint32            VulkanInstance::ApiVersion()     const { return m_apiVersion; }
VkInstance        VulkanInstance::InstanceHandle() const { return m_instance; }
VkSurfaceKHR      VulkanInstance::Surface()        const { return m_surface; }