#include "pch.h"
#include "LevelSystem.h"
#include "GameObjectSystem.h"
#include <CameraSystem.h>

LevelSystem& levelSystem = LevelSystem::Get();

void LevelSystem::LoadLevel(const char* levelPath)
{
    cameraSystem.CreateCamera(CameraTypeEnum::kPixelPerfectOrthographicCam, vec2((float)vulkanSystem.DefaultRenderPassResolution.x, (float)vulkanSystem.DefaultRenderPassResolution.y), vec2(0.0f, 0.0f));
    PerspectiveCamera = std::make_shared<Camera>(Camera_PerspectiveCamera(vec2((float)vulkanSystem.DefaultRenderPassResolution.x, (float)vulkanSystem.DefaultRenderPassResolution.y), vec3(0.0f, 0.0f, 0.0f)));

    VkGuid dummyGuid = VkGuid();
    VkGuid tileSetId = VkGuid();

#if defined(_WIN32)
    shaderSystem.CompileShaders(configSystem.ShaderSourceDirectory.c_str(), configSystem.CompiledShaderOutputDirectory.c_str());
#endif

    nlohmann::json json = fileSystem.LoadJsonFile(levelPath);
    shaderSystem.LoadShaderPipelineStructPrototypes(json["LoadRenderPasses"]);
    for (auto& texture     : json["LoadTextures"])    textureSystem.CreateTexture(texture);
    for (auto& ktxTexture  : json["LoadKTXTextures"]) textureSystem.LoadKTXTexture(ktxTexture);
    for (auto& material    : json["LoadMaterials"])   materialSystem.LoadMaterial(material);
    for (auto& spriteVRAM  : json["LoadSpriteVRAM"])  spriteSystem.LoadSpriteVRAM(spriteVRAM);
    for (auto& tileSetVRAM : json["LoadTileSetVRAM"]) tileSetId = LoadTileSetVRAM(tileSetVRAM.get<String>().c_str());
    for (auto& light : json["LoadSceneLights"])
    {
        nlohmann::json lightJson = fileSystem.LoadJsonFile(light);
        String s = lightJson.dump();
        nlohmann::json json = fileSystem.LoadJsonFile("GameObjects/LightGameObject.json");

        uint lightType = lightJson["LightType"];
        vec3 pos = vec3(0.0f);
        if (lightType == kPointLight)
        {
            pos = vec3(lightJson["LightPosition"][0], lightJson["LightPosition"][1], lightJson["LightPosition"][2]);
        }
        uint lightId = lightSystem.LoadLight(light);
        uint gameObjectId = gameObjectSystem.CreateGameObject("GameObjects/LightGameObject.json", pos);

        if (lightType == kPointLight)
        {
            PointLightComponent pointLightComponent = PointLightComponent
            {
                .GameObjectId = gameObjectId,
                .PointLightId = lightId
            };
            CreateGameObjectComponent<PointLightComponent>(gameObjectId, &pointLightComponent);
        }
        else
        {
            DirectionalLightComponent directionalLightComponent = DirectionalLightComponent
            {
                .GameObjectId = gameObjectId,
                .DirectionalLightId = lightId
            };
            CreateGameObjectComponent<DirectionalLightComponent>(gameObjectId, &directionalLightComponent);
        }
    }
    for (size_t x = 0; x <   json["GameObjectList"].size(); x++)
    {
        String objectJson = json["GameObjectList"][x]["GameObjectPath"];
        vec2 positionOverride(json["GameObjectList"][x]["GameObjectPositionOverride"][0], json["GameObjectList"][x]["GameObjectPositionOverride"][1]);
        gameObjectSystem.CreateGameObject(objectJson, positionOverride);
    }
    LoadSkyBox();
    LoadLevelLayout(json["LoadLevelLayout"].get<String>().c_str());
    LoadLevelMesh(tileSetId);

    SceneDataBuffer& sceneDataBuffer = memoryPoolSystem.UpdateSceneDataBuffer();

    brdfRenderPassId = renderSystem.LoadRenderPass(dummyGuid, "RenderPass/BRDFRenderPass.json");
    textureSystem.GenerateTexture(brdfRenderPassId);
    Vector<Texture> textures = textureSystem.RenderedTextureListMap[brdfRenderPassId];

    environmentToCubeMapRenderPassId = renderSystem.LoadRenderPass(levelLayout.LevelLayoutId, "RenderPass/EnvironmentToCubeMapRenderPass.json");
    textureSystem.GenerateCubeMapTexture(environmentToCubeMapRenderPassId);

    irradianceMapRenderPassId          = renderSystem.LoadRenderPass(levelLayout.LevelLayoutId, "RenderPass/IrradianceRenderPass.json");
    prefilterMapRenderPassId           = renderSystem.LoadRenderPass(levelLayout.LevelLayoutId, "RenderPass/PrefilterRenderPass.json");
    gBufferRenderPassId                = renderSystem.LoadRenderPass(levelLayout.LevelLayoutId, "RenderPass/GBufferRenderPass.json");
    /*verticalGaussianBlurRenderPassId = renderSystem.LoadRenderPass(dummyGuid,                 "RenderPass/VertGaussianBlurRenderPass.json");
    horizontalGaussianBlurRenderPassId = renderSystem.LoadRenderPass(dummyGuid,                 "RenderPass/HorizontalGaussianBlurRenderPass.json");
    bloomRenderPassId                  = renderSystem.LoadRenderPass(dummyGuid,                 "RenderPass/BloomRenderPass.json");*/
    hdrRenderPassId                    = renderSystem.LoadRenderPass(dummyGuid,                 "RenderPass/HdrRenderPass.json");
    objectPickerRenderPassId           = renderSystem.LoadRenderPass(dummyGuid,                 "RenderPass/ObjectPickerRenderPass.json");
    selectedObjectPickerRenderPassId   = renderSystem.LoadRenderPass(dummyGuid,                 "RenderPass/SelectedGameObjectPickerRenderPass.json");
    frameBufferId                      = renderSystem.LoadRenderPass(dummyGuid,                 "RenderPass/FrameBufferRenderPass.json");

    sceneDataBuffer.HDRMapIndex = textureSystem.FindRenderedTextureList(gBufferRenderPassId).back().bindlessTextureIndex - 1;
    sceneDataBuffer.FrameBufferIndex = textureSystem.FindRenderedTextureList(hdrRenderPassId).back().bindlessTextureIndex;
}

void LevelSystem::Update(const float& deltaTime)
{
   Camera_PerspectiveUpdate(*PerspectiveCamera.get());

    SceneDataBuffer& sceneDataBuffer = memoryPoolSystem.UpdateSceneDataBuffer();
    sceneDataBuffer.Projection = cameraSystem.CameraList[cameraSystem.ActiveCameraIndex].ProjectionMatrix;
    sceneDataBuffer.View = cameraSystem.CameraList[cameraSystem.ActiveCameraIndex].ViewMatrix;
    sceneDataBuffer.InverseProjection = glm::inverse(PerspectiveCamera->ProjectionMatrix);
    sceneDataBuffer.InverseView = glm::inverse(PerspectiveCamera->ViewMatrix);
    sceneDataBuffer.CameraPosition = cameraSystem.CameraList[cameraSystem.ActiveCameraIndex].Position;
    sceneDataBuffer.ViewDirection = ViewDirection;
    cameraSystem.Update();
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
    RenderGameObjectPickerRenderPass(commandBuffer, objectPickerRenderPassId);
    RenderSelectedGameObjectPickerRenderPass(commandBuffer, selectedObjectPickerRenderPassId);
    RenderFrameBuffer(commandBuffer, frameBufferId);
}

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

    return LevelLayer
    {
        .LevelId = levelId,
        .MaterialId = tileSet.MaterialId,
        .TileSetId = tileSet.TileSetId,
        .LevelLayerIndex = levelLayerIndex,
        .LevelBounds = levelBounds,
        .TileIdMap = tileIdMapList,
        .TileMap = tileMap,
        .VertexList = vertexList,
        .IndexList = indexList,
    };
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

    LevelTileSetMap[tileSetId] = LoadTileSetVRAM(tileSetPath, material, tileSetTexture);
    LoadTileSets(tileSetPath, LevelTileSetMap[tileSetId]);

    return tileSetId;
}

void LevelSystem::LoadLevelLayout(const char* levelLayoutPath)
{
    if (!levelLayoutPath)
    {
        return;
    }

    levelLayout = LoadLevelInfo(levelLayoutPath);

    Vector<Vector<uint>> levelLayerList;
    nlohmann::json json = fileSystem.LoadJsonFile(levelLayoutPath);
    for (int x = 0; x < json["LevelLayouts"].size(); x++)
    {
        Vector<uint> levelLayerMap;
        for (int y = 0; y < json["LevelLayouts"][x].size(); y++)
        {
            for (int z = 0; z < json["LevelLayouts"][x][y].size(); z++)
            {
                levelLayerMap.push_back(json["LevelLayouts"][x][y][z]);
            }
        }
        levelLayerList.push_back(levelLayerMap);
    }
    LevelTileMapList = levelLayerList;
}

void LevelSystem::LoadLevelMesh(VkGuid& tileSetId)
{
    for (size_t x = 0; x < LevelTileMapList.size(); x++)
    {
        const LevelTileSet& levelTileSet = LevelTileSetMap[tileSetId];
        LevelLayerList.emplace_back(LoadLevelInfo(levelLayout.LevelLayoutId, levelTileSet, LevelTileMapList[x].data(), LevelTileMapList[x].size(), levelLayout.LevelBounds, x));

        VertexLayout vertexData =
        {
            .VertexType = VertexLayoutEnum::kVertexLayout_Vertex2D,
            .VertexDataSize = LevelLayerList[x].VertexList.size() * sizeof(Vertex2DLayout),
            .VertexData = LevelLayerList[x].VertexList.data()
        };
        meshSystem.CreateMesh("__LevelMesh__", MeshTypeEnum::kMesh_LevelMesh, vertexData, LevelLayerList[x].IndexList, LevelLayerList[x].MaterialId);
    }
}

void LevelSystem::LoadSkyBox()
{
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

    meshSystem.CreateMesh("__SkyBoxMesh__", MeshTypeEnum::kMesh_SkyBoxMesh, vertexData, indexList, VkGuid());
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

    SceneDataBuffer& sceneDataBuffer = memoryPoolSystem.UpdateSceneDataBuffer();
    sceneDataBuffer.InvertResolution = vec2(1.0f / static_cast<float>(renderPass.RenderPassResolution.x), 1.0f / static_cast<float>(renderPass.RenderPassResolution.y));

    ShaderPushConstant& sceneDataPushConstant = shaderSystem.FindShaderPushConstant("sceneData");
    shaderSystem.UpdatePushConstantValue<int>(sceneDataPushConstant, "UseHeightMap", UseHeightMap);
    shaderSystem.UpdatePushConstantValue<float>(sceneDataPushConstant, "HeightScale", HeightScale);


    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, levelPipeline.Pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, levelPipeline.PipelineLayout, 0, levelPipeline.DescriptorSetList.size(), levelPipeline.DescriptorSetList.data(), 0, nullptr);

    auto pipelineList = renderSystem.FindRenderPipelineList(renderPassId);
    memoryPoolSystem.UpdateMemoryPool(pipelineList);

    VkDeviceSize offsets[] = { 0 };
    VkDeviceSize vertexOffset = 0;
    VkDeviceSize instanceOffset = memoryPoolSystem.MemoryPoolSubBufferInfo(kSpriteInstanceBuffer).Offset;
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
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, spritePipeline.Pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, spritePipeline.PipelineLayout, 0, spritePipeline.DescriptorSetList.size(), spritePipeline.DescriptorSetList.data(), 0, nullptr);
    for (const auto& layer : spriteSystem.SpriteLayerList)
    {
        if (layer.InstanceCount == 0) continue;

        const Mesh& spriteMesh = meshSystem.FindMesh(spriteSystem.SpriteMeshId);
        const MeshAssetData& meshAsset = meshSystem.FindMeshAssetData(spriteMesh.SharedAssetId);
        const VkBuffer& indexBuffer = bufferSystem.FindVulkanBuffer(meshAsset.IndexBufferId).Buffer;
        const VulkanBuffer& instanceBuffer = bufferSystem.FindVulkanBuffer(memoryPoolSystem.GpuDataBufferIndex);
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &instanceBuffer.Buffer, &memoryPoolSystem.GpuDataMemoryPoolHeader.SpriteInstanceOffset);
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, meshAsset.IndexCount, layer.InstanceCount, 0, 0,layer.StartInstanceIndex);
    }
    vkCmdNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);
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
    ShaderPushConstant& pushConstant = shaderSystem.FindShaderPushConstant("irradianceShaderConstants");

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
    for (auto skyboxMesh : skyBoxList)
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
    ShaderPushConstant& pushConstant = shaderSystem.FindShaderPushConstant("prefilterSamplerProperties");

    const Mesh& skyboxMesh = skyBoxList[0];
    const MeshAssetData& meshAsset = meshSystem.FindMeshAssetData(skyboxMesh.SharedAssetId);
    const VkBuffer& meshVertexBuffer = bufferSystem.FindVulkanBuffer(meshAsset.VertexBufferId).Buffer;
    const VkBuffer& meshIndexBuffer = bufferSystem.FindVulkanBuffer(meshAsset.IndexBufferId).Buffer;
    const SceneDataBuffer& sceneDataBuffer = memoryPoolSystem.UpdateSceneDataBuffer();
    
    auto a = textureSystem.CubeMapTextureList;

    VkDeviceSize offsets[] = { 0 };
    uint32 baseSize = renderPass.RenderPassResolution.x;
    uint32 prefilterMipmapCount = textureSystem.CubeMapTextureList[sceneDataBuffer.PrefilterMapId].mipMapLevels;
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

    ShaderPushConstant& pushConstant = shaderSystem.FindShaderPushConstant("bloomSettings");
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
    const Texture& srcTexture = textureSystem.FindRenderedTextureList(hdrRenderPassId).back();

    VkImageBlit blitRegion
    {
        .srcSubresource =
        {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1
        },
        .srcOffsets =
        {
            VkOffset3D
            {
                .x = 0,
                .y = 0,
                .z = 0
            },
            VkOffset3D
            {
                .x = srcTexture.width,
                .y = srcTexture.height,
                .z = 1
            }
        },
        .dstSubresource =
        {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1
        },
        .dstOffsets =
        {
            VkOffset3D
            {
                .x = 0,
                .y = 0,
                .z = 0
            },
            VkOffset3D
            {
                .x = static_cast<int>(vulkanSystem.SwapChainResolution.width),
                .y = static_cast<int>(vulkanSystem.SwapChainResolution.height),
                .z = 1
            }
        }
    };
    vkCmdBlitImage(commandBuffer, srcTexture.textureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, vulkanSystem.SwapChainImages[vulkanSystem.ImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blitRegion, VK_FILTER_LINEAR);
}

void LevelSystem::RenderGameObjectPickerRenderPass(VkCommandBuffer& commandBuffer, VkGuid renderPassId)
{
    const VulkanRenderPass& renderPass = renderSystem.FindRenderPass(renderPassId);
    VulkanPipeline pipeline = renderSystem.FindRenderPipelineList(renderPassId)[0];

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

    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.Pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.PipelineLayout, 0, pipeline.DescriptorSetList.size(), pipeline.DescriptorSetList.data(), 0, nullptr);
    for (const auto& layer : spriteSystem.SpriteLayerList)
    {
        if (layer.InstanceCount == 0) continue;

        const Mesh& spriteMesh = meshSystem.FindMesh(spriteSystem.SpriteMeshId);
        const MeshAssetData& meshAsset = meshSystem.FindMeshAssetData(spriteMesh.SharedAssetId);
        const VkBuffer& indexBuffer = bufferSystem.FindVulkanBuffer(meshAsset.IndexBufferId).Buffer;
        const VulkanBuffer& instanceBuffer = bufferSystem.FindVulkanBuffer(memoryPoolSystem.GpuDataBufferIndex);

        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &instanceBuffer.Buffer, &memoryPoolSystem.GpuDataMemoryPoolHeader.SpriteInstanceOffset);
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, meshAsset.IndexCount, layer.InstanceCount, 0, 0, layer.StartInstanceIndex);
    }
    vkCmdEndRenderPass(commandBuffer);
}

void LevelSystem::RenderSelectedGameObjectPickerRenderPass(VkCommandBuffer& commandBuffer, VkGuid renderPassId)
{
    const VulkanRenderPass& renderPass = renderSystem.FindRenderPass(renderPassId);
    VulkanPipeline pipeline = renderSystem.FindRenderPipelineList(renderPassId)[0];

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

    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.Pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.PipelineLayout, 0, pipeline.DescriptorSetList.size(), pipeline.DescriptorSetList.data(), 0, nullptr);
    for (const auto& layer : spriteSystem.SpriteLayerList)
    {
        if (layer.InstanceCount == 0) continue;

        const Mesh& spriteMesh = meshSystem.FindMesh(spriteSystem.SpriteMeshId);
        const MeshAssetData& meshAsset = meshSystem.FindMeshAssetData(spriteMesh.SharedAssetId);
        const VkBuffer& indexBuffer = bufferSystem.FindVulkanBuffer(meshAsset.IndexBufferId).Buffer;
        const VulkanBuffer& instanceBuffer = bufferSystem.FindVulkanBuffer(memoryPoolSystem.GpuDataBufferIndex);

        ShaderPushConstant& sceneDataPushConstant = shaderSystem.FindShaderPushConstant("gameObjectPickerId");
        shaderSystem.UpdatePushConstantValue<uint>(sceneDataPushConstant, "gameObjectIndex", SelectedGameObject);

        vkCmdPushConstants(commandBuffer, pipeline.PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sceneDataPushConstant.PushConstantSize, sceneDataPushConstant.PushConstantBuffer.data());
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &instanceBuffer.Buffer, &memoryPoolSystem.GpuDataMemoryPoolHeader.SpriteInstanceOffset);
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, meshAsset.IndexCount, layer.InstanceCount, 0, 0, layer.StartInstanceIndex);
    }
    vkCmdEndRenderPass(commandBuffer);
}

LevelTileSet LevelSystem::LoadTileSetVRAM(const char* tileSetPath, const Material& material, const Texture& tileVramTexture)
{
    nlohmann::json json = fileSystem.LoadJsonFile(tileSetPath);

    LevelTileSet tileSet = LevelTileSet();
    tileSet.TileSetId = VkGuid(json["TileSetId"].get<String>().c_str());
    tileSet.MaterialId = material.MaterialGuid;
    tileSet.TilePixelSize = ivec2{ json["TilePixelSize"][0], json["TilePixelSize"][1] };
    tileSet.TileSetBounds = ivec2{ tileVramTexture.width / tileSet.TilePixelSize.x,  tileVramTexture.height / tileSet.TilePixelSize.y };
    tileSet.TileUVSize = vec2(1.0f / (float)tileSet.TileSetBounds.x, 1.0f / (float)tileSet.TileSetBounds.y);

    return tileSet;
}


void LevelSystem::LoadTileSets(const char* tileSetPath, LevelTileSet& tileSet)
{
    nlohmann::json json = fileSystem.LoadJsonFile(tileSetPath);

    Vector<Tile> tileList;
    for (int x = 0; x < json["TileList"].size(); x++)
    {
        Tile tile;
        tile.TileId = json["TileList"][x]["TileId"];
        tile.TileUVCellOffset = ivec2(json["TileList"][x]["TileUVCellOffset"][0], json["TileList"][x]["TileUVCellOffset"][1]);
        tile.TileLayer = json["TileList"][x]["TileLayer"];
        tile.TileCollider = json["TileList"][x]["TileCollider"];
        tile.IsAnimatedTile = json["TileList"][x]["IsAnimatedTile"];
        tile.TileUVOffset = vec2(tile.TileUVCellOffset.x * tileSet.TileUVSize.x, tile.TileUVCellOffset.y * tileSet.TileUVSize.y);
        tileList.emplace_back(tile);
    }
    tileSet.LevelTileCount = tileList.size();

    tileSet.LevelTileListPtr = memorySystem.AddPtrBuffer<Tile>(tileList.size(), __FILE__, __LINE__, __func__);
    std::memcpy(tileSet.LevelTileListPtr, tileList.data(), tileList.size() * sizeof(Tile));
}

LevelLayout LevelSystem::LoadLevelInfo(const char* levelLayoutPath)
{
    nlohmann::json json = fileSystem.LoadJsonFile(levelLayoutPath);

    LevelLayout levelLayout;
    levelLayout.LevelLayoutId = VkGuid(json["LevelLayoutId"].get<String>().c_str());
    levelLayout.LevelBounds = ivec2(json["LevelBounds"][0], json["LevelBounds"][1]);
    levelLayout.TileSizeinPixels = ivec2(json["TileSizeInPixels"][0], json["TileSizeInPixels"][1]);
    return levelLayout;
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

