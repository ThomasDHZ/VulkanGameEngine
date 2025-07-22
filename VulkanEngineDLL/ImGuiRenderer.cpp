#include "ImGuiRenderer.h"
#include "VulkanError.h"

ImGuiRenderer imGuiRenderer = ImGuiRenderer();

ImGuiRenderer ImGui_StartUp(const GraphicsRenderer& renderer)
{
    ImGuiRenderer imGui;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();
    switch (vulkanWindow->WindowType)
    {
        //case SDL: ImGui_ImplSDL3_InitForVulkan((SDL_Window*)vulkanWindow->WindowHandle); break;
    case GLFW: ImGui_ImplGlfw_InitForVulkan((GLFWwindow*)vulkanWindow->WindowHandle, true); break;
    }

    imGui.RenderPass = ImGui_CreateRenderPass(renderer);
    imGui.SwapChainFramebuffers = ImGui_CreateRendererFramebuffers(renderer, imGui.RenderPass);

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
    VULKAN_RESULT(Renderer_CreateDescriptorPool(renderer.Device, &imGui.ImGuiDescriptorPool, &pool_info));

    for (size_t x = 0; x < renderer.SwapChainImageCount; x++)
    {
        VkCommandBufferAllocateInfo commandBufferAllocateInfo
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = renderer.CommandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };
        VULKAN_RESULT(vkAllocateCommandBuffers(renderer.Device, &commandBufferAllocateInfo, &imGui.ImGuiCommandBuffer));
    }

    ImGui_ImplVulkan_InitInfo init_info =
    {
        .Instance = renderer.Instance,
        .PhysicalDevice = renderer.PhysicalDevice,
        .Device = renderer.Device,
        .QueueFamily = renderer.GraphicsFamily,
        .Queue = renderer.GraphicsQueue,
        .DescriptorPool = imGui.ImGuiDescriptorPool,
        .RenderPass = imGui.RenderPass,
        .MinImageCount = static_cast<uint32>(renderer.SwapChainImageCount),
        .ImageCount = static_cast<uint32>(renderer.SwapChainImageCount),
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
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
}

void ImGui_EndFrame()
{
    ImGui::End();
    ImGui::Render();
}

VkCommandBuffer ImGui_Draw(const GraphicsRenderer& renderer, ImGuiRenderer& imGuiRenderer)
{
    std::vector<VkClearValue> clearValues
    {
        VkClearValue {.color = { {0.0f, 0.0f, 0.0f, 1.0f} } },
        VkClearValue {.depthStencil = { 1.0f, 0 } }
    };

    VkRenderPassBeginInfo renderPassInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = imGuiRenderer.RenderPass,
        .framebuffer = imGuiRenderer.SwapChainFramebuffers[renderer.ImageIndex],
        .renderArea
        {
            .offset = { 0, 0 },
            .extent = renderer.SwapChainResolution,
        },
        .clearValueCount = static_cast<uint32>(clearValues.size()),
        .pClearValues = clearValues.data()
    };

    VkCommandBufferBeginInfo beginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT
    };

    VULKAN_RESULT(vkResetCommandBuffer(imGuiRenderer.ImGuiCommandBuffer, 0));
    VULKAN_RESULT(vkBeginCommandBuffer(imGuiRenderer.ImGuiCommandBuffer, &beginInfo));
    vkCmdBeginRenderPass(imGuiRenderer.ImGuiCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), imGuiRenderer.ImGuiCommandBuffer);
    vkCmdEndRenderPass(imGuiRenderer.ImGuiCommandBuffer);
    VULKAN_RESULT(vkEndCommandBuffer(imGuiRenderer.ImGuiCommandBuffer));

    return imGuiRenderer.ImGuiCommandBuffer;
}

void ImGui_RebuildSwapChain(const GraphicsRenderer& renderer, ImGuiRenderer& imGuiRenderer)
{
    Renderer_DestroyRenderPass(renderer.Device, &imGuiRenderer.RenderPass);
    Renderer_DestroyFrameBuffers(renderer.Device, &imGuiRenderer.SwapChainFramebuffers[0], renderer.SwapChainImageCount);
    imGuiRenderer.RenderPass = ImGui_CreateRenderPass(renderer);
    imGuiRenderer.SwapChainFramebuffers = ImGui_CreateRendererFramebuffers(renderer, imGuiRenderer.RenderPass);
}

void ImGui_Destroy(GraphicsRenderer& renderer, ImGuiRenderer& imGuiRenderer)
{
    ImGui_ImplVulkan_Shutdown();
    Renderer_DestroyCommandBuffers(renderer.Device, &renderer.CommandPool, &imGuiRenderer.ImGuiCommandBuffer, 1);
    Renderer_DestroyDescriptorPool(renderer.Device, &imGuiRenderer.ImGuiDescriptorPool);
    Renderer_DestroyRenderPass(renderer.Device, &imGuiRenderer.RenderPass);
    Renderer_DestroyFrameBuffers(renderer.Device, &imGuiRenderer.SwapChainFramebuffers[0], renderer.SwapChainImageCount);
    switch (vulkanWindow->WindowType)
    {
        //case SDL: ImGui_ImplSDL3_Shutdown(); break;
    case GLFW: ImGui_ImplGlfw_Shutdown(); break;
    }
    ImGui::DestroyContext();
}

VkRenderPass ImGui_CreateRenderPass(const GraphicsRenderer& renderer)
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
    VULKAN_RESULT(vkCreateRenderPass(renderer.Device, &renderPassInfo, nullptr, &renderPass));
    return renderPass;
}

Vector<VkFramebuffer> ImGui_CreateRendererFramebuffers(const GraphicsRenderer& renderer, const VkRenderPass& renderPass)
{
    Vector<VkFramebuffer> frameBuffers = Vector<VkFramebuffer>(renderer.SwapChainImageCount);
    for (size_t x = 0; x < renderer.SwapChainImageCount; x++)
    {
        std::vector<VkImageView> attachments =
        {
            renderer.SwapChainImageViews[x]
        };

        VkFramebufferCreateInfo frameBufferInfo =
        {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = renderPass,
            .attachmentCount = static_cast<uint32>(attachments.size()),
            .pAttachments = attachments.data(),
            .width = renderer.SwapChainResolution.width,
            .height = renderer.SwapChainResolution.height,
            .layers = 1
        };
        VULKAN_RESULT(vkCreateFramebuffer(renderer.Device, &frameBufferInfo, nullptr, &frameBuffers[x]));
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
