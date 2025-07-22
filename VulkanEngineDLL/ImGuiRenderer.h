#pragma once
#include <../External/glfw/include/GLFW/glfw3.h>
#include <ImGui/backends/imgui_impl_glfw.h>
#include <ImGui/imgui.h>
#include <imgui/backends/imgui_impl_vulkan.h>

#include "DLL.h"
#include "CTypedef.h"
#include "VulkanWindow.h"
#include "VulkanRenderer.h"

struct ImGuiRenderer
{
	VkRenderPass RenderPass = VK_NULL_HANDLE;
	VkDescriptorPool ImGuiDescriptorPool = VK_NULL_HANDLE;
	VkCommandBuffer ImGuiCommandBuffer = VK_NULL_HANDLE;
	Vector<VkFramebuffer> SwapChainFramebuffers;
};
DLL_EXPORT ImGuiRenderer imGuiRenderer;

DLL_EXPORT ImGuiRenderer ImGui_StartUp(const GraphicsRenderer& renderer);
DLL_EXPORT void ImGui_StartFrame();
DLL_EXPORT void ImGui_EndFrame();
DLL_EXPORT VkCommandBuffer ImGui_Draw(const GraphicsRenderer& renderer, ImGuiRenderer& imGuiRenderer);
DLL_EXPORT void ImGui_RebuildSwapChain(const GraphicsRenderer& renderer, ImGuiRenderer& imGuiRenderer);
DLL_EXPORT void ImGui_Destroy(GraphicsRenderer& renderer, ImGuiRenderer& imGuiRenderer);

VkRenderPass ImGui_CreateRenderPass(const GraphicsRenderer& renderer);
Vector<VkFramebuffer> ImGui_CreateRendererFramebuffers(const GraphicsRenderer& renderer, const VkRenderPass& renderPass);
void ImGui_VkResult(VkResult err);


