#include <VulkanWindow.h>
#include "ImGuiRenderer.h"
#include "imgui_impl_vulkan.h"
#include "Platform.h"

#ifndef PLATFORM_ANDROID

ImGuiRenderer imGuiRenderer = ImGuiRenderer();

// ------------------------------------------------------------
// Startup
// ------------------------------------------------------------
ImGuiRenderer ImGui_StartUp()
{
    ImGuiRenderer imGui;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Keep multi-viewport OFF for now
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    GLFWwindow* window = static_cast<GLFWwindow*>(vulkanWindow.GetWindowHandle());
    if (!window)
        throw std::runtime_error("ImGui_StartUp: GLFW window is null");

#ifdef _WIN32
    HWND hwnd = vulkanWindow.GetHWND(window);
    if (!hwnd) throw std::runtime_error("ImGui_StartUp: Win32 HWND is null");
#endif

    // false = do not install GLFW callbacks (you already have your own)
    ImGui_ImplGlfw_InitForVulkan(window, false);

    // Create ImGui render pass + framebuffers (must match swapchain format)
    imGui.RenderPass = ImGui_CreateRenderPass();
    imGui.SwapChainFramebuffers = ImGui_CreateRendererFramebuffers(imGui.RenderPass);

    // Descriptor pool
    VkDescriptorPoolSize poolSizes[] =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLER,                1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       1000 }
    };

    VkDescriptorPoolCreateInfo poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = 1000 * IM_ARRAYSIZE(poolSizes),
        .poolSizeCount = static_cast<uint32_t>(IM_ARRAYSIZE(poolSizes)),
        .pPoolSizes = poolSizes
    };

    VULKAN_THROW_IF_FAIL(vkCreateDescriptorPool(
        vulkan.LogicalDevice(), &poolInfo, nullptr, &imGui.ImGuiDescriptorPool));

    // Vulkan backend init
    ImGui_ImplVulkan_InitInfo initInfo
    {
        .Instance = vulkan.InstanceHandle(),
        .PhysicalDevice = vulkan.PhysicalDevice(),
        .Device = vulkan.LogicalDevice(),
        .QueueFamily = vulkan.Device().GraphicsFamily(),
        .Queue = vulkan.GraphicsQueue(),
        .DescriptorPool = imGui.ImGuiDescriptorPool,
        .MinImageCount = static_cast<uint32_t>(vulkan.SwapChainImageCount()),
        .ImageCount = static_cast<uint32_t>(vulkan.SwapChainImageCount()),
        .PipelineCache = VK_NULL_HANDLE,
        .PipelineInfoMain = ImGui_ImplVulkan_PipelineInfo
        {
            .RenderPass = imGui.RenderPass,
            .Subpass = 0,
            .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
        },
        .Allocator = nullptr,
        .CheckVkResultFn = ImGui_VkResult
    };

    ImGui_ImplVulkan_Init(&initInfo);
    return imGui;
}

// ------------------------------------------------------------
// Per-frame draw (call from GameSystem::Draw after scene)
// ------------------------------------------------------------
void ImGui_Draw(VkCommandBuffer& commandBuffer, ImGuiRenderer& imGuiRenderer)
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::ShowDemoWindow();   // remove later

    ImGui::Render();

    VkClearValue clearValue{};
    clearValue.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

    VkRenderPassBeginInfo renderPassInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = imGuiRenderer.RenderPass,
        .framebuffer = imGuiRenderer.SwapChainFramebuffers[vulkan.Swapchain().ImageIndex()],
        .renderArea =
        {
            .offset = { 0, 0 },
            .extent = vulkan.SwapChainResolution()
        },
        .clearValueCount = 1,
        .pClearValues = &clearValue
    };

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    vkCmdEndRenderPass(commandBuffer);
}

// ------------------------------------------------------------
// Swapchain rebuild
// ------------------------------------------------------------
void ImGui_RebuildSwapChain(ImGuiRenderer& imGuiRenderer)
{
    vkDeviceWaitIdle(vulkan.LogicalDevice());

    // Destroy old framebuffers (add proper destroy helpers if you have them)
    for (VkFramebuffer fb : imGuiRenderer.SwapChainFramebuffers)
    {
        if (fb != VK_NULL_HANDLE)
            vkDestroyFramebuffer(vulkan.LogicalDevice(), fb, nullptr);
    }
    imGuiRenderer.SwapChainFramebuffers.clear();

    if (imGuiRenderer.RenderPass != VK_NULL_HANDLE)
    {
        vkDestroyRenderPass(vulkan.LogicalDevice(), imGuiRenderer.RenderPass, nullptr);
        imGuiRenderer.RenderPass = VK_NULL_HANDLE;
    }

    imGuiRenderer.RenderPass = ImGui_CreateRenderPass();
    imGuiRenderer.SwapChainFramebuffers = ImGui_CreateRendererFramebuffers(imGuiRenderer.RenderPass);

    // Tell ImGui the swapchain changed (if your ImGui version needs it)
    // ImGui_ImplVulkan_SetMinImageCount(static_cast<uint32_t>(vulkan.SwapChainImageCount()));
}

// ------------------------------------------------------------
// Shutdown
// ------------------------------------------------------------
void ImGui_Destroy(ImGuiRenderer& imGuiRenderer)
{
    vkDeviceWaitIdle(vulkan.LogicalDevice());

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (imGuiRenderer.ImGuiDescriptorPool != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(vulkan.LogicalDevice(), imGuiRenderer.ImGuiDescriptorPool, nullptr);
        imGuiRenderer.ImGuiDescriptorPool = VK_NULL_HANDLE;
    }

    for (VkFramebuffer fb : imGuiRenderer.SwapChainFramebuffers)
    {
        if (fb != VK_NULL_HANDLE)
            vkDestroyFramebuffer(vulkan.LogicalDevice(), fb, nullptr);
    }
    imGuiRenderer.SwapChainFramebuffers.clear();

    if (imGuiRenderer.RenderPass != VK_NULL_HANDLE)
    {
        vkDestroyRenderPass(vulkan.LogicalDevice(), imGuiRenderer.RenderPass, nullptr);
        imGuiRenderer.RenderPass = VK_NULL_HANDLE;
    }
}

// ------------------------------------------------------------
// Render pass – MUST use swapchain format
// ------------------------------------------------------------
VkRenderPass ImGui_CreateRenderPass()
{
    VkAttachmentDescription colorAttachment
    {
        .format = vulkan.Swapchain().SwapChainImageFormat(), // CRITICAL
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,  // CLEAR while testing; use LOAD to keep scene
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

    VkRenderPass renderPass = VK_NULL_HANDLE;
    VULKAN_THROW_IF_FAIL(vkCreateRenderPass(vulkan.LogicalDevice(), &renderPassInfo, nullptr, &renderPass));
    return renderPass;
}

// ------------------------------------------------------------
// Framebuffers for each swapchain image
// ------------------------------------------------------------
Vector<VkFramebuffer> ImGui_CreateRendererFramebuffers(const VkRenderPass& renderPass)
{
    Vector<VkFramebuffer> frameBuffers(vulkan.SwapChainImageCount());

    for (size_t i = 0; i < vulkan.SwapChainImageCount(); ++i)
    {
        VkImageView attachments[] = { vulkan.Swapchain().SwapChainImageViews()[i] };

        VkFramebufferCreateInfo framebufferInfo
        {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = renderPass,
            .attachmentCount = 1,
            .pAttachments = attachments,
            .width = vulkan.SwapChainResolution().width,
            .height = vulkan.SwapChainResolution().height,
            .layers = 1
        };

        VULKAN_THROW_IF_FAIL(vkCreateFramebuffer(
            vulkan.LogicalDevice(), &framebufferInfo, nullptr, &frameBuffers[i]));
    }

    return frameBuffers;
}

void ImGui_VkResult(VkResult err)
{
    if (err == VK_SUCCESS)
        return;

    printf("ImGui Vulkan error: VkResult %d\n", static_cast<int>(err));
    if (err < 0)
        abort();
}

#endif // !PLATFORM_ANDROID