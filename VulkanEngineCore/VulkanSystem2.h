#pragma once
#include <Platform.h>
#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanDebugger.h"
#include "VulkanSwapchain.h"
#include "VulkanCommandBuffer.h"

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
	VulkanDebugger									  m_debug;
	VulkanDevice									  m_device;
	VulkanSwapchain									  m_swapChain;
	VulkanCommandBuffer								  m_commandBuffer;
	bool											  m_usingCustomSurface = false;

	void											  RendererSetUp(void* windowHandle, ivec2 renderResolution);

public:
	DLL_EXPORT void									  VulkanSetUp(ivec2 windowResolution, ivec2 renderResolution);
	DLL_EXPORT void									  VulkanSetUp(void* windowHandle, ivec2 windowResolution, ivec2 renderResolution);
	DLL_EXPORT uint32								  GetMemoryType(VkPhysicalDevice physicalDevice, uint32 typeFilter, VkMemoryPropertyFlags properties);
	DLL_EXPORT void									  Shutdown();

	DLL_EXPORT VulkanInstance						  Instance();
	DLL_EXPORT VulkanDebugger						  Debug();
	DLL_EXPORT VulkanDevice							  Device();
	DLL_EXPORT VulkanSwapchain						  Swapchain();
	DLL_EXPORT VulkanCommandBuffer					  CommandBuffer();

	DLL_EXPORT [[nodiscard]] bool					  CustomSurface()			;
	DLL_EXPORT [[nodiscard]] const void*			  WindowHandle()			;
	DLL_EXPORT [[nodiscard]] ivec2					  WindowResolution()		;
	DLL_EXPORT [[nodiscard]] uint32					  ApiVersion()				;
	DLL_EXPORT [[nodiscard]] VkInstance				  InstanceHandle()			;
	DLL_EXPORT [[nodiscard]] VkSurfaceKHR			  Surface()					;
	DLL_EXPORT [[nodiscard]] VkPhysicalDevice		  PhysicalDevice()			;
	DLL_EXPORT [[nodiscard]] VkDevice				  LogicalDevice()			;
	DLL_EXPORT [[nodiscard]] VkQueue                  GraphicsQueue()			;
	DLL_EXPORT [[nodiscard]] VkQueue                  PresentQueue()			;
	DLL_EXPORT [[nodiscard]] VkSampleCountFlagBits    MaxSampleCount()			;
	DLL_EXPORT [[nodiscard]] uint32					  SwapChainImageCount()		;
	DLL_EXPORT [[nodiscard]] VkExtent2D				  SwapChainResolution()		;
	DLL_EXPORT [[nodiscard]] ivec2					  RenderPassResolution()	;
	DLL_EXPORT [[nodiscard]] VkCommandPool            CommandPool()				;
	DLL_EXPORT [[nodiscard]] Vector<VkCommandBuffer>  CommandBufferList()		;
};
extern DLL_EXPORT VulkanSystem2& vulkan;
inline VulkanSystem2& VulkanSystem2::Get()
{
	static VulkanSystem2 instance;
	return instance;
}