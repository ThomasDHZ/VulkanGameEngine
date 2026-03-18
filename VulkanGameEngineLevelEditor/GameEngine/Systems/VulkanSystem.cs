using Silk.NET.SDL;
using Silk.NET.Vulkan;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using Vulkan;
using static VulkanGameEngineLevelEditor.LevelEditorForm;

namespace VulkanGameEngineLevelEditor.GameEngine.Systems
{

    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public unsafe struct GraphicsRendererDLL
    {
        public uint ApiVersion { get; private set; } = uint.MaxValue;
        public VkInstance Instance { get; private set; } = VulkanCSConst.VK_NULL_HANDLE;
        public VkDevice Device { get; private set; } = VulkanCSConst.VK_NULL_HANDLE;
        public VkPhysicalDevice PhysicalDevice { get; private set; } = VulkanCSConst.VK_NULL_HANDLE;
        public VkSurfaceKHR Surface { get; private set; } = VulkanCSConst.VK_NULL_HANDLE;
        public VkCommandPool CommandPool { get; private set; } = VulkanCSConst.VK_NULL_HANDLE;
        public VkDebugUtilsMessengerEXT DebugMessenger { get; private set; } = VulkanCSConst.VK_NULL_HANDLE;
        public VkSwapchainKHR Swapchain { get; private set; } = VulkanCSConst.VK_NULL_HANDLE;
        public VkQueue GraphicsQueue { get; private set; } = VulkanCSConst.VK_NULL_HANDLE;
        public VkQueue PresentQueue { get; private set; } = VulkanCSConst.VK_NULL_HANDLE;
        public VkFence* InFlightFencesPtr { get; private set; } = null;
        public VkImage* SwapChainImagesPtr { get; private set; } = null;
        public VkCommandBuffer* CommandBuffersPtr { get; private set; } = null;
        public VkImageView* SwapChainImageViewsPtr { get; private set; } = null;
        public VkSemaphore* AcquireImageSemaphoresPtr { get; private set; } = null;
        public VkSemaphore* PresentImageSemaphoresPtr { get; private set; } = null;
        public size_t InFlightFencesCount { get; private set; } = default(size_t);
        public size_t CommandBufferCount { get; private set; } = default(size_t);
        public size_t SwapChainImageViewCount { get; private set; } = default(size_t);
        public size_t AcquireImageSemaphoreCount { get; private set; } = default(size_t);
        public size_t PresentImageSemaphoreCount { get; private set; } = default(size_t);
        public size_t SwapChainImageCount { get; private set; } = default(size_t);
        public size_t ImageIndex { get; private set; } = default(size_t);
        public size_t CommandIndex { get; private set; } = default(size_t);
        public size_t GraphicsFamily { get; private set; } = default(size_t);
        public size_t PresentFamily { get; private set; } = default(size_t);
        public size_t MaxFramesInFlight { get; private set; } = default(size_t);

        public VkExtent2D SwapChainResolution { get; private set; } = new VkExtent2D();
        public VkFormat Format { get; private set; } = new VkFormat();
        public VkColorSpaceKHR ColorSpace { get; private set; } = new VkColorSpaceKHR();
        public VkPresentModeKHR PresentMode { get; private set; } = new VkPresentModeKHR();
        public VkSampleCountFlagBits MaxSampleCount { get; private set; } = new VkSampleCountFlagBits();
        public GraphicsRendererDLL()
        {
        }
    }

    public unsafe class VulkanSystem
    {
        public static readonly WindowType windowType = WindowType.Win32;
        public static void* RenderAreaHandle = null;
        public static uint ApiVersion { get; private set; } = uint.MaxValue;
        public static VkInstance Instance { get; private set; } = VulkanCSConst.VK_NULL_HANDLE;
        public static VkDevice Device { get; private set; } = VulkanCSConst.VK_NULL_HANDLE;
        public static VkPhysicalDevice PhysicalDevice { get; private set; } = VulkanCSConst.VK_NULL_HANDLE;
        public static VkSurfaceKHR Surface { get; private set; } = VulkanCSConst.VK_NULL_HANDLE;
        public static VkCommandPool CommandPool { get; private set; } = VulkanCSConst.VK_NULL_HANDLE;
        public static VkDebugUtilsMessengerEXT DebugMessenger { get; private set; } = VulkanCSConst.VK_NULL_HANDLE;
        public static VkSwapchainKHR Swapchain { get; private set; } = VulkanCSConst.VK_NULL_HANDLE;
        public static VkQueue GraphicsQueue { get; private set; } = VulkanCSConst.VK_NULL_HANDLE;
        public static VkQueue PresentQueue { get; private set; } = VulkanCSConst.VK_NULL_HANDLE;
        public static ListPtr<VkFence> InFlightFencesList { get; private set; } = new ListPtr<VkFence>();
        public static ListPtr<VkImage> SwapChainImagesList { get; private set; } = new ListPtr<VkImage>();
        public static ListPtr<VkCommandBuffer> CommandBuffersList { get; private set; } = new ListPtr<VkCommandBuffer>();
        public static ListPtr<VkImageView> SwapChainImageViewList { get; private set; } = new ListPtr<VkImageView>();
        public static ListPtr<VkSemaphore> AcquireImageSemaphoresList { get; private set; } = new ListPtr<VkSemaphore>();
        public static ListPtr<VkSemaphore> PresentImageSemaphoresList { get; private set; } = new ListPtr<VkSemaphore>();
        public static size_t SwapChainImageCount { get; private set; } = default(size_t);
        public static size_t ImageIndex { get; private set; } = default(size_t);
        public static size_t GraphicsFamily { get; private set; } = default(size_t);
        public static size_t PresentFamily { get; private set; } = default(size_t);
        public static size_t MaxFramesInFlight { get; private set; } = default(size_t);
        public static VkExtent2D SwapChainResolution { get; private set; } = new VkExtent2D();
        public static VkFormat Format { get; private set; } = new VkFormat();
        public static VkColorSpaceKHR ColorSpace { get; private set; } = new VkColorSpaceKHR();
        public static VkPresentModeKHR PresentMode { get; private set; } = new VkPresentModeKHR();
        public static VkSampleCountFlagBits MaxSampleCount { get; private set; } = new VkSampleCountFlagBits();

        public static void CreateLogMessageCallback(LogVulkanMessageDelegate callback)
        {
            DLLSystem.CallDLLFunc(() => VulkanSystem_CreateLogMessageCallback(callback));
        }

        public static void StartUpVulkan(WindowType windowType, void* renderAreaHandle, void* debuggerHandle)
        {
            var a = sizeof(GraphicsRendererDLL);
            RenderAreaHandle = renderAreaHandle;
            VkInstance instance = VulkanSystem_CreateVulkanInstance();
            VkSurfaceKHR surface = VulkanSystem_CreateVulkanSurface(RenderAreaHandle, instance);
            GraphicsRendererDLL renderer = DLLSystem.CallDLLFunc(() => VulkanSystem_RendererSetUp(RenderAreaHandle, ref instance, ref surface));
            ApiVersion = renderer.ApiVersion;
            Instance = renderer.Instance;
            Device = renderer.Device;
            PhysicalDevice = renderer.PhysicalDevice;
            Surface = renderer.Surface;
            CommandPool = renderer.CommandPool;
            DebugMessenger = renderer.DebugMessenger;
            Swapchain = renderer.Swapchain;
            GraphicsQueue = renderer.GraphicsQueue;
            PresentQueue = renderer.PresentQueue;
            InFlightFencesList = new ListPtr<nint>(renderer.InFlightFencesPtr, renderer.InFlightFencesCount);
            SwapChainImagesList = new ListPtr<nint>(renderer.SwapChainImagesPtr, renderer.SwapChainImageCount);
            CommandBuffersList = new ListPtr<nint>(renderer.CommandBuffersPtr, renderer.CommandBufferCount);
            SwapChainImageViewList = new ListPtr<nint>(renderer.SwapChainImageViewsPtr, renderer.SwapChainImageViewCount);
            AcquireImageSemaphoresList = new ListPtr<nint>(renderer.AcquireImageSemaphoresPtr, renderer.AcquireImageSemaphoreCount);
            PresentImageSemaphoresList = new ListPtr<nint>(renderer.PresentImageSemaphoresPtr, renderer.PresentImageSemaphoreCount);
            SwapChainImageCount = renderer.SwapChainImageCount;
            ImageIndex = renderer.ImageIndex;
            GraphicsFamily = renderer.GraphicsFamily;
            PresentFamily = renderer.PresentFamily;
            MaxFramesInFlight = renderer.MaxFramesInFlight;
            SwapChainResolution = renderer.SwapChainResolution;
            Format = renderer.Format;
            ColorSpace = renderer.ColorSpace;
            PresentMode = renderer.PresentMode;
            MaxSampleCount = renderer.MaxSampleCount;
        }

        public static VkInstance CreateVulkanInstance()
        {
            return DLLSystem.CallDLLFunc(() => VulkanSystem_CreateVulkanInstance());
        }

        public static VkSurfaceKHR CreateVulkanSurface()
        {
            return DLLSystem.CallDLLFunc(() => VulkanSystem_CreateVulkanSurface(RenderAreaHandle, Instance));
        }

        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.Cdecl)] public static extern void VulkanSystem_CreateLogMessageCallback(LogVulkanMessageDelegate callback);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern VkInstance VulkanSystem_CreateVulkanInstance();
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern VkSurfaceKHR VulkanSystem_CreateVulkanSurface(void* windowHandle, VkInstance instance);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern GraphicsRendererDLL VulkanSystem_RendererSetUp(void* windowHandle, ref VkInstance instance, ref VkSurfaceKHR surface);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern size_t VulkanSystem_GetInFlightFenceCount();
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern size_t VulkanSystem_GetSwapChainImageCount();
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern size_t VulkanSystem_GetCommandBufferCount();
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern size_t VulkanSystem_GetSwapChainImageViewCount();
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern size_t VulkanSystem_GetAcquireImageSemaphoreCount();
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern size_t VulkanSystem_GetPresentImageSemaphoreCount();
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern size_t VulkanSystem_GetInFlightFences(VkFence* pFence, size_t capacity);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern size_t VulkanSystem_GetSwapChainImages(VkImage* pImages, size_t capacity);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern size_t VulkanSystem_GetCommandBuffers(VkCommandBuffer* pImages, size_t capacity);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern size_t VulkanSystem_GetSwapChainImageViews(VkImageView* pImages, size_t capacity);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern size_t VulkanSystem_GetAcquireImageSemaphores(VkSemaphore* pImages, size_t capacity);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern size_t VulkanSystem_GetPresentImageSemaphores(VkSemaphore* pImages, size_t capacity);
    }
}
