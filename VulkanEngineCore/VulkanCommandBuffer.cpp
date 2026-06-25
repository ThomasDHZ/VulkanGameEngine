#include "VulkanCommandBuffer.h"
#include "VulkanSystem2.h"
#include "VulkanSwapchain.h"

VulkanCommandBuffer::VulkanCommandBuffer() = default;
VulkanCommandBuffer::~VulkanCommandBuffer() = default;

VulkanCommandBuffer::VulkanCommandBuffer(const VulkanCommandBuffer& other)
{
    m_CommandPool = other.m_CommandPool;
    m_CommandBufferList = other.m_CommandBufferList;
}

void VulkanCommandBuffer::Initialize()
{
    SetUpCommandPool();
    SetUpCommandBuffers();
}

void VulkanCommandBuffer::SetUpCommandPool()
{
    VkCommandPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = vulkan.Device().GraphicsFamily()
    };

    VULKAN_THROW_IF_FAIL(vkCreateCommandPool(vulkan.LogicalDevice(), &poolInfo, nullptr, &m_CommandPool));
}

void VulkanCommandBuffer::SetUpCommandBuffers()
{
    uint32 imageCount = vulkan.Swapchain().SwapChainImageCount();
    m_CommandBufferList.resize(imageCount, VK_NULL_HANDLE);

    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = m_CommandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = imageCount
    };

    VULKAN_THROW_IF_FAIL(vkAllocateCommandBuffers(vulkan.LogicalDevice(), &allocInfo, m_CommandBufferList.data()));
}

VkCommandBuffer VulkanCommandBuffer::GetCurrentCommandBuffer() const
{
    return m_CommandBufferList[vulkan.Swapchain().CommandIndex()];
}

VkCommandBuffer VulkanCommandBuffer::BeginSingleUseCommand()
{
    VkCommandBuffer cmd = VK_NULL_HANDLE;
    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = m_CommandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };
    vkAllocateCommandBuffers(vulkan.LogicalDevice(), &allocInfo, &cmd);

    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    vkBeginCommandBuffer(cmd, &beginInfo);
    return cmd;
}

void VulkanCommandBuffer::EndSingleUseCommand(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer
    };

    vkQueueSubmit(vulkan.Device().GraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(vulkan.Device().GraphicsQueue());
    vkFreeCommandBuffers(vulkan.LogicalDevice(), m_CommandPool, 1, &commandBuffer);
}

VkCommandPool                   VulkanCommandBuffer::CommandPool()       const { return m_CommandPool; }
const Vector<VkCommandBuffer>& VulkanCommandBuffer::CommandBufferList() const { return m_CommandBufferList; }