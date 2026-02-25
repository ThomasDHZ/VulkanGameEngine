#include "pch.h"
#include "LevelSystem.h"
#include "GameObjectSystem.h"
LevelSystem& levelSystem = LevelSystem::Get();

LevelLayer LevelSystem::LoadLevelInfo(VkGuid& levelId, const LevelTileSet& tileSet, uint* tileIdMap, size_t tileIdMapCount, ivec2& levelBounds, int levelLayerIndex)
{
    Vector<Tile> tileMap;
    Vector<uint32> indexList;
    Vector<Vertex2DLayout> vertexList;
    Vector<Tile> tileSetList = Vector<Tile>(tileSet.LevelTileListPtr, tileSet.LevelTileListPtr + tileSet.LevelTileCount);
    Vector<uint> tileIdMapList = Vector<uint>(tileIdMap, tileIdMap + tileIdMapCount);

    for (uint x = 0; x < levelBounds.x; x++)
    {
        for (uint y = 0; y < levelBounds.y; y++)
        {
            const uint& tileId = tileIdMapList[(y * levelBounds.x) + x];
            const Tile& tile = tileSetList[tileId];

            const float LeftSideUV = tile.TileUVOffset.x;
            const float RightSideUV = tile.TileUVOffset.x + tileSet.TileUVSize.x;
            const float TopSideUV = tile.TileUVOffset.y;
            const float BottomSideUV = tile.TileUVOffset.y + tileSet.TileUVSize.y;

            const uint VertexCount = vertexList.size();
            const vec2 TilePixelSize = tileSet.TilePixelSize * tileSet.TileScale;
            const Vertex2DLayout BottomLeftVertex = 
            {
                { x * TilePixelSize.x, y * TilePixelSize.y },
                { LeftSideUV, BottomSideUV }
            };
            const Vertex2DLayout BottomRightVertex = 
            {
                { (x * TilePixelSize.x) + TilePixelSize.x, y * TilePixelSize.y },
                { RightSideUV, BottomSideUV }
            };
            const Vertex2DLayout TopRightVertex = 
            {
                { (x * TilePixelSize.x) + TilePixelSize.x, (y * TilePixelSize.y) + TilePixelSize.y },
                { RightSideUV, TopSideUV }
            };
            const Vertex2DLayout TopLeftVertex = 
            {
                { x * TilePixelSize.x, (y * TilePixelSize.y) + TilePixelSize.y },
                { LeftSideUV, TopSideUV }
            };

            vertexList.emplace_back(BottomLeftVertex);
            vertexList.emplace_back(BottomRightVertex);
            vertexList.emplace_back(TopRightVertex);
            vertexList.emplace_back(TopLeftVertex);

            indexList.emplace_back(VertexCount + 0);  
            indexList.emplace_back(VertexCount + 1);  
            indexList.emplace_back(VertexCount + 2); 
            indexList.emplace_back(VertexCount + 2);  
            indexList.emplace_back(VertexCount + 3); 
            indexList.emplace_back(VertexCount + 0); 

            tileMap.emplace_back(tile);
        }
    }

    LevelLayer levelLayout = LevelLayer
    {
        .LevelId = levelId,
        .MaterialId = tileSet.MaterialId,
        .TileSetId = tileSet.TileSetId,
        .LevelLayerIndex = levelLayerIndex,
        .LevelBounds = levelBounds,
        .TileIdMap = memorySystem.AddPtrBuffer<uint>(tileIdMapList.size(), __FILE__, __LINE__, __func__),
        .TileMap = memorySystem.AddPtrBuffer<Tile>(tileMap.size(), __FILE__, __LINE__, __func__),
        .VertexList = memorySystem.AddPtrBuffer<Vertex2DLayout>(vertexList.size(), __FILE__, __LINE__, __func__),
        .IndexList = memorySystem.AddPtrBuffer<uint32>(indexList.size(), __FILE__, __LINE__, __func__),
        .TileIdMapCount = tileIdMapList.size(),
        .TileMapCount = tileMap.size(),
        .VertexListCount = vertexList.size(),
        .IndexListCount = indexList.size()
    };

    std::memcpy(levelLayout.TileMap, tileMap.data(), tileMap.size() * sizeof(Tile));
    std::memcpy(levelLayout.TileIdMap, tileIdMapList.data(), tileIdMapList.size() * sizeof(uint));
    std::memcpy(levelLayout.VertexList, vertexList.data(), vertexList.size() * sizeof(Vertex2DLayout));
    std::memcpy(levelLayout.IndexList, indexList.data(), indexList.size() * sizeof(uint32));

    return levelLayout;
}

void LevelSystem::DeleteLevel(uint* TileIdMap, Tile* TileMap, Vertex2DLayout* VertexList, uint32* IndexList)
{
    memorySystem.DeletePtr<uint>(TileIdMap);
    memorySystem.DeletePtr<Tile>(TileMap);
    memorySystem.DeletePtr<Vertex2DLayout>(VertexList);
    memorySystem.DeletePtr<uint32>(IndexList);
}

VkGuid LevelSystem::LoadTileSetVRAM(const char* tileSetPath)
{
    if (!tileSetPath)
    {
        return VkGuid();
    }

    auto json = fileSystem.LoadJsonFile(tileSetPath);
    VkGuid tileSetId = VkGuid(json["TileSetId"].get<String>().c_str());
    VkGuid materialId = VkGuid(json["MaterialId"].get<String>().c_str());

    if (LevelTileSetMap.find(tileSetId) != LevelTileSetMap.end())
    {
        return tileSetId;
    }

    const Material& material = materialSystem.FindMaterial(materialId);
    const Texture& tileSetTexture = textureSystem.FindTexture(material.AlbedoDataId);

    LevelTileSetMap[tileSetId] = vramSystem.LoadTileSetVRAM(tileSetPath, material, tileSetTexture);
    vramSystem.LoadTileSets(tileSetPath, LevelTileSetMap[tileSetId]);

    return tileSetId;
}

void LevelSystem::LoadLevelLayout(const char* levelLayoutPath)
{
    if (!levelLayoutPath)
    {
        return;
    }

    size_t levelLayerCount = 0;
    size_t levelLayerMapCount = 0;
    levelLayout = vramSystem.LoadLevelInfo(levelLayoutPath);
    LevelTileMapList = vramSystem.LoadLevelLayout(levelLayoutPath);
}

void LevelSystem::LoadLevelMesh(VkGuid& tileSetId)
{
    for (size_t x = 0; x < LevelTileMapList.size(); x++)
    {
        const LevelTileSet& levelTileSet = LevelTileSetMap[tileSetId];
        LevelLayerList.emplace_back(LoadLevelInfo(levelLayout.LevelLayoutId, levelTileSet, LevelTileMapList[x].data(), LevelTileMapList[x].size(), levelLayout.LevelBounds, x));

        Vector<Vertex2DLayout> vertexList(LevelLayerList[x].VertexList, LevelLayerList[x].VertexList + LevelLayerList[x].VertexListCount);
        Vector<uint> indexList(LevelLayerList[x].IndexList, LevelLayerList[x].IndexList + LevelLayerList[x].IndexListCount);

        VertexLayout vertexData =
        {
            .VertexType = VertexLayoutEnum::kVertexLayout_Vertex2D,
            .VertexDataSize = vertexList.size() * sizeof(Vertex2DLayout),
            .VertexData = vertexList.data()
        };
        meshSystem.CreateMesh("__LevelMesh__", MeshTypeEnum::kMesh_LevelMesh, vertexData, indexList, LevelLayerList[x].MaterialId);
    }
}

void LevelSystem::LoadSkyBox(const char* skyBoxMaterialPath)
{
    nlohmann::json json = fileSystem.LoadJsonFile(skyBoxMaterialPath);
    VkGuid skyBoxMaterialGuid = VkGuid(json["MaterialId"]);

    Vector<SkyboxVertexLayout> skyBoxVertices = 
    {
        {{-1.0f, -1.0f, -1.0f}},
        {{ 1.0f, -1.0f, -1.0f}},
        {{ 1.0f,  1.0f, -1.0f}},
        {{-1.0f,  1.0f, -1.0f}},
        {{-1.0f, -1.0f,  1.0f}},
        {{ 1.0f, -1.0f,  1.0f}},
        {{ 1.0f,  1.0f,  1.0f}},
        {{-1.0f,  1.0f,  1.0f}}
    };

    Vector<uint32> indexList = 
    {
        0, 2, 1,   0, 3, 2,
        4, 5, 6,   4, 6, 7,
        4, 3, 0,   4, 7, 3,
        1, 6, 5,   1, 2, 6,
        0, 5, 4,   0, 1, 5,
        3, 6, 2,   3, 7, 6
    };

    VertexLayout vertexData =
    {
        .VertexType = VertexLayoutEnum::kVertexLayout_Vertex2D,
        .VertexDataSize = skyBoxVertices.size() * sizeof(SkyboxVertexLayout),
        .VertexData = skyBoxVertices.data()
    };

    meshSystem.CreateMesh("__SkyBoxMesh__", MeshTypeEnum::kMesh_SkyBoxMesh, vertexData, indexList, skyBoxMaterialGuid);
}

void LevelSystem::DestroyLevel()
{
    spriteSystem.Destroy();
    for (auto& tileMap : LevelTileSetMap)
    {
        vramSystem.DeleteLevelVRAM(tileMap.second.LevelTileListPtr);
    }

    for (auto& levelLayer : LevelLayerList)
    {
        DeleteLevel(levelLayer.TileIdMap, levelLayer.TileMap, levelLayer.VertexList, levelLayer.IndexList);
    }
}

void LevelSystem::Update(const float& deltaTime)
{
    Camera_UpdateOrthographicPixelPerfect(*OrthographicCamera.get());
    Camera_PerspectiveUpdate(*PerspectiveCamera.get());
}

void LevelSystem::LoadLevel(const char* levelPath)
{
    OrthographicCamera = std::make_shared<Camera>(Camera_CreatePixelPerfectOrthographic(vec2((float)vulkanSystem.SwapChainResolution.width, (float)vulkanSystem.SwapChainResolution.height), vec2(0.0f, 0.0f)));
    PerspectiveCamera = std::make_shared<Camera>(Camera_PerspectiveCamera(vec2((float)vulkanSystem.SwapChainResolution.width, (float)vulkanSystem.SwapChainResolution.height), vec3(0.0f, 0.0f, 0.0f)));

    VkGuid dummyGuid = VkGuid();
    VkGuid tileSetId = VkGuid();

#if defined(_WIN32)
    shaderSystem.CompileShaders(configSystem.ShaderSourceDirectory.c_str(), configSystem.CompiledShaderOutputDirectory.c_str());
#endif
    nlohmann::json json = fileSystem.LoadJsonFile(levelPath);
    nlohmann::json shaderJson = fileSystem.LoadJsonFile("RenderPass/LevelShader2DRenderPass.json");
    nlohmann::json shaderWiredJson = fileSystem.LoadJsonFile("RenderPass/LevelShader2DWireFrameRenderPass.json");
    nlohmann::json shaderLightJson = fileSystem.LoadJsonFile("RenderPass/GBufferLightingRenderPass.json");
    // spriteRenderPass2DId = VkGuid(shaderJson["RenderPassId"].get<String>().c_str());
    levelWireFrameRenderPass2DId = VkGuid(shaderWiredJson["RenderPassId"].get<String>().c_str());
    shaderSystem.LoadShaderPipelineStructPrototypes(json["LoadRenderPasses"]);

    for (auto& texture     : json["LoadTextures"])    textureSystem.CreateTexture(texture);
    for (auto& ktxTexture  : json["LoadKTXTextures"]) textureSystem.LoadKTXTexture(ktxTexture);
    for (auto& material    : json["LoadMaterials"])   materialSystem.LoadMaterial(material);
    for (auto& spriteVRAM  : json["LoadSpriteVRAM"])  spriteSystem.LoadSpriteVRAM(spriteVRAM);
    for (auto& tileSetVRAM : json["LoadTileSetVRAM"]) tileSetId = LoadTileSetVRAM(tileSetVRAM.get<String>().c_str());
    for (auto& light       : json["LoadSceneLights"]) lightSystem.LoadSceneLights(light);
    for (auto& skyBox      : json["LoadSkyBox"])      LoadSkyBox(skyBox.get<String>().c_str());
    for (size_t x = 0; x < json["GameObjectList"].size(); x++)
    {
        String objectJson = json["GameObjectList"][x]["GameObjectPath"];
        vec2 positionOverride(json["GameObjectList"][x]["GameObjectPositionOverride"][0], json["GameObjectList"][x]["GameObjectPositionOverride"][1]);
        gameObjectSystem.CreateGameObject(objectJson, positionOverride);
    }
    LoadLevelLayout(json["LoadLevelLayout"].get<String>().c_str());
    LoadLevelMesh(tileSetId);

    VkGuid levelId = VkGuid(json["LevelID"].get<String>().c_str());
    brdfRenderPassId = renderSystem.LoadRenderPass(dummyGuid, "RenderPass/BRDFRenderPass.json");
    renderSystem.GenerateTexture(brdfRenderPassId);
    textureSystem.BRDFMap = textureSystem.FindRenderedTextureList(brdfRenderPassId).back();

    environmentToCubeMapRenderPassId   = renderSystem.LoadRenderPass(levelLayout.LevelLayoutId, "RenderPass/EnvironmentToCubeMapRenderPass.json");
    renderSystem.GenerateCubeMapTexture(environmentToCubeMapRenderPassId);
    textureSystem.CubeMap = textureSystem.FindRenderedTextureList(environmentToCubeMapRenderPassId).back();

    irradianceMapRenderPassId          = renderSystem.LoadRenderPass(levelLayout.LevelLayoutId, "RenderPass/IrradianceRenderPass.json");
    prefilterMapRenderPassId           = renderSystem.LoadRenderPass(levelLayout.LevelLayoutId, "RenderPass/PrefilterRenderPass.json");
    gBufferRenderPassId                = renderSystem.LoadRenderPass(levelLayout.LevelLayoutId, "RenderPass/GBufferRenderPass.json");
    /*verticalGaussianBlurRenderPassId = renderSystem.LoadRenderPass(dummyGuid,                 "RenderPass/VertGaussianBlurRenderPass.json");
    horizontalGaussianBlurRenderPassId = renderSystem.LoadRenderPass(dummyGuid,                 "RenderPass/HorizontalGaussianBlurRenderPass.json");
    bloomRenderPassId                  = renderSystem.LoadRenderPass(dummyGuid,                 "RenderPass/BloomRenderPass.json");*/
    hdrRenderPassId                    = renderSystem.LoadRenderPass(dummyGuid,                 "RenderPass/HdrRenderPass.json");
    frameBufferId                      = renderSystem.LoadRenderPass(dummyGuid,                 "RenderPass/FrameBufferRenderPass.json");
}


void LevelSystem::Draw(VkCommandBuffer& commandBuffer, const float& deltaTime)
{
    RenderIrradianceMapRenderPass(commandBuffer, irradianceMapRenderPassId, deltaTime);
    RenderPrefilterMapRenderPass(commandBuffer, prefilterMapRenderPassId, deltaTime);
    RenderGBuffer(commandBuffer, gBufferRenderPassId, levelLayout.LevelLayoutId, deltaTime);
    //RenderGaussianBlurPass(commandBuffer, verticalGaussianBlurRenderPassId, 0);
    //RenderGaussianBlurPass(commandBuffer, horizontalGaussianBlurRenderPassId, 1);
    //RenderBloomPass(commandBuffer, bloomRenderPassId);
    RenderHdrPass(commandBuffer, hdrRenderPassId);
    RenderFrameBuffer(commandBuffer, frameBufferId);
}

void LevelSystem::RenderEnvironmentToCubeMapRenderPass(VkCommandBuffer& commandBuffer, VkGuid& renderPassId)
{
    const VulkanRenderPass& renderPass = renderSystem.FindRenderPass(renderPassId);
    VulkanPipeline skyboxPipeline = renderSystem.FindRenderPipelineList(renderPassId)[0];
    const Vector<Mesh>& skyBoxList = meshSystem.FindMeshByMeshType(MeshTypeEnum::kMesh_SkyBoxMesh);
    ShaderPushConstantDLL& pushConstant = shaderSystem.FindShaderPushConstant("irradianceShaderConstants");

    VkViewport viewport
    {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(renderPass.RenderPassResolution.x),
        .height = static_cast<float>(renderPass.RenderPassResolution.y),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    VkRenderPassBeginInfo renderPassBeginInfo = VkRenderPassBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderPass.RenderPass,
        .framebuffer = renderPass.FrameBufferList.front(),
        .renderArea = VkRect2D
        {
           .offset = VkOffset2D {.x = 0, .y = 0 },
           .extent = VkExtent2D {.width = static_cast<uint>(renderPass.RenderPassResolution.x), .height = static_cast<uint>(renderPass.RenderPassResolution.y) }
        },
        .clearValueCount = static_cast<uint32>(renderPass.ClearValueList.size()),
        .pClearValues = renderPass.ClearValueList.data()
    };

    VkDeviceSize offsets[] = { 0 };
    VkDeviceSize instanceOffset[] = { 0 };
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &renderPassBeginInfo.renderArea);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, skyboxPipeline.Pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, skyboxPipeline.PipelineLayout, 0, skyboxPipeline.DescriptorSetList.size(), skyboxPipeline.DescriptorSetList.data(), 0, nullptr);
    for (auto& skyboxMesh : skyBoxList)
    {
        const MeshAssetData& meshAsset = meshSystem.FindMeshAssetData(skyboxMesh.SharedAssetId);
        const VkBuffer& meshVertexBuffer = bufferSystem.FindVulkanBuffer(meshAsset.VertexBufferId).Buffer;
        const VkBuffer& meshIndexBuffer = bufferSystem.FindVulkanBuffer(meshAsset.IndexBufferId).Buffer;

        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &meshVertexBuffer, offsets);
        vkCmdBindIndexBuffer(commandBuffer, meshIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, meshAsset.IndexCount, 1, 0, 0, 0);
    }
    vkCmdEndRenderPass(commandBuffer);
}

void LevelSystem::RenderGBuffer(VkCommandBuffer& commandBuffer, VkGuid& renderPassId, VkGuid& levelId, const float deltaTime)
{
    const VulkanRenderPass& renderPass = renderSystem.FindRenderPass(renderPassId);
    VulkanPipeline spritePipeline = renderSystem.FindRenderPipelineList(renderPassId)[0];
    VulkanPipeline levelPipeline = renderSystem.FindRenderPipelineList(renderPassId)[1];
    VulkanPipeline lightingPipeline = renderSystem.FindRenderPipelineList(renderPassId)[2];

    const Vector<Mesh>& levelLayerList = meshSystem.FindMeshByMeshType(MeshTypeEnum::kMesh_LevelMesh);

    VkRenderPassBeginInfo renderPassBeginInfo = VkRenderPassBeginInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderPass.RenderPass,
        .framebuffer = renderPass.FrameBufferList.front(),
        .renderArea = VkRect2D{
            .offset = VkOffset2D{0, 0},
            .extent = VkExtent2D{
                static_cast<uint>(renderPass.RenderPassResolution.x),
                static_cast<uint>(renderPass.RenderPassResolution.y)
            }
        },
        .clearValueCount = static_cast<uint32>(renderPass.ClearValueList.size()),
        .pClearValues = renderPass.ClearValueList.data()
    };

    ShaderPushConstantDLL& sceneDataPushConstant = shaderSystem.FindShaderPushConstant("sceneData");
    shaderSystem.UpdatePushConstantValue<int>(sceneDataPushConstant, "UseHeightMap", UseHeightMap);
    shaderSystem.UpdatePushConstantValue<float>(sceneDataPushConstant, "HeightScale", HeightScale);
    shaderSystem.UpdatePushConstantValue<vec3>(sceneDataPushConstant, "ViewDirection", ViewDirection);
    shaderSystem.UpdatePushConstantValue<mat4>(sceneDataPushConstant, "Projection", OrthographicCamera->ProjectionMatrix);
    shaderSystem.UpdatePushConstantValue<mat4>(sceneDataPushConstant, "View", OrthographicCamera->ViewMatrix);
    shaderSystem.UpdatePushConstantValue<vec3>(sceneDataPushConstant, "CameraPosition", OrthographicCamera->Position);

    ShaderPushConstantDLL& gBufferSceneDataBuffer = shaderSystem.FindShaderPushConstant("gBufferSceneDataBuffer");
    shaderSystem.UpdatePushConstantValue<int>(gBufferSceneDataBuffer, "Isolate", isolateLayer);
    shaderSystem.UpdatePushConstantValue<vec2>(gBufferSceneDataBuffer, "InvertResolution", vec2(1.0f / static_cast<float>(renderPass.RenderPassResolution.x), 1.0f / static_cast<float>(renderPass.RenderPassResolution.y)));
    shaderSystem.UpdatePushConstantValue<vec3>(gBufferSceneDataBuffer, "OrthographicCameraPosition", OrthographicCamera->Position);
    shaderSystem.UpdatePushConstantValue<vec3>(gBufferSceneDataBuffer, "PerspectiveViewDirection", ViewDirection);
    shaderSystem.UpdatePushConstantValue<uint>(gBufferSceneDataBuffer, "DirectionalLightCount", memoryPoolSystem.MemoryPoolSubBufferInfo(kDirectionalLightBuffer).ActiveCount);
    shaderSystem.UpdatePushConstantValue<uint>(gBufferSceneDataBuffer, "PointLightCount", memoryPoolSystem.MemoryPoolSubBufferInfo(kPointLightBuffer).ActiveCount);
    shaderSystem.UpdatePushConstantValue<mat4>(gBufferSceneDataBuffer, "InvProjection", glm::inverse(PerspectiveCamera->ProjectionMatrix));
    shaderSystem.UpdatePushConstantValue<mat4>(gBufferSceneDataBuffer, "InvView", glm::inverse(PerspectiveCamera->ViewMatrix));
    shaderSystem.UpdatePushConstantBuffer(gBufferSceneDataBuffer);

    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, levelPipeline.Pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, levelPipeline.PipelineLayout, 0, levelPipeline.DescriptorSetList.size(), levelPipeline.DescriptorSetList.data(), 0, nullptr);

    auto pipelineList = renderSystem.FindRenderPipelineList(renderPassId);
    spriteSystem.Update(deltaTime);
    meshSystem.Update(deltaTime, pipelineList);
    materialSystem.Update(deltaTime, pipelineList);
    lightSystem.Update(deltaTime, pipelineList);
    memoryPoolSystem.UpdateMemoryPool(pipelineList);

    VkDeviceSize offsets[] = { 0 };
    VkDeviceSize vertexOffset = 0;
    VkDeviceSize instanceOffset = 0;
    for (auto& levelLayer : levelLayerList)
    {
        const MeshAssetData& meshAsset = meshSystem.FindMeshAssetData(levelLayer.SharedAssetId);
        const VkBuffer& meshVertexBuffer = bufferSystem.FindVulkanBuffer(meshAsset.VertexBufferId).Buffer;
        const VkBuffer& meshIndexBuffer = bufferSystem.FindVulkanBuffer(meshAsset.IndexBufferId).Buffer;

        shaderSystem.UpdatePushConstantValue<uint>(sceneDataPushConstant, "MeshBufferIndex", levelLayer.MeshId);
        shaderSystem.UpdatePushConstantBuffer(sceneDataPushConstant);
        vkCmdPushConstants(commandBuffer, levelPipeline.PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sceneDataPushConstant.PushConstantSize, sceneDataPushConstant.PushConstantBuffer.data());
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &meshVertexBuffer, offsets);
        vkCmdBindIndexBuffer(commandBuffer, meshIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, meshAsset.IndexCount, 1, 0, 0, 0);
    }
    const Mesh& spriteMesh = meshSystem.FindMesh(spriteSystem.SpriteMeshId);
    const MeshAssetData& meshAsset = meshSystem.FindMeshAssetData(spriteMesh.SharedAssetId);
    const VkBuffer& quadIndexBuffer = bufferSystem.FindVulkanBuffer(meshAsset.IndexBufferId).Buffer;
    const VkBuffer& quadVertexBuffer = bufferSystem.FindVulkanBuffer(meshAsset.VertexBufferId).Buffer;
    const VkBuffer& instanceBuffer = bufferSystem.FindVulkanBuffer(memoryPoolSystem.GpuDataBufferIndex).Buffer;
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, spritePipeline.Pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, spritePipeline.PipelineLayout, 0, spritePipeline.DescriptorSetList.size(), spritePipeline.DescriptorSetList.data(), 0, nullptr);
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &quadVertexBuffer, &vertexOffset);
    vkCmdBindVertexBuffers(commandBuffer, 1, 1, &instanceBuffer, &instanceOffset);
    vkCmdBindIndexBuffer(commandBuffer, quadIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
    for (const auto& layer : spriteSystem.SpriteLayerList)
    {
        if (layer.InstanceCount == 0) continue;
        vkCmdDrawIndexed(commandBuffer, meshAsset.IndexCount, layer.InstanceCount, 0, 0,layer.StartInstanceIndex);
    }
    vkCmdNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdPushConstants(commandBuffer, lightingPipeline.PipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, gBufferSceneDataBuffer.PushConstantSize, gBufferSceneDataBuffer.PushConstantBuffer.data());
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, lightingPipeline.Pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, lightingPipeline.PipelineLayout, 0, lightingPipeline.DescriptorSetList.size(), lightingPipeline.DescriptorSetList.data(), 0, nullptr);
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    vkCmdEndRenderPass(commandBuffer);
}

void LevelSystem::RenderIrradianceMapRenderPass(VkCommandBuffer& commandBuffer, VkGuid& renderPassId, float deltaTime)
{
    const VulkanRenderPass& renderPass = renderSystem.FindRenderPass(renderPassId);
    VulkanPipeline skyboxPipeline = renderSystem.FindRenderPipelineList(renderPassId)[0];
    const Vector<Mesh>& skyBoxList = meshSystem.FindMeshByMeshType(MeshTypeEnum::kMesh_SkyBoxMesh);
    ShaderPushConstantDLL& pushConstant = shaderSystem.FindShaderPushConstant("irradianceShaderConstants");

    VkViewport viewport
    {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(renderPass.RenderPassResolution.x),
        .height = static_cast<float>(renderPass.RenderPassResolution.y),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    VkRenderPassBeginInfo renderPassBeginInfo = VkRenderPassBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderPass.RenderPass,
        .framebuffer = renderPass.FrameBufferList.front(),
        .renderArea = VkRect2D
        {
           .offset = VkOffset2D {.x = 0, .y = 0 },
           .extent = VkExtent2D {.width = static_cast<uint>(renderPass.RenderPassResolution.x), .height = static_cast<uint>(renderPass.RenderPassResolution.y) }
        },
        .clearValueCount = static_cast<uint32>(renderPass.ClearValueList.size()),
        .pClearValues = renderPass.ClearValueList.data()
    };

    VkDeviceSize offsets[] = { 0 };
    VkDeviceSize instanceOffset[] = { 0 };
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &renderPassBeginInfo.renderArea);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, skyboxPipeline.Pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, skyboxPipeline.PipelineLayout, 0, skyboxPipeline.DescriptorSetList.size(), skyboxPipeline.DescriptorSetList.data(), 0, nullptr);
    for (auto& skyboxMesh : skyBoxList)
    {
        const MeshAssetData& meshAsset = meshSystem.FindMeshAssetData(skyboxMesh.SharedAssetId);
        const VkBuffer& meshVertexBuffer = bufferSystem.FindVulkanBuffer(meshAsset.VertexBufferId).Buffer;
        const VkBuffer& meshIndexBuffer = bufferSystem.FindVulkanBuffer(meshAsset.IndexBufferId).Buffer;

        shaderSystem.UpdatePushConstantValue<float>(pushConstant, "sampleDelta", 0.1f);
        shaderSystem.UpdatePushConstantBuffer(pushConstant);

        vkCmdPushConstants(commandBuffer, skyboxPipeline.PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, pushConstant.PushConstantSize, pushConstant.PushConstantBuffer.data());
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &meshVertexBuffer, offsets);
        vkCmdBindIndexBuffer(commandBuffer, meshIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, meshAsset.IndexCount, 1, 0, 0, 0);
    }
    vkCmdEndRenderPass(commandBuffer);
}

void LevelSystem::RenderPrefilterMapRenderPass(VkCommandBuffer& commandBuffer, VkGuid& renderPassId, float deltaTime)
{
    VulkanRenderPass renderPass = renderSystem.FindRenderPass(renderPassId);
    VulkanPipeline skyboxPipeline = renderSystem.FindRenderPipelineList(renderPassId)[0];
    const Vector<Mesh>& skyBoxList = meshSystem.FindMeshByMeshType(MeshTypeEnum::kMesh_SkyBoxMesh);
    ShaderPushConstantDLL& pushConstant = shaderSystem.FindShaderPushConstant("prefilterSamplerProperties");

    const Mesh& skyboxMesh = skyBoxList[0];
    const MeshAssetData& meshAsset = meshSystem.FindMeshAssetData(skyboxMesh.SharedAssetId);
    const VkBuffer& meshVertexBuffer = bufferSystem.FindVulkanBuffer(meshAsset.VertexBufferId).Buffer;
    const VkBuffer& meshIndexBuffer = bufferSystem.FindVulkanBuffer(meshAsset.IndexBufferId).Buffer;

    VkDeviceSize offsets[] = { 0 };
    uint32 baseSize = renderPass.RenderPassResolution.x;
    uint32 prefilterMipmapCount = textureSystem.PrefilterCubeMap.mipMapLevels;
    for (uint32 mip = 0; mip < prefilterMipmapCount; ++mip)
    {
        uint32 mipWidth = baseSize >> mip;
        uint32 mipHeight = baseSize >> mip;
        float roughness = static_cast<float>(mip) / static_cast<float>(prefilterMipmapCount - 1);

        shaderSystem.UpdatePushConstantValue<uint>(pushConstant, "CubeMapResolution", baseSize);
        shaderSystem.UpdatePushConstantValue<float>(pushConstant, "Roughness", roughness);
        shaderSystem.UpdatePushConstantBuffer(pushConstant);

        VkViewport viewport =
        {
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(mipWidth),
            .height = static_cast<float>(mipHeight),
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };
        
        VkRect2D scissor =
        {
            .offset = { 0, 0 },
            .extent = { mipWidth, mipHeight },
        };

        VkRenderPassBeginInfo renderPassBeginInfo =
        {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = renderPass.RenderPass,
            .framebuffer = renderPass.FrameBufferList[mip],
            .renderArea = scissor,
            .clearValueCount = static_cast<uint32>(renderPass.ClearValueList.size()),
            .pClearValues = renderPass.ClearValueList.data(),
        };

        vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdPushConstants(commandBuffer, skyboxPipeline.PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, pushConstant.PushConstantSize, pushConstant.PushConstantBuffer.data());
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, skyboxPipeline.Pipeline);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, skyboxPipeline.PipelineLayout, 0, static_cast<uint32>(skyboxPipeline.DescriptorSetList.size()), skyboxPipeline.DescriptorSetList.data(), 0, nullptr);
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &meshVertexBuffer, offsets);
        vkCmdBindIndexBuffer(commandBuffer, meshIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
        vkCmdDrawIndexed(commandBuffer, meshAsset.IndexCount, 1, 0, 0, 0);
        vkCmdEndRenderPass(commandBuffer);
    }
}

void LevelSystem::RenderGaussianBlurPass(VkCommandBuffer& commandBuffer, VkGuid& renderPassId, uint blurDirection)
{
    const VulkanRenderPass renderPass = renderSystem.FindRenderPass(renderPassId);
    const VulkanPipeline  pipeline = renderSystem.FindRenderPipelineList(renderPassId)[0];

    ShaderPushConstantDLL& pushConstant = shaderSystem.FindShaderPushConstant("bloomSettings");
    shaderSystem.UpdatePushConstantValue<uint>(pushConstant, "blurDirection", blurDirection);
    shaderSystem.UpdatePushConstantValue<float>(pushConstant, "blurScale", 1.0f);
    shaderSystem.UpdatePushConstantValue<float>(pushConstant, "blurStrength", 0.04f);

    VkViewport viewport
    {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(renderPass.RenderPassResolution.x),
        .height = static_cast<float>(renderPass.RenderPassResolution.y),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    VkRenderPassBeginInfo renderPassBeginInfo = VkRenderPassBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderPass.RenderPass,
        .framebuffer = renderPass.FrameBufferList.front(),
        .renderArea = VkRect2D
        {
           .offset = VkOffset2D {.x = 0, .y = 0 },
           .extent = VkExtent2D {.width = static_cast<uint>(renderPass.RenderPassResolution.x), .height = static_cast<uint>(renderPass.RenderPassResolution.y) }
        },
        .clearValueCount = static_cast<uint32>(renderPass.ClearValueList.size()),
        .pClearValues = renderPass.ClearValueList.data()
    };

    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdPushConstants(commandBuffer, pipeline.PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, pushConstant.PushConstantSize, pushConstant.PushConstantBuffer.data());
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &renderPassBeginInfo.renderArea);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.Pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.PipelineLayout, 0, pipeline.DescriptorSetList.size(), pipeline.DescriptorSetList.data(), 0, nullptr);
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    vkCmdEndRenderPass(commandBuffer);
}

void LevelSystem::RenderHdrPass(VkCommandBuffer& commandBuffer, VkGuid& renderPassId)
{
    const VulkanRenderPass renderPass = renderSystem.FindRenderPass(renderPassId);
    VulkanPipeline pipeline = renderSystem.FindRenderPipelineList(renderPassId)[0];
    Vector<Texture> renderPassTexture = textureSystem.FindRenderedTextureList(renderPassId);

    VkViewport viewport
    {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(renderPass.RenderPassResolution.x),
        .height = static_cast<float>(renderPass.RenderPassResolution.y),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    VkRenderPassBeginInfo renderPassBeginInfo = VkRenderPassBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderPass.RenderPass,
        .framebuffer = renderPass.FrameBufferList.front(),
        .renderArea = VkRect2D
        {
           .offset = VkOffset2D {.x = 0, .y = 0 },
           .extent = VkExtent2D {.width = static_cast<uint>(renderPass.RenderPassResolution.x), .height = static_cast<uint>(renderPass.RenderPassResolution.y) }
        },
        .clearValueCount = static_cast<uint32>(renderPass.ClearValueList.size()),
        .pClearValues = renderPass.ClearValueList.data()
    };

    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &renderPassBeginInfo.renderArea);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.Pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.PipelineLayout, 0, pipeline.DescriptorSetList.size(), pipeline.DescriptorSetList.data(), 0, nullptr);
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    vkCmdEndRenderPass(commandBuffer);
}

void LevelSystem::RenderBloomPass(VkCommandBuffer& commandBuffer, VkGuid& renderPassId)
{
    const VulkanRenderPass renderPass = renderSystem.FindRenderPass(renderPassId);
    const VulkanPipeline pipeline = renderSystem.FindRenderPipelineList(renderPassId)[0];

    VkRenderPassBeginInfo renderPassBeginInfo = VkRenderPassBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderPass.RenderPass,
        .framebuffer = renderPass.FrameBufferList.front(),
        .renderArea = VkRect2D
        {
           .offset = VkOffset2D {.x = 0, .y = 0 },
           .extent = VkExtent2D {.width = static_cast<uint>(renderPass.RenderPassResolution.x), .height = static_cast<uint>(renderPass.RenderPassResolution.y) }
        },
        .clearValueCount = static_cast<uint32>(renderPass.ClearValueList.size()),
        .pClearValues = renderPass.ClearValueList.data()
    };

    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.Pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.PipelineLayout, 0, pipeline.DescriptorSetList.size(), pipeline.DescriptorSetList.data(), 0, nullptr);
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    vkCmdEndRenderPass(commandBuffer);
}

void LevelSystem::RenderFrameBuffer(VkCommandBuffer& commandBuffer, VkGuid& renderPassId)
{
    const VulkanRenderPass renderPass = renderSystem.FindRenderPass(renderPassId);
    VulkanPipeline pipeline = renderSystem.FindRenderPipelineList(renderPassId)[0];

    VkRenderPassBeginInfo renderPassBeginInfo = VkRenderPassBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderPass.RenderPass,
        .framebuffer = renderPass.FrameBufferList[vulkanSystem.ImageIndex],
        .renderArea = VkRect2D
        {
           .offset = VkOffset2D {.x = 0, .y = 0 },
           .extent = VkExtent2D {.width = static_cast<uint>(renderPass.RenderPassResolution.x), .height = static_cast<uint>(renderPass.RenderPassResolution.y) }
        },
        .clearValueCount = static_cast<uint32>(renderPass.ClearValueList.size()),
        .pClearValues = renderPass.ClearValueList.data()
    };

    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.Pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.PipelineLayout, 0, pipeline.DescriptorSetList.size(), pipeline.DescriptorSetList.data(), 0, nullptr);
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    vkCmdEndRenderPass(commandBuffer);
}

void LevelSystem::RenderShadowDebug(VkCommandBuffer& commandBuffer, VkGuid& renderPassId)
{
    const VulkanRenderPass renderPass = renderSystem.FindRenderPass(renderPassId);
    VulkanPipeline pipeline = renderSystem.FindRenderPipelineList(renderPassId)[0];
    Vector<Texture> renderPassTexture = textureSystem.FindRenderedTextureList(renderPassId);

    VkViewport viewport
    {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(renderPass.RenderPassResolution.x),
        .height = static_cast<float>(renderPass.RenderPassResolution.y),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    VkRenderPassBeginInfo renderPassBeginInfo = VkRenderPassBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderPass.RenderPass,
        .framebuffer = renderPass.FrameBufferList.front(),
        .renderArea = VkRect2D
        {
           .offset = VkOffset2D {.x = 0, .y = 0 },
           .extent = VkExtent2D {.width = static_cast<uint>(renderPass.RenderPassResolution.x), .height = static_cast<uint>(renderPass.RenderPassResolution.y) }
        },
        .clearValueCount = static_cast<uint32>(renderPass.ClearValueList.size()),
        .pClearValues = renderPass.ClearValueList.data()
    };

    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &renderPassBeginInfo.renderArea);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.Pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.PipelineLayout, 0, pipeline.DescriptorSetList.size(), pipeline.DescriptorSetList.data(), 0, nullptr);
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    vkCmdEndRenderPass(commandBuffer);
}

LevelLayout LevelSystem::GetLevelLayout()
{
    return levelLayout;
}

Vector<LevelLayer> LevelSystem::GetLevelLayerList()
{
    return LevelLayerList;
}

Vector<Vector<uint>> LevelSystem::GetLevelTileMapList()
{
    return LevelTileMapList;
}

Vector<LevelTileSet> LevelSystem::GetLevelTileSetList()
{
    Vector<LevelTileSet> levelTileSetList;
    for (auto& levelTile : LevelTileSetMap)
    {
        levelTileSetList.push_back(levelTile.second);
    }
    return levelTileSetList;
}