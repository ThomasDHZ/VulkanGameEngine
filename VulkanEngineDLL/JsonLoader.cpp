#include "JsonLoader.h"
#include "json.h"

RenderPassLoader JsonLoader_LoadRenderPassLoaderInfo(const char* renderPassLoaderJson, const ivec2& defaultRenderPassResoultion)
{
    RenderPassLoader renderPassLoader = {};
    try 
    {
        nlohmann::json j = Json::ReadJson(renderPassLoaderJson);

        j.at("RenderPassId").get_to(renderPassLoader.RenderPassId);
        j.at("IsRenderedToSwapchain").get_to(renderPassLoader.IsRenderedToSwapchain);
        j.at("RenderPipelineList").get_to(renderPassLoader.RenderPipelineList);
        j.at("RenderedTextureInfoModelList").get_to(renderPassLoader.RenderedTextureInfoModelList);
        j.at("SubpassDependencyList").get_to(renderPassLoader.SubpassDependencyModelList);
        j.at("ClearValueList").get_to(renderPassLoader.ClearValueList);
        if (j.contains("InputTextureList"))
        {
            for (int x = 0; x < j.at("InputTextureList").size(); x++)
            {
                renderPassLoader.InputTextureList.emplace_back(VkGuid(j["InputTextureList"][x].get<String>().c_str()));
            }
        }
        if (j.contains("RenderArea"))
        {
            j.at("RenderArea").get_to(renderPassLoader.RenderArea);
            renderPassLoader.RenderArea.RenderArea.extent.width = defaultRenderPassResoultion.x;
            renderPassLoader.RenderArea.RenderArea.extent.height = defaultRenderPassResoultion.y;
            for (auto& renderTexture : renderPassLoader.RenderedTextureInfoModelList)
            {
                renderTexture.ImageCreateInfo.extent.width = defaultRenderPassResoultion.x;
                renderTexture.ImageCreateInfo.extent.height = defaultRenderPassResoultion.y;
            }
        }
    }
    catch (const std::exception& e) 
    {
        std::cerr << "Error loading RenderPassLoader from " << renderPassLoaderJson << ": " << e.what() << std::endl;
        throw;
    }

    return renderPassLoader;
}

RenderPipelineLoader JsonLoader_LoadRenderPipelineLoaderInfo(const char* renderPassLoaderJson, const ivec2& defaultRenderPassResoultion)
{
    RenderPipelineLoader renderPipelineLoader = {};
    try
    {
        nlohmann::json j = Json::ReadJson(renderPassLoaderJson);

        j.at("PipelineId").get_to(renderPipelineLoader.PipelineId);
        j.at("VertexShader").get_to(renderPipelineLoader.VertexShaderPath);
        j.at("FragmentShader").get_to(renderPipelineLoader.FragmentShaderPath);
        j.at("VertexType").get_to(renderPipelineLoader.VertexType);
        renderPipelineLoader.PipelineRasterizationStateCreateInfo = j.at("PipelineRasterizationStateCreateInfo");
        renderPipelineLoader.PipelineMultisampleStateCreateInfo = j.at("PipelineMultisampleStateCreateInfo");
        renderPipelineLoader.PipelineDepthStencilStateCreateInfo = j.at("PipelineDepthStencilStateCreateInfo");
        renderPipelineLoader.PipelineInputAssemblyStateCreateInfo = j.at("PipelineInputAssemblyStateCreateInfo");

        for (int x = 0; x < j.at("PipelineColorBlendAttachmentStateList").size(); x++)
        {
            renderPipelineLoader.PipelineColorBlendAttachmentStateList.emplace_back(j.at("PipelineColorBlendAttachmentStateList")[x]);
        }
        renderPipelineLoader.PipelineColorBlendStateCreateInfoModel = j.at("PipelineColorBlendStateCreateInfoModel");

        for (int x = 0; x < j.at("PipelineDescriptorModelsList").size(); x++)
        {
            renderPipelineLoader.PipelineDescriptorModelsList.emplace_back(j.at("PipelineDescriptorModelsList")[x]);
        }
        if (j.contains("ViewportList"))
        {
            for (int x = 0; x < j.at("ViewportList").size(); x++)
            {
                renderPipelineLoader.ViewportList.emplace_back(j.at("ViewportList")[x]);
            }
        }
        if (j.contains("ScissorList"))
        {
            for (int x = 0; x < j.at("ScissorList").size(); x++)
            {
                renderPipelineLoader.ScissorList.emplace_back(j.at("ScissorList")[x]);
            }
        }
        for (int x = 0; x < j.at("VertexInputBindingDescriptionList").size(); x++)
        {
            renderPipelineLoader.VertexInputBindingDescriptionList.emplace_back(
                renderPipelineLoader.VertexInputBindingDescriptionList.emplace_back(j.at("VertexInputBindingDescriptionList")[x]));
        }
        for (int x = 0; x < j.at("VertexInputAttributeDescriptionList").size(); x++)
        {
            renderPipelineLoader.VertexInputAttributeDescriptionList.emplace_back(j.at("VertexInputAttributeDescriptionList")[x]);
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error loading RenderPassLoader from " << renderPassLoaderJson << ": " << e.what() << std::endl;
        throw;
    }

    return renderPipelineLoader;
}