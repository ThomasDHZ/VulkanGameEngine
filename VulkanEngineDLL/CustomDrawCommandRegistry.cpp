#include "CustomDrawCommandRegistry.h"
#include "RenderSystem.h"
#include "LightSystem.h"

CustomDrawCommandRegistry& customDrawCommandRegistry = CustomDrawCommandRegistry::Get();

void CustomDrawCommandRegistry::RegisterCustomDrawCommand(const String& sourceName, UpdateFunc func)
{
	_registry[sourceName] = std::move(func);
}

void CustomDrawCommandRegistry::ApplyCustomDrawCommand(const String& sourceName, VkCommandBuffer& commandBuffer, MeshDrawMessage& drawMeshMessage, const CustomDrawInitContext& context, VulkanDrawMessage& message, uint meshDrawIndex, uint mip, uint mipCount)
{
    auto it = _registry.find(sourceName);
    if (it != _registry.end())
    {
        it->second(commandBuffer, drawMeshMessage, context, message, meshDrawIndex, mip, mipCount);
    }
}

void CustomDrawCommandRegistry::RegisterDefaultCustomDrawCommand()
{
    static bool firstFrame = true;
    RegisterCustomDrawCommand("drawPointLightDepthViews", [&](VkCommandBuffer& commandBuffer, MeshDrawMessage& drawMeshMessage, const CustomDrawInitContext& context, VulkanDrawMessage& message, uint meshDrawIndex, uint mip, uint mipCount)
        {
            VulkanRenderPass renderPass = renderSystem.FindRenderPass(context.RenderPassId);
            Texture textureAttachment = renderSystem.FindRenderPassAttachment(context.TextureAttachmentId);
            uint32 tilesPerRow = textureAttachment.texture.TextureSize().x / context.TileSize;

            int x = 0;
            for (auto [entity, light] : gameObjectSystem.EntityRegistry.view<PointLightComponent>().each())
            {
       /*         if (!light.DirtyFlag) { x++; continue; }
                if (x > tilesPerRow * tilesPerRow) { x++; continue; }*/
                
                PointLight& pointLight = lightSystem.GetPointLight(light.PointLightMemoryPoolIndex);
                //if (pointLight.LightActive) { x++; continue; }

                VkViewport viewport
                {
                    .x = (x % tilesPerRow) * context.TileSize,
                    .y = (x / tilesPerRow) * context.TileSize,
                    .width = context.TileSize,
                    .height = context.TileSize,
                    .minDepth = 0.0f,
                    .maxDepth = 1.0f
                };

                VkRect2D rect2D = VkRect2D
                {
                   .offset = VkOffset2D {.x = static_cast<int>(viewport.x), .y = static_cast<int>(viewport.y) },
                   .extent = VkExtent2D {.width = static_cast<uint32>(context.TileSize), .height = static_cast<uint32>(context.TileSize) }
                };

                //if (firstFrame)
                //{
                //    VkClearDepthStencilValue clear{ 1.0f, 0 };
                //    VkImageSubresourceRange range{ VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 };
                //    vkCmdClearDepthStencilImage(commandBuffer, textureAttachment.texture.m_textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear, 1, &range);
                //    firstFrame = false;
                //}

                vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
                vkCmdSetScissor(commandBuffer, 0, 1, &rect2D);
                renderSystem.BindPushConstants(commandBuffer, message, meshDrawIndex, mip, mipCount, x);
                renderPass.DrawMesh(commandBuffer, drawMeshMessage);
                light.DirtyFlag = false;
                x++;
            }
        });

}
