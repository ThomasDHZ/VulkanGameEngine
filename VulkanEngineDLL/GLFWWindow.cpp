#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "GLFWWindow.h"
#include "Mouse.h"
#include "Keyboard.h"
#include "MemorySystem.h"
#include "GameController.h"

void joystick_callback(int jid, int event) {
	if (event == GLFW_CONNECTED && glfwJoystickIsGamepad(jid)) {
		printf("Controller connected: %s\n", glfwGetGamepadName(jid));
		GLFWWindow* window = ((GLFWWindow*)vulkanWindow);
		window->jid = jid;
	}
	else if (event == GLFW_DISCONNECTED) {
		printf("Controller disconnected\n");
		GLFWWindow* window = ((GLFWWindow*)vulkanWindow);
		window->jid = -1;
	}
}

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

void Window_GLFW_CreateGraphicsWindow(VulkanWindow* self, const char* WindowName, uint32_t width, uint32_t height)
{
	self->FrameBufferResized = false;
	self->Width = width;
	self->Height = height;
	self->ShouldClose = false;
	self->mouse.X = 0;
	self->mouse.Y = 0;
	self->mouse.WheelOffset = 0;
	memset(self->mouse.MouseButtonState, 0, sizeof(self->mouse.MouseButtonState));
	memset(self->keyboard.KeyPressed, 0, sizeof(self->keyboard.KeyPressed));

	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	self->WindowHandle = (void*)glfwCreateWindow(width, height, WindowName, NULL, NULL);

	glfwMakeContextCurrent((GLFWwindow*)self->WindowHandle);
	glfwSetWindowUserPointer((GLFWwindow*)self->WindowHandle, self);
	glfwSetErrorCallback(error_callback);
	glfwSetFramebufferSizeCallback((GLFWwindow*)self->WindowHandle, Window_GLFW_FrameBufferResizeCallBack);
	//glfwSetCursorPosCallback((GLFWWindow*)self->WindowHandle, GameEngine_GLFW_MouseMoveEvent);
	//glfwSetMouseButtonCallback((GLFWWindow*)self->WindowHandle, GameEngine_GLFW_MouseButtonPressedEvent);
	//glfwSetScrollCallback((GLFWWindow*)self->WindowHandle, GameEngine_GLFW_MouseWheelEvent);
	//glfwSetKeyCallback((GLFWWindow*)self->WindowHandle, GameEngine_GLFW_KeyboardKeyPressed);
	glfwSetJoystickCallback(joystick_callback);
}

void Window_GLFW_PollEventHandler(VulkanWindow* self)
{
	glfwPollEvents();
}

void Window_GLFW_FrameBufferResizeCallBack(GLFWwindow* self, int width, int height)
{
	//GLFWwindow* app = glfwGetWindowUserPointer(vulkanWindow->WindowHandle);
	//if (app)
	//{
	//	vulkanWindow->FrameBufferResized = true;
	//	glfwGetFramebufferSize(vulkanWindow->WindowHandle, &width, &height);

	//	while (width == 0 || height == 0)
	//	{
	//		glfwGetFramebufferSize(vulkanWindow->WindowHandle, &width, &height);
	//		glfwWaitEvents();
	//	}

	//	vulkanWindow->Width = width;
	//	vulkanWindow->Height = height;
	//}
}

void Window_GLFW_SwapBuffer(VulkanWindow* self)
{
}

const char** Window_GLFW_GetInstanceExtensions(struct VulkanWindow* self, uint32_t* outExtensionCount, bool enableValidationLayers)
{
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

void Window_GLFW_CreateSurface(void* windowHandle, VkInstance* instance, VkSurfaceKHR* surface)
{
	GLFWwindow* handle = (GLFWwindow*)windowHandle;
	glfwCreateWindowSurface(*instance, handle, NULL, surface);
}

void Window_GLFW_GetFrameBufferSize(void* windowHandle, int* width, int* height)
{
	GLFWwindow* handel = (GLFWwindow*)windowHandle;
	glfwGetFramebufferSize(handel, &*width, &*height);
}

void Window_GLFW_DestroyWindow(VulkanWindow* self)
{
	GLFWwindow* handel = (GLFWwindow*)self->WindowHandle;
	glfwDestroyWindow(handel);
	glfwTerminate();
}

bool Window_GLFW_WindowShouldClose(VulkanWindow* self)
{
	GLFWwindow* handel = (GLFWwindow*)self->WindowHandle;
	return  glfwWindowShouldClose(handel);
}
