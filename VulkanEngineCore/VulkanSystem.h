#pragma once
#include <Platform.h>
#include "VulkanInstance.h"
//#include "VulkanDevice.h"
#include "VulkanDebugger.h"
//#include "VulkanSwapchain.h"
//#include "VulkanCommandBuffer.h"

class VulkanSystem2
{
public:
	static VulkanSystem2& Get();

private:
	VulkanSystem2() = default;
	~VulkanSystem2() = default;
	VulkanSystem2(const VulkanSystem2&) = delete;
	VulkanSystem2& operator=(const VulkanSystem2&) = delete;
	VulkanSystem2(VulkanSystem2&&) = delete;
	VulkanSystem2& operator=(VulkanSystem2&&) = delete;

	void*											  m_windowHandle = nullptr;
	ivec2											  m_windowResolution;
	VulkanInstance									  m_instance;
	//VulkanDebugger									  m_debug;
	//VulkanDevice									  m_device;
	//VulkanSwapchain									  m_swapChain;
	//VulkanCommandBuffer								  m_commandBuffer;
	bool											  m_usingCustomSurface = false;

	void											  RendererSetUp(void* windowHandle, ivec2 renderResolution);

public:
	DLL_EXPORT void									  VulkanSetUp(ivec2 windowResolution, ivec2 renderResolution);
	DLL_EXPORT void									  VulkanSetUp(void* windowHandle, ivec2 windowResolution, ivec2 renderResolution);
	DLL_EXPORT uint32								  GetMemoryType(VkPhysicalDevice physicalDevice, uint32 typeFilter, VkMemoryPropertyFlags properties);
	DLL_EXPORT void									  Shutdown();

	DLL_EXPORT VulkanInstance						  Instance() { return m_instance; }
	//DLL_EXPORT VulkanDebugger						  Debug() { return m_debug; }
	//DLL_EXPORT VulkanDevice							  Device() { return m_device; }
	//// VulkanSwapchain						  Swapchain() { return m_swapChain; }
	//DLL_EXPORT VulkanCommandBuffer					  CommandBuffer() { return m_commandBuffer; }

	DLL_EXPORT [[nodiscard]] bool					  CustomSurface()			const;
	DLL_EXPORT [[nodiscard]] const void* WindowHandle()			const;
	DLL_EXPORT [[nodiscard]] ivec2					  WindowResolution()		const;
	DLL_EXPORT [[nodiscard]] uint32					  ApiVersion()				const;
	DLL_EXPORT [[nodiscard]] VkInstance				  InstanceHandle()			const;
	DLL_EXPORT [[nodiscard]] VkSurfaceKHR			  Surface()					const;
	//DLL_EXPORT [[nodiscard]] VkPhysicalDevice		  PhysicalDevice()			const { return m_device.PhysicalDevice(); }
	//DLL_EXPORT [[nodiscard]] VkDevice				  LogicalDevice()			const { return m_device.LogicalDevice(); }
	//DLL_EXPORT [[nodiscard]] VkQueue                  GraphicsQueue()			const { return m_device.GraphicsQueue(); }
	//DLL_EXPORT [[nodiscard]] VkQueue                  PresentQueue()			const { return m_device.PresentQueue(); }
	//DLL_EXPORT [[nodiscard]] VkSampleCountFlagBits    MaxSampleCount()			const { return m_device.MaxSampleCount(); }
	//DLL_EXPORT [[nodiscard]] uint32					  SwapChainImageCount()		const { return m_swapChain.ImageIndex(); }
	//DLL_EXPORT [[nodiscard]] VkExtent2D				  SwapChainResolution()		const { return m_swapChain.SwapChainResolution(); }
	//DLL_EXPORT [[nodiscard]] ivec2					  RenderPassResolution()	const { return m_swapChain.RenderPassResolution(); }
	//DLL_EXPORT [[nodiscard]] VkCommandPool            CommandPool()				const { return m_commandBuffer.CommandPool(); }
	//DLL_EXPORT [[nodiscard]] Vector<VkCommandBuffer>  CommandBufferList()		const { return m_commandBuffer.CommandBufferList(); }

};
extern DLL_EXPORT VulkanSystem2& vulkan;
inline VulkanSystem2& VulkanSystem2::Get()
{
	static VulkanSystem2 instance;
	return instance;
}