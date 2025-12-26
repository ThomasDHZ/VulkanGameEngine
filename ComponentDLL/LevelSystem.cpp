#include "pch.h"
#include "LevelSystem.h"
#include "GameObjectSystem.h"

LevelSystem& levelSystem = LevelSystem::Get();

LevelLayer LevelSystem::LoadLevelInfo(VkGuid& levelId, const LevelTileSet& tileSet, uint* tileIdMap, size_t tileIdMapCount, ivec2& levelBounds, int levelLayerIndex)
{
    Vector<Tile>     tileMap;
    Vector<uint32>   indexList;
    Vector<Vertex2D> vertexList;
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
            const Vertex2D BottomLeftVertex = { { x * TilePixelSize.x,                         y * TilePixelSize.y},                     {LeftSideUV, BottomSideUV} };
            const Vertex2D BottomRightVertex = { {(x * TilePixelSize.x) + TilePixelSize.x,      y * TilePixelSize.y},                     {RightSideUV, BottomSideUV} };
            const Vertex2D TopRightVertex = { {(x * TilePixelSize.x) + TilePixelSize.x,     (y * TilePixelSize.y) + TilePixelSize.y},  {RightSideUV, TopSideUV} };
            const Vertex2D TopLeftVertex = { { x * TilePixelSize.x,                        (y * TilePixelSize.y) + TilePixelSize.y},  {LeftSideUV, TopSideUV} };

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
        .VertexList = memorySystem.AddPtrBuffer<Vertex2D>(vertexList.size(), __FILE__, __LINE__, __func__),
        .IndexList = memorySystem.AddPtrBuffer<uint32>(indexList.size(), __FILE__, __LINE__, __func__),
        .TileIdMapCount = tileIdMapList.size(),
        .TileMapCount = tileMap.size(),
        .VertexListCount = vertexList.size(),
        .IndexListCount = indexList.size()
    };

    std::memcpy(levelLayout.TileMap, tileMap.data(), tileMap.size() * sizeof(Tile));
    std::memcpy(levelLayout.TileIdMap, tileIdMapList.data(), tileIdMapList.size() * sizeof(uint));
    std::memcpy(levelLayout.VertexList, vertexList.data(), vertexList.size() * sizeof(Vertex2D));
    std::memcpy(levelLayout.IndexList, indexList.data(), indexList.size() * sizeof(uint32));

    return levelLayout;
}

void LevelSystem::DeleteLevel(uint* TileIdMap, Tile* TileMap, Vertex2D* VertexList, uint32* IndexList)
{
    memorySystem.DeletePtr<uint>(TileIdMap);
    memorySystem.DeletePtr<Tile>(TileMap);
    memorySystem.DeletePtr<Vertex2D>(VertexList);
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

        Vector<Vertex2D> vertexList(LevelLayerList[x].VertexList, LevelLayerList[x].VertexList + LevelLayerList[x].VertexListCount);
        Vector<uint> indexList(LevelLayerList[x].IndexList, LevelLayerList[x].IndexList + LevelLayerList[x].IndexListCount);
        meshSystem.CreateMesh(MeshTypeEnum::Mesh_LevelMesh, vertexList, indexList);
    }
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
    Camera_Update(*OrthographicCamera.get());

}

void LevelSystem::LoadLevel(const char* levelPath)
{
    OrthographicCamera = std::make_shared<Camera>(Camera_OrthographicCamera2D(vec2((float)vulkanSystem.SwapChainResolution.width, (float)vulkanSystem.SwapChainResolution.height), vec3(0.0f, 0.0f, 0.0f)));

    VkGuid dummyGuid = VkGuid();
    VkGuid tileSetId = VkGuid();

#if defined(_WIN32)
    shaderSystem.CompileShaders(configSystem.ShaderSourceDirectory.c_str(), configSystem.CompiledShaderOutputDirectory.c_str());
#endif
    nlohmann::json json = fileSystem.LoadJsonFile(levelPath);
    nlohmann::json shaderJson = fileSystem.LoadJsonFile("RenderPass/LevelShader2DRenderPass.json");
    nlohmann::json shaderWiredJson = fileSystem.LoadJsonFile("RenderPass/LevelShader2DWireFrameRenderPass.json");
    spriteRenderPass2DId = VkGuid(shaderJson["RenderPassId"].get<String>().c_str());
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

    for (size_t x = 0; x < json["GameObjectList"].size(); x++)
    {
        String objectJson = json["GameObjectList"][x]["GameObjectPath"];
        vec2 positionOverride(json["GameObjectList"][x]["GameObjectPositionOverride"][0], json["GameObjectList"][x]["GameObjectPositionOverride"][1]);
        gameObjectSystem.CreateGameObject(objectJson, positionOverride);
    }

    LoadLevelLayout(json["LoadLevelLayout"].get<String>().c_str());
    LoadLevelMesh(tileSetId);

    VkGuid levelId = VkGuid(json["LevelID"].get<String>().c_str());
    gBufferRenderPassId = renderSystem.LoadRenderPass(levelLayout.LevelLayoutId, "RenderPass/GBufferRenderPass.json", ivec2(vulkanSystem.SwapChainResolution.width, vulkanSystem.SwapChainResolution.height));
    spriteRenderPass2DId = renderSystem.LoadRenderPass(levelLayout.LevelLayoutId, "RenderPass/LevelShader2DRenderPass.json", ivec2(vulkanSystem.SwapChainResolution.width, vulkanSystem.SwapChainResolution.height));
    verticalGaussianBlurRenderPassId = renderSystem.LoadRenderPass(dummyGuid, "RenderPass/VertGaussianBlurRenderPass.json", ivec2(vulkanSystem.SwapChainResolution.width, vulkanSystem.SwapChainResolution.height));
    horizontalGaussianBlurRenderPassId = renderSystem.LoadRenderPass(dummyGuid, "RenderPass/HorizontalGaussianBlurRenderPass.json", ivec2(vulkanSystem.SwapChainResolution.width, vulkanSystem.SwapChainResolution.height));
    bloomRenderPassId = renderSystem.LoadRenderPass(dummyGuid, "RenderPass/BloomRenderPass.json", ivec2(vulkanSystem.SwapChainResolution.width, vulkanSystem.SwapChainResolution.height));
    hdrRenderPassId = renderSystem.LoadRenderPass(dummyGuid, "RenderPass/HdrRenderPass.json", ivec2(vulkanSystem.SwapChainResolution.width, vulkanSystem.SwapChainResolution.height));
    frameBufferId = renderSystem.LoadRenderPass(dummyGuid, "RenderPass/FrameBufferRenderPass.json", ivec2(vulkanSystem.SwapChainResolution.width, vulkanSystem.SwapChainResolution.height));
    
    //    levelWireFrameRenderPass2DId = LoadRenderPass(levelLayout.LevelLayoutId, "RenderPass/LevelShader2DWireFrameRenderPass.json", ivec2(vulkanSystem.SwapChainResolution.width, vulkanSystem.SwapChainResolution.height));
}

 void LevelSystem::Draw(VkCommandBuffer& commandBuffer, const float& deltaTime)
 {
     RenderGBuffer(commandBuffer, gBufferRenderPassId, levelLayout.LevelLayoutId, deltaTime);
     RenderLevel(commandBuffer, spriteRenderPass2DId, levelLayout.LevelLayoutId, deltaTime);
     RenderGaussianBlurPass(commandBuffer, verticalGaussianBlurRenderPassId, 0);
     RenderGaussianBlurPass(commandBuffer, horizontalGaussianBlurRenderPassId, 1);
     RenderBloomPass(commandBuffer, bloomRenderPassId);
     RenderHdrPass(commandBuffer, hdrRenderPassId);
     RenderFrameBuffer(commandBuffer, frameBufferId);

     //commandBufferList.emplace_back(LevelSystem_RenderBloomPass(gaussianBlurRenderPassId));
 }

 void LevelSystem::RenderGBuffer(VkCommandBuffer& commandBuffer, VkGuid& renderPassId, VkGuid& levelId, const float deltaTime)
 {
     const VulkanRenderPass& renderPass = renderSystem.FindRenderPass(renderPassId);
     VulkanPipeline spritePipeline = renderSystem.FindRenderPipelineList(renderPassId)[0];
     VulkanPipeline levelPipeline = renderSystem.FindRenderPipelineList(renderPassId)[1];
     const Vector<Mesh>& levelLayerList = meshSystem.FindMeshByMeshType(MeshTypeEnum::Mesh_LevelMesh);
     ShaderPushConstantDLL pushConstant = shaderSystem.FindShaderPushConstant("sceneData");
     Vector<Texture> renderPassTextures = textureSystem.FindRenderedTextureList(renderPass.RenderPassId);

     VkRenderPassBeginInfo renderPassBeginInfo = VkRenderPassBeginInfo
     {
         .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
         .renderPass = renderPass.RenderPass,
         .framebuffer = renderPass.FrameBufferList[vulkanSystem.ImageIndex],
         .renderArea = VkRect2D
         {
             .offset = VkOffset2D
             {
                 .x = 0,
                 .y = 0
             },
            .extent = VkExtent2D
                 {
                         .width = static_cast<uint>(renderPass.RenderPassResolution.x),
                         .height = static_cast<uint>(renderPass.RenderPassResolution.y)
                 }
         },
         .clearValueCount = static_cast<uint32>(renderPass.ClearValueList.size()),
         .pClearValues = renderPass.ClearValueList.data()
     };

     VkDeviceSize offsets[] = { 0 };
     VkDeviceSize instanceOffset[] = { 0 };
     spriteSystem.Update(deltaTime);
     shaderSystem.UpdateGlobalShaderBuffer("sceneData");
     meshSystem.Update(deltaTime);
     vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
     for (auto& levelLayer : levelLayerList)
     {
         const Vector<uint32>& indiceList = meshSystem.IndexList[levelLayer.IndexIndex];
         const VkBuffer& meshVertexBuffer = bufferSystem.FindVulkanBuffer(levelLayer.MeshVertexBufferId).Buffer;
         const VkBuffer& meshIndexBuffer = bufferSystem.FindVulkanBuffer(levelLayer.MeshIndexBufferId).Buffer;

         // memcpy(shaderSystem.SearchGlobalShaderConstantVar(&sceneDataBuffer, "MeshBufferIndex")->Value, &meshIndex, sizeof(meshIndex));
         vkCmdPushConstants(commandBuffer, levelPipeline.PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, pushConstant.PushConstantSize, pushConstant.PushConstantBuffer.data());
         vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, levelPipeline.Pipeline);
         vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, levelPipeline.PipelineLayout, 0, levelPipeline.DescriptorSetList.size(), levelPipeline.DescriptorSetList.data(), 0, nullptr);
         vkCmdBindVertexBuffers(commandBuffer, 0, 1, &meshVertexBuffer, offsets);
         vkCmdBindIndexBuffer(commandBuffer, meshIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
         vkCmdDrawIndexed(commandBuffer, indiceList.size(), 1, 0, 0, 0);
     }
     for (auto& spriteLayer : spriteSystem.SpriteLayerList)
     {
         const Mesh& spriteMesh = meshSystem.FindMesh(spriteLayer.second.SpriteLayerMeshId);
         const VkBuffer& meshVertexBuffer = bufferSystem.FindVulkanBuffer(spriteMesh.MeshVertexBufferId).Buffer;
         const VkBuffer& meshIndexBuffer = bufferSystem.FindVulkanBuffer(spriteMesh.MeshIndexBufferId).Buffer;
         const Vector<SpriteInstance>& spriteInstanceList = spriteSystem.FindSpriteInstancesByLayer(spriteLayer.second);
         const VkBuffer& spriteInstanceBuffer = bufferSystem.FindVulkanBuffer(spriteLayer.second.SpriteLayerBufferId).Buffer;
         const Vector<uint32>& indiceList = meshSystem.IndexList[spriteMesh.IndexIndex];

         // memcpy(shaderSystem.SearchGlobalShaderConstantVar(&sceneDataBuffer, "MeshBufferIndex")->Value, &spriteLayer.SpriteLayerMeshId, sizeof(spriteLayer.SpriteLayerMeshId));
         vkCmdPushConstants(commandBuffer, spritePipeline.PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, pushConstant.PushConstantSize, pushConstant.PushConstantBuffer.data());
         vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, spritePipeline.Pipeline);
         vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, spritePipeline.PipelineLayout, 0, spritePipeline.DescriptorSetList.size(), spritePipeline.DescriptorSetList.data(), 0, nullptr);
         vkCmdBindVertexBuffers(commandBuffer, 0, 1, &spriteInstanceBuffer, instanceOffset);
         vkCmdBindIndexBuffer(commandBuffer, meshIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
         vkCmdDrawIndexed(commandBuffer, indiceList.size(), spriteInstanceList.size(), 0, 0, 0);
     }
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

  void LevelSystem::RenderLevel(VkCommandBuffer& commandBuffer, VkGuid& renderPassId, VkGuid& levelId, const float deltaTime)
  {
      const VulkanRenderPass& renderPass = renderSystem.FindRenderPass(renderPassId);
      VulkanPipeline spritePipeline = renderSystem.FindRenderPipelineList(renderPassId)[0];
      VulkanPipeline levelPipeline = renderSystem.FindRenderPipelineList(renderPassId)[1];
      const Vector<Mesh>& levelLayerList = meshSystem.FindMeshByMeshType(MeshTypeEnum::Mesh_LevelMesh);
      ShaderPushConstantDLL pushConstant = shaderSystem.FindShaderPushConstant("sceneData");
      Vector<Texture> renderPassTextures = textureSystem.FindRenderedTextureList(renderPass.RenderPassId);

      VkRenderPassBeginInfo renderPassBeginInfo = VkRenderPassBeginInfo
      {
          .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
          .renderPass = renderPass.RenderPass,
          .framebuffer = renderPass.FrameBufferList[vulkanSystem.ImageIndex],
          .renderArea = VkRect2D
          {
              .offset = VkOffset2D
              {
                  .x = 0,
                  .y = 0
              },
             .extent = VkExtent2D
                  {
                          .width = static_cast<uint>(renderPass.RenderPassResolution.x),
                          .height = static_cast<uint>(renderPass.RenderPassResolution.y)
                  }
          },
          .clearValueCount = static_cast<uint32>(renderPass.ClearValueList.size()),
          .pClearValues = renderPass.ClearValueList.data()
      };

      VkDeviceSize offsets[] = { 0 };
      VkDeviceSize instanceOffset[] = { 0 };
      spriteSystem.Update(deltaTime);
      shaderSystem.UpdateGlobalShaderBuffer("sceneData");
      meshSystem.Update(deltaTime);
      vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
      for (auto& levelLayer : levelLayerList)
      {
          const Vector<uint32>& indiceList = meshSystem.IndexList[levelLayer.IndexIndex];
          const VkBuffer& meshVertexBuffer = bufferSystem.FindVulkanBuffer(levelLayer.MeshVertexBufferId).Buffer;
          const VkBuffer& meshIndexBuffer = bufferSystem.FindVulkanBuffer(levelLayer.MeshIndexBufferId).Buffer;

          // memcpy(shaderSystem.SearchGlobalShaderConstantVar(&sceneDataBuffer, "MeshBufferIndex")->Value, &meshIndex, sizeof(meshIndex));
          vkCmdPushConstants(commandBuffer, levelPipeline.PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, pushConstant.PushConstantSize, pushConstant.PushConstantBuffer.data());
          vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, levelPipeline.Pipeline);
          vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, levelPipeline.PipelineLayout, 0, levelPipeline.DescriptorSetList.size(), levelPipeline.DescriptorSetList.data(), 0, nullptr);
          vkCmdBindVertexBuffers(commandBuffer, 0, 1, &meshVertexBuffer, offsets);
          vkCmdBindIndexBuffer(commandBuffer, meshIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
          vkCmdDrawIndexed(commandBuffer, indiceList.size(), 1, 0, 0, 0);
      }
      for (auto& spriteLayer : spriteSystem.SpriteLayerList)
      {
          const Mesh& spriteMesh = meshSystem.FindMesh(spriteLayer.second.SpriteLayerMeshId);
          const VkBuffer& meshVertexBuffer = bufferSystem.FindVulkanBuffer(spriteMesh.MeshVertexBufferId).Buffer;
          const VkBuffer& meshIndexBuffer = bufferSystem.FindVulkanBuffer(spriteMesh.MeshIndexBufferId).Buffer;
          const Vector<SpriteInstance>& spriteInstanceList = spriteSystem.FindSpriteInstancesByLayer(spriteLayer.second);
          const VkBuffer& spriteInstanceBuffer = bufferSystem.FindVulkanBuffer(spriteLayer.second.SpriteLayerBufferId).Buffer;
          const Vector<uint32>& indiceList = meshSystem.IndexList[spriteMesh.IndexIndex];

          // memcpy(shaderSystem.SearchGlobalShaderConstantVar(&sceneDataBuffer, "MeshBufferIndex")->Value, &spriteLayer.SpriteLayerMeshId, sizeof(spriteLayer.SpriteLayerMeshId));
          vkCmdPushConstants(commandBuffer, spritePipeline.PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, pushConstant.PushConstantSize, pushConstant.PushConstantBuffer.data());
          vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, spritePipeline.Pipeline);
          vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, spritePipeline.PipelineLayout, 0, spritePipeline.DescriptorSetList.size(), spritePipeline.DescriptorSetList.data(), 0, nullptr);
          vkCmdBindVertexBuffers(commandBuffer, 0, 1, &spriteInstanceBuffer, instanceOffset);
          vkCmdBindIndexBuffer(commandBuffer, meshIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
          vkCmdDrawIndexed(commandBuffer, indiceList.size(), spriteInstanceList.size(), 0, 0, 0);
      }
      vkCmdEndRenderPass(commandBuffer);
  }

  void LevelSystem::RenderGaussianBlurPass(VkCommandBuffer& commandBuffer, VkGuid& renderPassId, uint blurDirection)
  {
      const VulkanRenderPass renderPass = renderSystem.FindRenderPass(renderPassId);
      const VulkanPipeline  pipeline = renderSystem.FindRenderPipelineList(renderPassId)[0];

      ShaderPushConstantDLL pushConstant = shaderSystem.FindShaderPushConstant("bloomSettings");
      shaderSystem.UpdatePushConstantValue<uint>("bloomSettings", "blurDirection", blurDirection);
      shaderSystem.UpdatePushConstantValue<float>("bloomSettings", "blurScale", 1.0f);
      shaderSystem.UpdatePushConstantValue<float>("bloomSettings", "blurStrength", 0.04f);

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
      vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.PipelineLayout, 0, pipeline.DescriptorSetList.size(), pipeline.DescriptorSetList.data(), 0, nullptr);
      vkCmdDraw(commandBuffer, 6, 1, 0, 0);
      vkCmdEndRenderPass(commandBuffer);
  }

  void LevelSystem::RenderHdrPass(VkCommandBuffer& commandBuffer, VkGuid& renderPassId)
  {
      const VulkanRenderPass renderPass = renderSystem.FindRenderPass(renderPassId);
      VulkanPipeline pipeline = renderSystem.FindRenderPipelineList(renderPassId)[0];
      Vector<Texture> renderPassTexture = textureSystem.FindRenderedTextureList(renderPassId);

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
     vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.PipelineLayout, 0, pipeline.DescriptorSetList.size(), pipeline.DescriptorSetList.data(), 0, nullptr);
     vkCmdDraw(commandBuffer, 6, 1, 0, 0);
     vkCmdEndRenderPass(commandBuffer);
 }

 void LevelSystem::RenderFrameBuffer(VkCommandBuffer& commandBuffer, VkGuid& renderPassId)
 {
     const VulkanRenderPass renderPass = renderSystem.FindRenderPass(renderPassId);
     VulkanPipeline pipeline = renderSystem.FindRenderPipelineList(renderPassId)[0];
     Vector<Texture> renderPassTexture = textureSystem.FindRenderedTextureList(spriteRenderPass2DId);

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
     vkCmdDraw(commandBuffer, 6, 1, 0, 0);
     vkCmdEndRenderPass(commandBuffer);
 }
