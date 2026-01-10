#include "pch.h"
#include "LevelSystem.h"
#include "GameObjectSystem.h"
LevelSystem& levelSystem = LevelSystem::Get();

LevelLayer LevelSystem::LoadLevelInfo(VkGuid& levelId, const LevelTileSet& tileSet, uint* tileIdMap, size_t tileIdMapCount, ivec2& levelBounds, int levelLayerIndex)
{
    Vector<Tile>     tileMap;
    Vector<uint32>   indexList;
    Vector<Vertex2DLayout> vertexList;
    Vector<Tile>     tileSetList = Vector<Tile>(tileSet.LevelTileListPtr, tileSet.LevelTileListPtr + tileSet.LevelTileCount);
    Vector<uint>     tileIdMapList = Vector<uint>(tileIdMap, tileIdMap + tileIdMapCount);
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
            const Vertex2DLayout BottomLeftVertex = { { x * TilePixelSize.x,                         y * TilePixelSize.y},                     {LeftSideUV, BottomSideUV} };
            const Vertex2DLayout BottomRightVertex = { {(x * TilePixelSize.x) + TilePixelSize.x,      y * TilePixelSize.y},                     {RightSideUV, BottomSideUV} };
            const Vertex2DLayout TopRightVertex = { {(x * TilePixelSize.x) + TilePixelSize.x,     (y * TilePixelSize.y) + TilePixelSize.y},  {RightSideUV, TopSideUV} };
            const Vertex2DLayout TopLeftVertex = { { x * TilePixelSize.x,                        (y * TilePixelSize.y) + TilePixelSize.y},  {LeftSideUV, TopSideUV} };

            vertexList.emplace_back(BottomLeftVertex);
            vertexList.emplace_back(BottomRightVertex);
            vertexList.emplace_back(TopRightVertex);
            vertexList.emplace_back(TopLeftVertex);

            indexList.emplace_back(VertexCount);
            indexList.emplace_back(VertexCount + 1);
            indexList.emplace_back(VertexCount + 2);
            indexList.emplace_back(VertexCount + 2);
            indexList.emplace_back(VertexCount + 3);
            indexList.emplace_back(VertexCount);

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
        return tileSetId;

    const Material& material = materialSystem.FindMaterial(materialId);
    const Texture& tileSetTexture = textureSystem.FindTexture(material.AlbedoMapId);

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
        meshSystem.CreateMesh(MeshTypeEnum::kMesh_LevelMesh, vertexData, indexList, LevelLayerList[x].MaterialId);
    }
}

void LevelSystem::LoadSkyBox(const char* skyBoxMaterialPath)
{
    nlohmann::json json = fileSystem.LoadJsonFile(skyBoxMaterialPath);
    VkGuid skyBoxMaterialGuid = VkGuid(json["MaterialId"]);
    meshSystem.CreateSkyBox(skyBoxMaterialGuid);
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
    Camera_OrthographicUpdate(*OrthographicCamera.get());
    Camera_PerspectiveUpdate(*PerspectiveCamera.get());
}

void LevelSystem::LoadLevel(const char* levelPath)
{
    OrthographicCamera = std::make_shared<Camera>(Camera_OrthographicCamera2D(vec2((float)vulkanSystem.SwapChainResolution.width, (float)vulkanSystem.SwapChainResolution.height), vec3(0.0f, 0.0f, 0.0f)));
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

    for (size_t x = 0; x < json["LoadTextures"].size(); x++)
    {
        textureSystem.CreateTexture(json["LoadTextures"][x]);
    }

    for (size_t x = 0; x < json["LoadMaterials"].size(); x++)
    {
        materialSystem.LoadMaterial(json["LoadMaterials"][x]);
    }

    for (size_t x = 0; x < json["LoadSpriteVRAM"].size(); x++)
    {
        spriteSystem.LoadSpriteVRAM(json["LoadSpriteVRAM"][x]);
    }

    for (size_t x = 0; x < json["LoadTileSetVRAM"].size(); x++)
    {
        tileSetId = LoadTileSetVRAM(json["LoadTileSetVRAM"][x].get<String>().c_str());
    }

    for (size_t x = 0; x < json["LoadSceneLights"].size(); x++)
    {
        lightSystem.LoadSceneLights(json["LoadSceneLights"][x]);
    }

    for (size_t x = 0; x < json["LoadSkyBox"][x].size(); x++)
    {
        LoadSkyBox(json["LoadSkyBox"][x].get<String>().c_str());
    }


    for (size_t x = 0; x < json["GameObjectList"].size(); x++)
    {
        String objectJson = json["GameObjectList"][x]["GameObjectPath"];
        vec2 positionOverride(json["GameObjectList"][x]["GameObjectPositionOverride"][0], json["GameObjectList"][x]["GameObjectPositionOverride"][1]);
        gameObjectSystem.CreateGameObject(objectJson, positionOverride);
    }

    LoadLevelLayout(json["LoadLevelLayout"].get<String>().c_str());
    LoadLevelMesh(tileSetId);

    VkGuid levelId = VkGuid(json["LevelID"].get<String>().c_str());
    brdfRenderPassId = renderSystem.LoadRenderPass(dummyGuid, "RenderPass/BRDFRenderPass.json", ivec2(2048, 2048));
    renderSystem.GenerateTexture(brdfRenderPassId);

    directionalShadowRenderPassId = renderSystem.LoadRenderPass(levelLayout.LevelLayoutId, "RenderPass/DirectionalShadowRenderPass.json", ivec2(2048, 2048));
    sdfShaderRenderPassId = renderSystem.LoadRenderPass(levelLayout.LevelLayoutId, "RenderPass/SDFShadowRenderPass.json", ivec2(128, 128));
    skyBoxRenderPassId = renderSystem.LoadRenderPass(levelLayout.LevelLayoutId, "RenderPass/SkyBoxRenderPass.json", ivec2(vulkanSystem.SwapChainResolution.width, vulkanSystem.SwapChainResolution.height));
    irradianceMapRenderPassId = renderSystem.LoadRenderPass(levelLayout.LevelLayoutId, "RenderPass/IrradianceRenderPass.json", ivec2(256, 256));
    prefilterMapRenderPassId = renderSystem.LoadRenderPass(levelLayout.LevelLayoutId, "RenderPass/PrefilterRenderPass.json", ivec2(256, 256));
    gBufferRenderPassId = renderSystem.LoadRenderPass(levelLayout.LevelLayoutId, "RenderPass/GBufferRenderPass.json", ivec2(vulkanSystem.SwapChainResolution.width, vulkanSystem.SwapChainResolution.height));
    geometryRenderPassId = renderSystem.LoadRenderPass(levelLayout.LevelLayoutId, "RenderPass/GBufferLightingRenderPass.json", ivec2(vulkanSystem.SwapChainResolution.width, vulkanSystem.SwapChainResolution.height));
    verticalGaussianBlurRenderPassId = renderSystem.LoadRenderPass(dummyGuid, "RenderPass/VertGaussianBlurRenderPass.json", ivec2(vulkanSystem.SwapChainResolution.width, vulkanSystem.SwapChainResolution.height));
    horizontalGaussianBlurRenderPassId = renderSystem.LoadRenderPass(dummyGuid, "RenderPass/HorizontalGaussianBlurRenderPass.json", ivec2(vulkanSystem.SwapChainResolution.width, vulkanSystem.SwapChainResolution.height));
    bloomRenderPassId = renderSystem.LoadRenderPass(dummyGuid, "RenderPass/BloomRenderPass.json", ivec2(vulkanSystem.SwapChainResolution.width, vulkanSystem.SwapChainResolution.height));
   // shadowDebugRenderPassId = renderSystem.LoadRenderPass(levelLayout.LevelLayoutId, "RenderPass/ShadowDebugRenderPass.json", ivec2(vulkanSystem.SwapChainResolution.width, vulkanSystem.SwapChainResolution.height));
    hdrRenderPassId = renderSystem.LoadRenderPass(dummyGuid, "RenderPass/HdrRenderPass.json", ivec2(vulkanSystem.SwapChainResolution.width, vulkanSystem.SwapChainResolution.height));
    frameBufferId = renderSystem.LoadRenderPass(dummyGuid, "RenderPass/FrameBufferRenderPass.json", ivec2(vulkanSystem.SwapChainResolution.width, vulkanSystem.SwapChainResolution.height));
    
    //    levelWireFrameRenderPass2DId = LoadRenderPass(levelLayout.LevelLayoutId, "RenderPass/LevelShader2DWireFrameRenderPass.json", ivec2(vulkanSystem.SwapChainResolution.width, vulkanSystem.SwapChainResolution.height));
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

  void LevelSystem::Draw(VkCommandBuffer& commandBuffer, const float& deltaTime)
  {
      RenderDirectionalShadowRenderPass(commandBuffer, directionalShadowRenderPassId, levelLayout.LevelLayoutId, deltaTime);
      RenderSDFRenderPass(commandBuffer, sdfShaderRenderPassId, levelLayout.LevelLayoutId, deltaTime);
      RenderIrradianceMapRenderPass(commandBuffer, irradianceMapRenderPassId, deltaTime);
      RenderPrefilterMapRenderPass(commandBuffer, prefilterMapRenderPassId, deltaTime);
      RenderGBuffer(commandBuffer, gBufferRenderPassId, levelLayout.LevelLayoutId, deltaTime);
      RenderGeometryRenderPass(commandBuffer, geometryRenderPassId);
      RenderSkyBox(commandBuffer, skyBoxRenderPassId, deltaTime);
      RenderGaussianBlurPass(commandBuffer, verticalGaussianBlurRenderPassId, 0);
      RenderGaussianBlurPass(commandBuffer, horizontalGaussianBlurRenderPassId, 1);
      RenderBloomPass(commandBuffer, bloomRenderPassId);
      //RenderShadowDebug(commandBuffer, shadowDebugRenderPassId);
      RenderHdrPass(commandBuffer, hdrRenderPassId);
      RenderFrameBuffer(commandBuffer, frameBufferId);

      //commandBufferList.emplace_back(LevelSystem_RenderBloomPass(gaussianBlurRenderPassId));
  }

  void LevelSystem::RenderDirectionalShadowRenderPass(VkCommandBuffer& commandBuffer, VkGuid& renderPassId, VkGuid& levelId, const float deltaTime)
  {
      const VulkanRenderPass& renderPass = renderSystem.FindRenderPass(renderPassId);
      VulkanPipeline spritePipeline = renderSystem.FindRenderPipelineList(renderPassId)[0];
      VulkanPipeline levelPipeline = renderSystem.FindRenderPipelineList(renderPassId)[1];
      const Vector<Mesh>& levelLayerList = meshSystem.FindMeshByMeshType(MeshTypeEnum::kMesh_LevelMesh);
      Vector<Texture> renderPassTextures = textureSystem.FindRenderedTextureList(renderPass.RenderPassId);
      ShaderPushConstantDLL pushConstant = shaderSystem.FindShaderPushConstant("directionalLightPushConstant");

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
          .framebuffer = renderPass.FrameBufferList[vulkanSystem.ImageIndex],
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
      vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, levelPipeline.Pipeline);
      vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, levelPipeline.PipelineLayout, 0, 1, &levelPipeline.DescriptorSet, 0, nullptr);
      for (auto& levelLayer : levelLayerList)
      {
          const Vector<uint32>& indiceList = meshSystem.IndexList[levelLayer.IndexIndex];
          const VkBuffer& meshVertexBuffer = bufferSystem.FindVulkanBuffer(levelLayer.MeshVertexBufferId).Buffer;
          const VkBuffer& meshIndexBuffer = bufferSystem.FindVulkanBuffer(levelLayer.MeshIndexBufferId).Buffer;

          lightSystem.UpdateDirectionalLightOrthographicView(lightSystem.DirectionalLightList[0]);
          shaderSystem.UpdatePushConstantValue<uint>("directionalLightPushConstant", "MeshBufferIndex", levelLayer.MeshId);
          shaderSystem.UpdatePushConstantBuffer("directionalLightPushConstant");

          vkCmdPushConstants(commandBuffer, levelPipeline.PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, pushConstant.PushConstantSize, pushConstant.PushConstantBuffer.data());
          vkCmdBindVertexBuffers(commandBuffer, 0, 1, &meshVertexBuffer, offsets);
          vkCmdBindIndexBuffer(commandBuffer, meshIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
          vkCmdDrawIndexed(commandBuffer, indiceList.size(), 1, 0, 0, 0);
      }

      vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, spritePipeline.Pipeline);
      vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, spritePipeline.PipelineLayout, 0, 1, &spritePipeline.DescriptorSet, 0, nullptr);
      for (auto& spriteLayer : spriteSystem.SpriteLayerList)
      {
          const Mesh& spriteMesh = meshSystem.FindMesh(spriteLayer.second.SpriteLayerMeshId);
          const VkBuffer& meshIndexBuffer = bufferSystem.FindVulkanBuffer(spriteMesh.MeshIndexBufferId).Buffer;
          const Vector<SpriteInstance>& spriteInstanceList = spriteSystem.FindSpriteInstancesByLayer(spriteLayer.second);
          const VkBuffer& spriteInstanceBuffer = bufferSystem.FindVulkanBuffer(spriteLayer.second.SpriteLayerBufferId).Buffer;
          const Vector<uint32>& indiceList = meshSystem.IndexList[spriteMesh.IndexIndex];

          vkCmdBindVertexBuffers(commandBuffer, 0, 1, &spriteInstanceBuffer, instanceOffset);
          vkCmdBindIndexBuffer(commandBuffer, meshIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
          vkCmdDrawIndexed(commandBuffer, indiceList.size(), spriteInstanceList.size(), 0, 0, 0);
      }
      vkCmdEndRenderPass(commandBuffer);
      textureSystem.TransitionImageLayout(textureSystem.FindDepthTexture(levelSystem.sdfShaderRenderPassId), VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
  }

  void LevelSystem::RenderSDFRenderPass(VkCommandBuffer& commandBuffer, VkGuid& renderPassId, VkGuid& levelId, const float deltaTime)
  {
      const VulkanRenderPass& renderPass = renderSystem.FindRenderPass(renderPassId);
      VulkanPipeline spritePipeline = renderSystem.FindRenderPipelineList(renderPassId)[0];
      VulkanPipeline levelPipeline = renderSystem.FindRenderPipelineList(renderPassId)[1];
      const Vector<Mesh>& levelLayerList = meshSystem.FindMeshByMeshType(MeshTypeEnum::kMesh_LevelMesh);
      Vector<Texture> renderPassTextures = textureSystem.FindRenderedTextureList(renderPass.RenderPassId);
      ShaderPushConstantDLL& pushConstant = shaderSystem.FindShaderPushConstant("spfPointLightPushConstant");

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
          .framebuffer = renderPass.FrameBufferList[vulkanSystem.ImageIndex],
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
      lightSystem.UpdatePointLightOrthographicView(lightSystem.PointLightList[0]);
      vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
      vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
      vkCmdSetScissor(commandBuffer, 0, 1, &renderPassBeginInfo.renderArea);
      vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, levelPipeline.Pipeline);
      vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, levelPipeline.PipelineLayout, 0, 1, &levelPipeline.DescriptorSet, 0, nullptr);
      for (auto& levelLayer : levelLayerList)
      {
          const Vector<uint32>& indiceList = meshSystem.IndexList[levelLayer.IndexIndex];
          const VkBuffer& meshVertexBuffer = bufferSystem.FindVulkanBuffer(levelLayer.MeshVertexBufferId).Buffer;
          const VkBuffer& meshIndexBuffer = bufferSystem.FindVulkanBuffer(levelLayer.MeshIndexBufferId).Buffer;

          shaderSystem.UpdatePushConstantValue<uint>(pushConstant, "MeshBufferIndex", levelLayer.MeshId);
          shaderSystem.UpdatePushConstantBuffer(pushConstant);

          vkCmdPushConstants(commandBuffer, levelPipeline.PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, pushConstant.PushConstantSize, pushConstant.PushConstantBuffer.data());
          vkCmdBindVertexBuffers(commandBuffer, 0, 1, &meshVertexBuffer, offsets);
          vkCmdBindIndexBuffer(commandBuffer, meshIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
          vkCmdDrawIndexed(commandBuffer, indiceList.size(), 1, 0, 0, 0);
      }

      vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, spritePipeline.Pipeline);
      vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, spritePipeline.PipelineLayout, 0, 1, &spritePipeline.DescriptorSet, 0, nullptr);
      for (auto& spriteLayer : spriteSystem.SpriteLayerList)
      {
          const Mesh& spriteMesh = meshSystem.FindMesh(spriteLayer.second.SpriteLayerMeshId);
          const VkBuffer& meshIndexBuffer = bufferSystem.FindVulkanBuffer(spriteMesh.MeshIndexBufferId).Buffer;
          const Vector<SpriteInstance>& spriteInstanceList = spriteSystem.FindSpriteInstancesByLayer(spriteLayer.second);
          const VkBuffer& spriteInstanceBuffer = bufferSystem.FindVulkanBuffer(spriteLayer.second.SpriteLayerBufferId).Buffer;
          const Vector<uint32>& indiceList = meshSystem.IndexList[spriteMesh.IndexIndex];

          vkCmdBindVertexBuffers(commandBuffer, 0, 1, &spriteInstanceBuffer, instanceOffset);
          vkCmdBindIndexBuffer(commandBuffer, meshIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
          vkCmdDrawIndexed(commandBuffer, indiceList.size(), spriteInstanceList.size(), 0, 0, 0);
      }
      vkCmdEndRenderPass(commandBuffer);
      textureSystem.TransitionImageLayout(textureSystem.FindDepthTexture(levelSystem.sdfShaderRenderPassId), VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
  }

  void LevelSystem::RenderGBuffer(VkCommandBuffer& commandBuffer, VkGuid& renderPassId, VkGuid& levelId, const float deltaTime)
  {
      const VulkanRenderPass& renderPass = renderSystem.FindRenderPass(renderPassId);
      VulkanPipeline spritePipeline = renderSystem.FindRenderPipelineList(renderPassId)[0];
      VulkanPipeline levelPipeline = renderSystem.FindRenderPipelineList(renderPassId)[1];
      const Vector<Mesh>& levelLayerList = meshSystem.FindMeshByMeshType(MeshTypeEnum::kMesh_LevelMesh);
      ShaderPushConstantDLL& pushConstant = shaderSystem.FindShaderPushConstant("sceneData");

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

      VkDeviceSize offsets[] = { 0 };
      VkDeviceSize instanceOffset[] = { 0 };
      spriteSystem.Update(deltaTime);
      meshSystem.Update(deltaTime);
      lightSystem.Update(deltaTime);
      vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
      vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, levelPipeline.Pipeline);
      vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, levelPipeline.PipelineLayout, 0, 1, &levelPipeline.DescriptorSet, 0, nullptr);
      for (auto& levelLayer : levelLayerList)
      {
          const Vector<uint32>& indiceList = meshSystem.IndexList[levelLayer.IndexIndex];
          const VkBuffer& meshVertexBuffer = bufferSystem.FindVulkanBuffer(levelLayer.MeshVertexBufferId).Buffer;
          const VkBuffer& meshIndexBuffer = bufferSystem.FindVulkanBuffer(levelLayer.MeshIndexBufferId).Buffer;

          shaderSystem.UpdatePushConstantValue<uint>(pushConstant, "MeshBufferIndex", levelLayer.MeshId);
          shaderSystem.UpdatePushConstantBuffer(pushConstant);

          vkCmdPushConstants(commandBuffer, levelPipeline.PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, pushConstant.PushConstantSize, pushConstant.PushConstantBuffer.data());
          vkCmdBindVertexBuffers(commandBuffer, 0, 1, &meshVertexBuffer, offsets);
          vkCmdBindIndexBuffer(commandBuffer, meshIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
          vkCmdDrawIndexed(commandBuffer, indiceList.size(), 1, 0, 0, 0);
      }

      vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, spritePipeline.Pipeline);
      vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, spritePipeline.PipelineLayout, 0, 1, &spritePipeline.DescriptorSet, 0, nullptr);
      for (auto& spriteLayer : spriteSystem.SpriteLayerList)
      {
          const Mesh& spriteMesh = meshSystem.FindMesh(spriteLayer.second.SpriteLayerMeshId);
          const VkBuffer& meshIndexBuffer = bufferSystem.FindVulkanBuffer(spriteMesh.MeshIndexBufferId).Buffer;
          const Vector<SpriteInstance>& spriteInstanceList = spriteSystem.FindSpriteInstancesByLayer(spriteLayer.second);
          const VkBuffer& spriteInstanceBuffer = bufferSystem.FindVulkanBuffer(spriteLayer.second.SpriteLayerBufferId).Buffer;
          const Vector<uint32>& indiceList = meshSystem.IndexList[spriteMesh.IndexIndex];

          vkCmdBindVertexBuffers(commandBuffer, 0, 1, &spriteInstanceBuffer, instanceOffset);
          vkCmdBindIndexBuffer(commandBuffer, meshIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
          vkCmdDrawIndexed(commandBuffer, indiceList.size(), spriteInstanceList.size(), 0, 0, 0);
      }
      vkCmdEndRenderPass(commandBuffer);
  }

  void LevelSystem::RenderSkyBox(VkCommandBuffer& commandBuffer, VkGuid& renderPassId, float deltaTime)
  {
      const VulkanRenderPass& renderPass = renderSystem.FindRenderPass(renderPassId);
      VulkanPipeline skyboxPipeline = renderSystem.FindRenderPipelineList(renderPassId)[0];
      const Vector<Mesh>& skyBoxList = meshSystem.FindMeshByMeshType(MeshTypeEnum::kMesh_SkyBoxMesh);
      ShaderPushConstantDLL& pushConstant = shaderSystem.FindShaderPushConstant("skyBoxViewData");

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

      VkDeviceSize offsets[] = { 0 };
      VkDeviceSize instanceOffset[] = { 0 };
      vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
      vkCmdPushConstants(commandBuffer, skyboxPipeline.PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, pushConstant.PushConstantSize, pushConstant.PushConstantBuffer.data());
      vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, skyboxPipeline.Pipeline);
      vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, skyboxPipeline.PipelineLayout, 0, 1, &skyboxPipeline.DescriptorSet, 0, nullptr);
      for (auto& skybox : skyBoxList)
      {
          const Vector<uint32>& indiceList = meshSystem.IndexList[skybox.IndexIndex];
          const VkBuffer& meshVertexBuffer = bufferSystem.FindVulkanBuffer(skybox.MeshVertexBufferId).Buffer;
          const VkBuffer& meshIndexBuffer = bufferSystem.FindVulkanBuffer(skybox.MeshIndexBufferId).Buffer;

          shaderSystem.UpdatePushConstantValue<uint>(pushConstant, "MeshBufferIndex", skybox.MeshId);
          shaderSystem.UpdatePushConstantBuffer(pushConstant);

          vkCmdBindVertexBuffers(commandBuffer, 0, 1, &meshVertexBuffer, offsets);
          vkCmdBindIndexBuffer(commandBuffer, meshIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
          vkCmdDrawIndexed(commandBuffer, indiceList.size(), 1, 0, 0, 0);
      }
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
          .framebuffer = renderPass.FrameBufferList[vulkanSystem.ImageIndex],
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
      vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, skyboxPipeline.PipelineLayout, 0, 1, &skyboxPipeline.DescriptorSet, 0, nullptr);
      for (auto& skybox : skyBoxList)
      {
          const Vector<uint32>& indiceList = meshSystem.IndexList[skybox.IndexIndex];
          const VkBuffer& meshVertexBuffer = bufferSystem.FindVulkanBuffer(skybox.MeshVertexBufferId).Buffer;
          const VkBuffer& meshIndexBuffer = bufferSystem.FindVulkanBuffer(skybox.MeshIndexBufferId).Buffer;

          shaderSystem.UpdatePushConstantValue<float>(pushConstant, "sampleDelta", 0.1f);
          shaderSystem.UpdatePushConstantBuffer(pushConstant);

          vkCmdPushConstants(commandBuffer, skyboxPipeline.PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, pushConstant.PushConstantSize, pushConstant.PushConstantBuffer.data());
          vkCmdBindVertexBuffers(commandBuffer, 0, 1, &meshVertexBuffer, offsets);
          vkCmdBindIndexBuffer(commandBuffer, meshIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
          vkCmdDrawIndexed(commandBuffer, indiceList.size(), 1, 0, 0, 0);
      }
      vkCmdEndRenderPass(commandBuffer);
  }

  void LevelSystem::RenderPrefilterMapRenderPass(VkCommandBuffer& commandBuffer, VkGuid& renderPassId, float deltaTime)
  {
      VulkanRenderPass renderPass = renderSystem.FindRenderPass(renderPassId);
      VulkanPipeline skyboxPipeline = renderSystem.FindRenderPipelineList(renderPassId)[0];
      const Vector<Mesh>& skyBoxList = meshSystem.FindMeshByMeshType(MeshTypeEnum::kMesh_SkyBoxMesh);
      ShaderPushConstantDLL& pushConstant = shaderSystem.FindShaderPushConstant("prefilterSamplerProperties");

      // Assuming the skybox cube mesh is the first one (or however you store it)
      const Mesh& skyboxMesh = skyBoxList[0];
      const Vector<uint32>& indiceList = meshSystem.IndexList[skyboxMesh.IndexIndex];
      const VkBuffer& meshVertexBuffer = bufferSystem.FindVulkanBuffer(skyboxMesh.MeshVertexBufferId).Buffer;
      const VkBuffer& meshIndexBuffer = bufferSystem.FindVulkanBuffer(skyboxMesh.MeshIndexBufferId).Buffer;

      VkDeviceSize offsets[] = { 0 };

      // Base resolution of the prefilter cubemap (e.g., 1024)
      uint32_t baseSize = renderPass.RenderPassResolution.x;  // Should be the same as y
      uint32_t prefilterMipmapCount = textureSystem.PrefilterCubeMap.PrefilterMipmapCount;

      for (uint32_t mip = 0; mip < prefilterMipmapCount; ++mip)
      {
          uint32_t mipWidth = baseSize >> mip;
          uint32_t mipHeight = baseSize >> mip;

          // Update roughness for this mip level (0.0 at mip 0, 1.0 at last mip)
          float roughness = static_cast<float>(mip) / static_cast<float>(prefilterMipmapCount - 1);

          shaderSystem.UpdatePushConstantValue<uint>(pushConstant, "CubeMapResolution", baseSize);
          shaderSystem.UpdatePushConstantValue<float>(pushConstant, "Roughness", roughness);
          shaderSystem.UpdatePushConstantBuffer(pushConstant);

          VkRenderPassBeginInfo renderPassBeginInfo = {};
          renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
          renderPassBeginInfo.renderPass = renderPass.RenderPass;
          renderPassBeginInfo.framebuffer = textureSystem.PrefilterCubeMap.PrefilterMipFramebufferList[mip];
          renderPassBeginInfo.renderArea.offset = { 0, 0 };
          renderPassBeginInfo.renderArea.extent = { mipWidth, mipHeight };
          renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(renderPass.ClearValueList.size());
          renderPassBeginInfo.pClearValues = renderPass.ClearValueList.data();

          vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

          vkCmdPushConstants(commandBuffer,
              skyboxPipeline.PipelineLayout,
              VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
              0,
              pushConstant.PushConstantSize,
              pushConstant.PushConstantBuffer.data());

          vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, skyboxPipeline.Pipeline);
          vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, skyboxPipeline.PipelineLayout, 0, 1, &skyboxPipeline.DescriptorSet, 0, nullptr);
          vkCmdBindVertexBuffers(commandBuffer, 0, 1, &meshVertexBuffer, offsets);
          vkCmdBindIndexBuffer(commandBuffer, meshIndexBuffer, 0, VK_INDEX_TYPE_UINT32);

          // Viewport and scissor matched to current mip size
          VkViewport viewport = {};
          viewport.x = 0.0f;
          viewport.y = 0.0f;
          viewport.width = static_cast<float>(mipWidth);
          viewport.height = static_cast<float>(mipHeight);
          viewport.minDepth = 0.0f;
          viewport.maxDepth = 1.0f;
          vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

          VkRect2D scissor = {};
          scissor.offset = { 0, 0 };
          scissor.extent = { mipWidth, mipHeight };
          vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

          // Single draw call renders all 6 faces thanks to multiview + layered framebuffer
          vkCmdDrawIndexed(commandBuffer,
              static_cast<uint32_t>(indiceList.size()),
              1,         // instanceCount = 1
              0,         // firstIndex
              0,         // vertexOffset
              0);        // firstInstance

          vkCmdEndRenderPass(commandBuffer);

          // No per-mip barrier needed here — Vulkan guarantees visibility within the same render pass sequence
          // (and we're writing to different mip levels anyway)
      }

      // Final layout transition: make all mips readable as shader texture (for sampling in lighting)
      VkImageMemoryBarrier finalBarrier = {};
      finalBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
      finalBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      finalBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      finalBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      finalBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
      finalBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      finalBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      finalBarrier.image = textureSystem.PrefilterCubeMap.PrefilterCubeMap.textureImage;
      finalBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      finalBarrier.subresourceRange.baseMipLevel = 0;
      finalBarrier.subresourceRange.levelCount = prefilterMipmapCount;
      finalBarrier.subresourceRange.baseArrayLayer = 0;
      finalBarrier.subresourceRange.layerCount = 6;

      vkCmdPipelineBarrier(commandBuffer,
          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
          VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
          0,
          0, nullptr,
          0, nullptr,
          1, &finalBarrier);
  }
 
 void LevelSystem::RenderGeometryRenderPass(VkCommandBuffer& commandBuffer, VkGuid& renderPassId)
 {
     const VulkanRenderPass renderPass = renderSystem.FindRenderPass(renderPassId);
     VulkanPipeline pipeline = renderSystem.FindRenderPipelineList(renderPassId)[0];
     Vector<Texture> renderPassTexture = textureSystem.FindRenderedTextureList(renderPassId);

     ShaderPushConstantDLL& pushConstant = shaderSystem.FindShaderPushConstant("gBufferSceneDataBuffer");
     shaderSystem.UpdatePushConstantValue<int>(pushConstant, "UseHeightMap", UseHeightMap);
     shaderSystem.UpdatePushConstantValue<float>(pushConstant, "HeightScale", HeightScale);
     shaderSystem.UpdatePushConstantValue<vec3>(pushConstant, "ViewDirection", ViewDirection);
     shaderSystem.UpdatePushConstantValue<uint>(pushConstant, "DirectionalLightCount", lightSystem.DirectionalLightList.size());
     shaderSystem.UpdatePushConstantValue<uint>(pushConstant, "PointLightCount", lightSystem.PointLightList.size());
     shaderSystem.UpdatePushConstantValue<mat4>(pushConstant, "InvProjection", glm::inverse(PerspectiveCamera->ProjectionMatrix));
     shaderSystem.UpdatePushConstantValue<mat4>(pushConstant, "InvView", glm::inverse(PerspectiveCamera->ViewMatrix));
     shaderSystem.UpdatePushConstantBuffer(pushConstant);

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
     vkCmdPushConstants(commandBuffer, pipeline.PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, pushConstant.PushConstantSize, pushConstant.PushConstantBuffer.data());
     vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.Pipeline);
     vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.PipelineLayout, 0, 1, &pipeline.DescriptorSet, 0, nullptr);
     vkCmdDraw(commandBuffer, 6, 1, 0, 0);
     vkCmdEndRenderPass(commandBuffer);
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
      vkCmdPushConstants(commandBuffer, pipeline.PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, pushConstant.PushConstantSize, pushConstant.PushConstantBuffer.data());
      vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
      vkCmdSetScissor(commandBuffer, 0, 1, &renderPassBeginInfo.renderArea);
      vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.Pipeline);
      vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.PipelineLayout, 0, 1, &pipeline.DescriptorSet, 0, nullptr);
      vkCmdDraw(commandBuffer, 6, 1, 0, 0);
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
      vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
      vkCmdSetScissor(commandBuffer, 0, 1, &renderPassBeginInfo.renderArea);
      vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.Pipeline);
      vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.PipelineLayout, 0, 1, &pipeline.DescriptorSet, 0, nullptr);
      vkCmdDraw(commandBuffer, 6, 1, 0, 0);
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
     vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.PipelineLayout, 0, 1, &pipeline.DescriptorSet, 0, nullptr);
     vkCmdDraw(commandBuffer, 6, 1, 0, 0);
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
     vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.PipelineLayout, 0, 1, &pipeline.DescriptorSet, 0, nullptr);
     vkCmdDraw(commandBuffer, 6, 1, 0, 0);
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
     vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
     vkCmdSetScissor(commandBuffer, 0, 1, &renderPassBeginInfo.renderArea);
     vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.Pipeline);
     vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.PipelineLayout, 0, 1, &pipeline.DescriptorSet, 0, nullptr);
     vkCmdDraw(commandBuffer, 6, 1, 0, 0);
     vkCmdEndRenderPass(commandBuffer);
 }