#include "LevelSystem.h"
#include "EngineConfigSystem.h"
#include "GameObjectSystem.h"
#include "MeshSystem.h"
#include "RenderSystem.h"
#include "CameraSystem.h"
#include "LightSystem.h"
#include "PushConstantRegistry.h"
#include "Camera.h"
#include <algorithm>

LevelSystem& levelSystem = LevelSystem::Get();

void LevelSystem::LoadLevel(const char* levelPath)
{
#if defined(_WIN32)
    shaderSystem.CompileShaders(configSystem.ShaderSourceDirectory.c_str(), configSystem.CompiledShaderOutputDirectory.c_str());
#endif

    cameraSystem.CreateCamera(CameraTypeEnum::kPixelPerfectOrthographicCam, vec2((float)vulkan.RenderPassResolution().x, (float)vulkan.RenderPassResolution().y), vec2(0.0f, 0.0f));
    PerspectiveCamera = std::make_shared<Camera>(Camera_PerspectiveCamera(vec2((float)vulkan.RenderPassResolution().x, (float)vulkan.RenderPassResolution().y), vec3(0.0f, 0.0f, 0.0f)));
    pushConstantRegistry.RegisterDefaultPushConstantRules();

    VkGuid tileSetId = VkGuid();
    nlohmann::json json = fileSystem.LoadJsonFile(levelPath);
    for (auto& texture : json["LoadTextures"])    textureSystem.LoadTexture(texture);
    for (auto& ktxTexture : json["LoadKTXTextures"]) textureSystem.LoadTexture(ktxTexture);
    for (auto& material : json["LoadMaterials"])   materialSystem.LoadMaterial(material.get<std::string>());
    for (auto& spriteVRAM : json["LoadSpriteVRAM"])  spriteSystem.LoadSpriteVRAM(spriteVRAM);
    for (auto& tileSetVRAM : json["LoadTileSetVRAM"]) tileSetId = LoadTileSetVRAM(tileSetVRAM.get<String>().c_str());

    Vector<String> gameObjectTempleteList;
    for (size_t x = 0; x < json["LoadGameObjects"].size(); x++)
    {
        gameObjectTempleteList.emplace_back(json["LoadGameObjects"][x]);
    }
    gameObjectSystem.LoadGameObjectTempletes(gameObjectTempleteList);
    gameObjectSystem.CreateGameObjects(json["GameObjectList"]);

    LoadSkyBox();
    brdfRenderPassId                   = renderSystem.LoadRenderPass("RenderPass/BRDFRenderPass.json");
    environmentToCubeMapRenderPassId   = renderSystem.LoadRenderPass("RenderPass/EnvironmentToCubeMapRenderPass.json");
    irradianceMapRenderPassId          = renderSystem.LoadRenderPass("RenderPass/IrradianceRenderPass.json");
    prefilterMapRenderPassId           = renderSystem.LoadRenderPass("RenderPass/PrefilterRenderPass.json");
    gBufferRenderPassId                = renderSystem.LoadRenderPass("RenderPass/GBufferRenderPass.json");
    hdrRenderPassId                    = renderSystem.LoadRenderPass("RenderPass/HdrRenderPass.json");
   // textRenderPassId                   = renderSystem.LoadRenderPass("RenderPass/TextRenderPass.json");
    //objectPickerRenderPassId         =   renderSystem.LoadRenderPass("RenderPass/ObjectPickerRenderPass.json");
    //selectedObjectPickerRenderPassId =   renderSystem.LoadRenderPass("RenderPass/SelectedGameObjectPickerRenderPass.json");

    //shaderSystem.LoadShaderPipelineStructPrototypes(json["LoadRenderPasses"]);
    LoadLevelLayout(json["LoadLevelLayout"].get<String>().c_str());
    LoadLevelMesh(tileSetId);
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
    sceneDataBuffer.BRDFMapId = 13;
    sceneDataBuffer.HDRMapIndex = 23;
    sceneDataBuffer.FrameBufferIndex = 25;
    sceneDataBuffer.IrradianceMapId = 1;
    sceneDataBuffer.PrefilterMapId = 2;
    sceneDataBuffer.CubeMapId = 0;
    cameraSystem.Update();
}

Vector<RenderPassNode> LevelSystem::Draw(VkCommandBuffer& commandBuffer, const float& deltaTime)
{
    Vector<VkGuid> renderPassesToDraw
    {
        irradianceMapRenderPassId,
        prefilterMapRenderPassId,
        gBufferRenderPassId,
        hdrRenderPassId
    };

    Vector<RenderPassNode> renderPassNodeList;
    for (auto& renderPassGuid : renderPassesToDraw)
    {
        const VulkanRenderPass& renderPass = renderSystem.FindRenderPass(renderPassGuid);

        uint32 maxMipLevelCount = 1;
        Vector<Vector<VulkanDrawMessage>> vulkanDrawMessageList;
        for (auto& renderPassList : renderPass.SubPassList())
        {
            Vector<VulkanDrawMessage> vulkanSubPassMessageList;
            for (auto& subPass : renderPassList)
            {
                for (auto& inputTexture : subPass.InputTextureList)
                {
                    const Texture& texture = textureSystem.FindRenderedTexture(inputTexture);
                    if (maxMipLevelCount < texture.texture.MipMapLevels()) maxMipLevelCount = texture.texture.MipMapLevels() - 1;
                }

                Vector<MeshDrawMessage> meshList;
                meshList = MeshTypeEnum::kMesh_StaticMesh && renderPass.IsCubeMapRenderPass() ? meshSystem.DrawMesh("__SkyBoxMesh__") : meshSystem.DrawMesh(subPass.MeshType);
                vulkanSubPassMessageList.emplace_back(VulkanDrawMessage
                    {
                        .RenderPassGuid = renderPassGuid,
                        .PipelinePackageGuid = subPass.PipelinePackageGuid,
                        .PushConstant = subPass.ShaderPushConstant,
                        .DrawMeshList = subPass.MeshType != MeshTypeEnum::kMesh_InstanceMesh ? meshList : meshSystem.DrawInstancedMesh(spriteSystem.SpriteMeshId, spriteSystem.SpriteLayerList),
                        .RenderPassInputs = subPass.InputTextureList,
                        .RenderPassOutputs = subPass.OutputTextureList,
                        .OffScreenRenderPass = subPass.OffScreenFrameBuffer
                    });
            }
            vulkanDrawMessageList.emplace_back(vulkanSubPassMessageList);
        }
        renderPassNodeList.emplace_back(RenderPassNode
            {
               .RenderPassGuid = renderPassGuid,
               .SubPassDrawMessage = vulkanDrawMessageList,
               .MipCount = maxMipLevelCount
            });
    }
    return renderPassNodeList;
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
            .VertexDataSize = LevelLayerList[x].VertexList.size() * sizeof(Vertex2DLayout),
            .VertexData = LevelLayerList[x].VertexList.data()
        };
        meshSystem.CreateMesh("__LevelMesh__", MeshTypeEnum::kMesh_StaticMesh, vertexData, LevelLayerList[x].IndexList, LevelLayerList[x].MaterialId);
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
        .VertexDataSize = skyBoxVertices.size() * sizeof(SkyboxVertexLayout),
        .VertexData = skyBoxVertices.data()
    };

    meshSystem.CreateMesh("__SkyBoxMesh__", MeshTypeEnum::kMesh_StaticMesh, vertexData, indexList, VkGuid());
}

void LevelSystem::RenderFrameBuffer(VkCommandBuffer& commandBuffer, VkGuid& renderPassId)
{
    Texture& srcTexture = textureSystem.FindRenderedTextureList(hdrRenderPassId).back();
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
                .x = srcTexture.texture.TextureSize().x,
                .y = srcTexture.texture.TextureSize().y,
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
                .x = static_cast<int>(vulkan.SwapChainResolution().width),
                .y = static_cast<int>(vulkan.SwapChainResolution().height),
                .z = 1
            }
        }
    };
    vkCmdBlitImage(commandBuffer, srcTexture.texture.TextureImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, vulkan.Swapchain().SwapChainImages()[vulkan.Swapchain().ImageIndex()], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blitRegion, VK_FILTER_LINEAR);
}

LevelTileSet LevelSystem::LoadTileSetVRAM(const char* tileSetPath, const Material& material, const Texture& tileVramTexture)
{
    nlohmann::json json = fileSystem.LoadJsonFile(tileSetPath);

    LevelTileSet tileSet = LevelTileSet();
    tileSet.TileSetId = VkGuid(json["TileSetId"].get<String>().c_str());
    tileSet.MaterialId = material.MaterialGuid;
    tileSet.TilePixelSize = ivec2{ json["TilePixelSize"][0], json["TilePixelSize"][1] };
    tileSet.TileSetBounds = ivec2{ tileVramTexture.texture.m_textureSize.x / tileSet.TilePixelSize.x,  tileVramTexture.texture.m_textureSize.y / tileSet.TilePixelSize.y};
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
