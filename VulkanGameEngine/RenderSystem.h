#pragma once
#include <VulkanError.h>
#include <VulkanRenderer.h>
#include <RenderSystem.h>
#include <VulkanRenderPass.h>
#include "ImGuiRenderer.h"
#include <nlohmann/json.hpp>
#include <VulkanShaderSystem.h>
#include <VulkanPipeline.h>
#include "VulkanWindow.h"

    VkCommandBuffer RenderBloomPass(VkGuid& renderPassId);
    VkCommandBuffer RenderFrameBuffer(VkGuid& renderPassId);
    VkCommandBuffer RenderLevel(VkGuid& renderPassId, VkGuid& levelId, const float deltaTime);