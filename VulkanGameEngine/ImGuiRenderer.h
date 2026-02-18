//#pragma once
//#include "DLL.h"
//#include "Typedef.h"
//#include "VulkanSystem.h"
//#include <GLFW/glfw3.h>
//
//#ifndef PLATFORM_ANDROID
//#include <imgui_impl_glfw.h>
//#include <imgui.h>
//#include <imgui_impl_vulkan.h>
//
//struct ImGuiRenderer
//{
//	VkRenderPass RenderPass = VK_NULL_HANDLE;
//	VkDescriptorPool ImGuiDescriptorPool = VK_NULL_HANDLE;
//	Vector<VkFramebuffer> SwapChainFramebuffers;
//};
// extern ImGuiRenderer imGuiRenderer;
//
//ImGuiRenderer ImGui_StartUp();
//void ImGui_StartFrame();
//void ImGui_EndFrame();
//void ImGui_Draw(VkCommandBuffer& commandBuffer, ImGuiRenderer& imGuiRenderer);
//void ImGui_RebuildSwapChain(ImGuiRenderer& imGuiRenderer);
//void ImGui_Destroy(ImGuiRenderer& imGuiRenderer);
//VkRenderPass ImGui_CreateRenderPass();
//Vector<VkFramebuffer> ImGui_CreateRendererFramebuffers(const VkRenderPass& renderPass);
//void ImGui_VkResult(VkResult err);
//#endif
//
