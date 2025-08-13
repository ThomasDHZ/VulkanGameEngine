#include "JsonLoader.h"
#include "json.h"

RenderPassLoader JsonLoader_LoadRenderPassLoaderInfo(const char* renderPassLoaderJson, const ivec2& defaultRenderPassResoultion)
{
    using namespace nlohmann;
    RenderPassLoader renderPassLoader = {};
    try 
    {
        json json = Json::ReadJson(renderPassLoaderJson);

        json.at("RenderPassId").get_to(renderPassLoader.RenderPassId);
        json.at("IsRenderedToSwapchain").get_to(renderPassLoader.IsRenderedToSwapchain);
        json.at("RenderPipelineList").get_to(renderPassLoader.RenderPipelineList);
        json.at("RenderedTextureInfoModelList").get_to(renderPassLoader.RenderedTextureInfoModelList);
        json.at("SubpassDependencyList").get_to(renderPassLoader.SubpassDependencyModelList);
        json.at("ClearValueList").get_to(renderPassLoader.ClearValueList);
        json.at("RenderArea").get_to(renderPassLoader.RenderArea);
    }
    catch (const std::exception& e) 
    {
        std::cerr << "Error loading RenderPassLoader from " << renderPassLoaderJson << ": " << e.what() << std::endl;
        throw;
    }

    return renderPassLoader;
}