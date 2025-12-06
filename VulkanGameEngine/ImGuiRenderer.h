//#pragma once
//#include "DLL.h"
//#include "Typedef.h"
//#include "VulkanRenderer.h"
//#include <GLFW/glfw3.h>
//
//#ifndef PLATFORM_ANDROID
//#include <imgui_impl_glfw.h>
//#include <imgui.h>
//#include <imgui_impl_vulkan.h>
//
//
//struct ImGuiRenderer
//{
//	VkRenderPass RenderPass = VK_NULL_HANDLE;
//	VkDescriptorPool ImGuiDescriptorPool = VK_NULL_HANDLE;
//	VkCommandBuffer ImGuiCommandBuffer = VK_NULL_HANDLE;
//	Vector<VkFramebuffer> SwapChainFramebuffers;
//};
// extern ImGuiRenderer imGuiRenderer;
//
//ImGuiRenderer ImGui_StartUp(const GraphicsRenderer& renderer);
//void ImGui_StartFrame();
//void ImGui_EndFrame();
//VkCommandBuffer ImGui_Draw(const GraphicsRenderer& renderer, ImGuiRenderer& imGuiRenderer);
//void ImGui_RebuildSwapChain(const GraphicsRenderer& renderer, ImGuiRenderer& imGuiRenderer);
//void ImGui_Destroy(GraphicsRenderer& renderer, ImGuiRenderer& imGuiRenderer);
//VkRenderPass ImGui_CreateRenderPass(const GraphicsRenderer& renderer);
//Vector<VkFramebuffer> ImGui_CreateRendererFramebuffers(const GraphicsRenderer& renderer, const VkRenderPass& renderPass);
//void ImGui_VkResult(VkResult err);
//#endif
//
