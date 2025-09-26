#include "VulkanWindow.h"
#include "MemorySystem.h"
#include "GLFWWindow.h"
#include "Mouse.h"
#include "Keyboard.h"
#include "GameController.h"
#include <glfw/include/GLFW/glfw3.h>

VulkanWindow vulkanWindow = VulkanWindow();


void VulkanWindow_CreateGraphicsWindow(WindowTypeEnum windowTypeEnum, const char* WindowName, uint32_t width, uint32_t height)
{
	// Initialize GLFW if needed (consider initializing once globally)
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	// Create the GLFW window
	GLFWwindow* window = glfwCreateWindow(width, height, WindowName, NULL, NULL);

	if (!window)
	{
		// Handle window creation failure
		fprintf(stderr, "Failed to create GLFW window\n");
		glfwTerminate(); // Consider if you want to terminate here
		return;
	}


	// Make context current
	glfwMakeContextCurrent(window);

	// Store the pointer to the window in user pointer for callbacks
	glfwSetWindowUserPointer(window, window);

	// Set error callback (consider setting this once globally)
	glfwSetErrorCallback(VulkanWindow_GLFWErrorCallBack);

	// Set framebuffer resize callback
	//glfwSetFramebufferSizeCallback(window, VulkanWindow_GLFWFrameBufferResizeCallBack);

	// Set other input callbacks as needed
	// glfwSetCursorPosCallback(window, VulkanWindow::GameEngine_GLFW_MouseMoveEvent);

	vulkanWindow.Width = width;
	vulkanWindow.Height = height;
	vulkanWindow.WindowType = windowTypeEnum;
	vulkanWindow.FrameBufferResized = false;
	vulkanWindow.ShouldClose = false;
	vulkanWindow.WindowHandle = window;
}

void VulkanWindow_PollEventHandler()
{
	glfwPollEvents();
}

void VulkanWindow_SwapBuffer()
{
}

void VulkanWindow_GetFrameBufferSize(void* windowHandle, int* width, int* height)
{
	GLFWwindow* window = static_cast<GLFWwindow*>(windowHandle);
	glfwGetFramebufferSize(window, width, height);
}

const char** VulkanWindow_GetInstanceExtensions(void* windowHandle, uint32_t* outExtensionCount, bool enableValidationLayers)
{
	GLFWwindow* window = static_cast<GLFWwindow*>(windowHandle);
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	size_t totalCount = glfwExtensionCount + (enableValidationLayers ? 1 : 0);

	const char** extensions = memorySystem.AddPtrBuffer<const char*>(totalCount * sizeof(const char*), __FILE__, __LINE__, __func__);
	if (!extensions) {
		fprintf(stderr, "Failed to allocate memory for extensions\n");
		return NULL;
	}

	for (uint32_t x = 0; x < glfwExtensionCount; x++) {
		extensions[x] = glfwExtensions[x];
	}

	if (enableValidationLayers)
	{
		extensions[glfwExtensionCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
	}

	*outExtensionCount = totalCount;
	return extensions;
}

void VulkanWindow_DestroyWindow(void* windowHandle)
{
	GLFWwindow* window = static_cast<GLFWwindow*>(windowHandle);
	glfwDestroyWindow(window);
}


void VulkanWindow_GLFWErrorCallBack(int error, const char* description)
{
	fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

//void VulkanWindow_GLFWFrameBufferResizeCallBack(void* windowHandle, int width, int height)
//{
//	GLFWwindow* window = static_cast<GLFWwindow*>(windowHandle);
//	// Retrieve your VulkanWindow object
//	//VulkanWindow* vw = static_cast<VulkanWindow*>(glfwGetWindowUserPointer(window));
//	//if (vw)
//	//{
//	//	vw->FrameBufferResized = true;
//	//	// Optionally update Width and Height
//	//	vw->Width = width;
//	//	vw->Height = height;
//
//	//	// Wait until size is non-zero
//	//	while (width == 0 || height == 0)
//	//	{
//	//		glfwGetFramebufferSize(window, &width, &height);
//	//		glfwWaitEvents();
//	//	}
//	//}
//}

bool VulkanWindow_WindowShouldClose(void* windowHandle)
{
	GLFWwindow* window = static_cast<GLFWwindow*>(windowHandle);
	return glfwWindowShouldClose(window);
}