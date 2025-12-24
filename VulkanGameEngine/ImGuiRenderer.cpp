#include "VulkanWindow.h"
#include "ImGuiRenderer.h"
#include "Platform.h"

#ifndef PLATFORM_ANDROID
ImGuiRenderer imGuiRenderer = ImGuiRenderer();

ImGuiRenderer ImGui_StartUp()
{
    ImGuiRenderer imGui;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui_ImplGlfw_InitForVulkan((GLFWwindow*)vulkanWindow->WindowHandle, true);

    imGui.RenderPass = ImGui_CreateRenderPass();
    imGui.SwapChainFramebuffers = ImGui_CreateRendererFramebuffers(imGui.RenderPass);

    VkDescriptorPoolSize poolSizes[] =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };
    VkDescriptorPoolCreateInfo pool_info =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = 1000 * IM_ARRAYSIZE(poolSizes),
        .poolSizeCount = (uint32)IM_ARRAYSIZE(poolSizes),
        .pPoolSizes = poolSizes
    };
    vkCreateDescriptorPool(vulkanSystem.Device, &pool_info, nullptr, &imGui.ImGuiDescriptorPool);

    ImGui_ImplVulkan_InitInfo init_info =
    {
        .Instance = vulkanSystem.Instance,
        .PhysicalDevice = vulkanSystem.PhysicalDevice,
        .Device = vulkanSystem.Device,
        .QueueFamily = vulkanSystem.GraphicsFamily,
        .Queue = vulkanSystem.GraphicsQueue,
        .DescriptorPool = imGui.ImGuiDescriptorPool,
        .RenderPass = imGui.RenderPass,
        .MinImageCount = static_cast<uint32>(vulkanSystem.SwapChainImageCount),
        .ImageCount = static_cast<uint32>(vulkanSystem.SwapChainImageCount),
        .PipelineCache = VK_NULL_HANDLE,
        .Allocator = nullptr,
        .CheckVkResultFn = ImGui_VkResult
    };
    ImGui_ImplVulkan_Init(&init_info);
    ImGui_ImplVulkan_CreateFontsTexture();

    return imGui;
}

void ImGui_StartFrame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("Button Window");
}

void ImGui_EndFrame()
{
    ImGui::End();
    ImGui::Render();
}

void ImGui_Draw(VkCommandBuffer& commandBuffer, ImGuiRenderer& imGuiRenderer)
{
    std::vector<VkClearValue> clearValues
    {
        VkClearValue {.color = { {0.0f, 0.0f, 0.0f, 1.0f} } }
    };

    VkRenderPassBeginInfo renderPassInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = imGuiRenderer.RenderPass,
        .framebuffer = imGuiRenderer.SwapChainFramebuffers[vulkanSystem.ImageIndex],
        .renderArea
        {
            .offset = { 0, 0 },
            .extent = vulkanSystem.SwapChainResolution,
        },
.clearValueCount = 0,
.pClearValues = nullptr
    };

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    vkCmdEndRenderPass(commandBuffer);
}

void ImGui_RebuildSwapChain(ImGuiRenderer& imGuiRenderer)
{
    vulkanSystem.DestroyRenderPass(vulkanSystem.Device, &imGuiRenderer.RenderPass);
    vulkanSystem.DestroyFrameBuffers(vulkanSystem.Device, &imGuiRenderer.SwapChainFramebuffers[0], vulkanSystem.SwapChainImageCount);
    imGuiRenderer.RenderPass = ImGui_CreateRenderPass();
    imGuiRenderer.SwapChainFramebuffers = ImGui_CreateRendererFramebuffers(imGuiRenderer.RenderPass);
}

void ImGui_Destroy(ImGuiRenderer& imGuiRenderer)
{
    ImGui_ImplVulkan_Shutdown();
    vulkanSystem.DestroyDescriptorPool(vulkanSystem.Device, &imGuiRenderer.ImGuiDescriptorPool);
    vulkanSystem.DestroyRenderPass(vulkanSystem.Device, &imGuiRenderer.RenderPass);
    vulkanSystem.DestroyFrameBuffers(vulkanSystem.Device, &imGuiRenderer.SwapChainFramebuffers[0], vulkanSystem.SwapChainImageCount);
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

VkRenderPass ImGui_CreateRenderPass()
{
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkAttachmentDescription colorAttachment
    {
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    VkAttachmentReference colorAttachmentRef
    {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkSubpassDescription subpass
    {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef
    };

    VkSubpassDependency dependency
    {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    };


    VkRenderPassCreateInfo renderPassInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &colorAttachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency
    };
    VULKAN_THROW_IF_FAIL(vkCreateRenderPass(vulkanSystem.Device, &renderPassInfo, nullptr, &renderPass));
    return renderPass;
}

Vector<VkFramebuffer> ImGui_CreateRendererFramebuffers(const VkRenderPass& renderPass)
{
    Vector<VkFramebuffer> frameBuffers = Vector<VkFramebuffer>(vulkanSystem.SwapChainImageCount);
    for (size_t x = 0; x < vulkanSystem.SwapChainImageCount; x++)
    {
        std::vector<VkImageView> attachments =
        {
            vulkanSystem.SwapChainImageViews[x]
        };

        VkFramebufferCreateInfo frameBufferInfo =
        {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = renderPass,
            .attachmentCount = static_cast<uint32>(attachments.size()),
            .pAttachments = attachments.data(),
            .width = vulkanSystem.SwapChainResolution.width,
            .height = vulkanSystem.SwapChainResolution.height,
            .layers = 1
        };
        VULKAN_THROW_IF_FAIL(vkCreateFramebuffer(vulkanSystem.Device, &frameBufferInfo, nullptr, &frameBuffers[x]));
    }
    return frameBuffers;
}

void ImGui_VkResult(VkResult err)
{
    if (err == 0) return;
    printf("VkResult %d\n", err);
    if (err < 0)
        abort();
}
#endif