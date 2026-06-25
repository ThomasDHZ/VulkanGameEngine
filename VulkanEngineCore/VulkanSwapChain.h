#pragma once
#include <Platform.h>

class DLL_EXPORT VulkanSwapchain
{
private:
	VkExtent2D							    m_SwapChainResolution{};
	uint32								    m_SwapChainImageCount = UINT32_MAX;
	ivec2								    m_renderResolution;
	bool								    m_RebuildSwapChainFlag = false;

	void								    StartUpSwapChain();
	void								    StartUpSwapChainImages();
	VkSurfaceKHR						    StartUpVulkanSurface(void* windowHandle, VkInstance instance);
	void								    StartUpSwapChainImageViews();
	VkExtent2D							    StartUpSwapChainExtent();
	void								    StartUpSemaphores();
	VkSurfaceFormatKHR					    FindSwapSurfaceFormat(Vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR					    FindSwapPresentMode(Vector<VkPresentModeKHR>& availablePresentModes);
	void								    DestroySwapChainImageViews();
	void								    DestroySwapChain();

public:
	uint32								    m_ImageIndex = UINT32_MAX;
	uint32								    m_CommandIndex = UINT32_MAX;
	VkSwapchainKHR						    m_Swapchain = VK_NULL_HANDLE;
	Vector<VkFence>						    m_InFlightFences = Vector<VkFence>();
	Vector<VkImage>						    m_SwapChainImages = Vector<VkImage>();
	Vector<VkImageView>					    m_SwapChainImageViews = Vector<VkImageView>();
	Vector<VkSemaphore>					    m_AcquireImageSemaphores = Vector<VkSemaphore>();
	Vector<VkSemaphore>					    m_PresentImageSemaphores = Vector<VkSemaphore>();
	VulkanSwapchain();
	VulkanSwapchain(const VulkanSwapchain& other);
	~VulkanSwapchain();

	void									Initialize(ivec2 renderResolution);
	void									RebuildSwapChain(void* windowHandle);
	void									StartFrame();
	void									EndFrame(VkCommandBuffer& commandBufferSubmit);
	void NextCommandIndex();

	Vector<VkSurfaceFormatKHR>			    GetSurfaceFormats(VkPhysicalDevice physicalDevice);
	Vector<VkPresentModeKHR>			    GetSurfacePresentModes(VkPhysicalDevice physicalDevice);
	VkSurfaceCapabilitiesKHR			    GetSurfaceCapabilities(VkPhysicalDevice physicalDevice);
	void								    TriggerSwapChainFlag();

	uint32*				    ImageIndex()		   ;
	VkSwapchainKHR*				    Swapchain()		 ;
	 uint32				    CommandIndex()		   ;
	 uint32				    SwapChainImageCount()  ;
	 VkExtent2D			    SwapChainResolution()  ;
	 ivec2					    RenderPassResolution() ;
	 Vector<VkImage>	    SwapChainImages()      ;
	Vector<VkImageView>	SwapChainImageViews()  ;
};