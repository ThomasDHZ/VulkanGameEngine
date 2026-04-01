#pragma once
#include "pch.h"
#include "Camera.h"
#include "SpriteSystem.h"
#include <entt/entt.hpp>

#ifndef PLATFORM_ANDROID
#pragma comment(lib, "vulkan-1.lib")
#endif

struct Tile
{
    uint                 TileId = 0;
    ivec2                TileUVCellOffset = ivec2();
    vec2                 TileUVOffset = vec2();
    int	                 TileLayer = 0;
    TileColliderTypeEnum TileCollider = kTileColliderNone;
    bool                 IsAnimatedTile = false;
};

struct GameObjectLoader
{
    String GameObjectPath;
    Vector<vec2> GameObjectPositionOverride;
};

struct LevelLoader
{
    VkGuid LevelID;
    Vector<String>           LoadTextures;
    Vector<String>           LoadMaterials;
    Vector<String>           LoadSpriteVRAM;
    Vector<String>           LoadTileSetVRAM;
    Vector<GameObjectLoader> GameObjectList;
    String                   LoadLevelLayout;
};

struct LevelLayer
{
    VkGuid				     LevelId = VkGuid();
    uint				     MeshId;
    VkGuid				     MaterialId = VkGuid();
    VkGuid				     TileSetId = VkGuid();
    int					     LevelLayerIndex;
    ivec2				     LevelBounds;
    Vector<uint>             TileIdMap;
    Vector<Tile>             TileMap;
    Vector<Vertex2DLayout>   VertexList;
    Vector<uint32>           IndexList;
};

struct LevelTileSet
{
    VkGuid			  TileSetId = VkGuid();
    VkGuid			  MaterialId = VkGuid();
    vec2			  TilePixelSize = vec2();
    ivec2			  TileSetBounds = ivec2();
    vec2			  TileScale = vec2(5.0f);
    vec2			  TileUVSize = vec2();
    Tile*             LevelTileListPtr = nullptr;
    size_t            LevelTileCount = 0;
};

struct LevelLayout
{
    VkGuid					  LevelLayoutId;
    ivec2					  LevelBounds;
    ivec2					  TileSizeinPixels;
};

class LevelSystem
{
public:
    static LevelSystem& Get();

private:
    template <typename T>
    ComponentTypeEnum GetComponentEnum()
    {
        if constexpr (std::is_same_v<T, InputComponent>)         return kInputComponent;
        if constexpr (std::is_same_v<T, SpriteComponent>)        return kSpriteComponent;
        if constexpr (std::is_same_v<T, Transform2DComponent>)   return kTransform2DComponent;
        if constexpr (std::is_same_v<T, Transform3DComponent>)   return kTransform3DComponent;
        if constexpr (std::is_same_v<T, CameraFollowComponent>)  return kCameraFollowComponent;
        return kUndefined;
    }

private:
    LevelSystem() = default;
    ~LevelSystem() = default;
    LevelSystem(const LevelSystem&) = delete;
    LevelSystem& operator=(const LevelSystem&) = delete;
    LevelSystem(LevelSystem&&) = delete;
    LevelSystem& operator=(LevelSystem&&) = delete;

    bool WireframeModeFlag = false;

    LevelLayer                                 LoadLevelInfo(VkGuid& levelId, const LevelTileSet& tileSet, uint* tileIdMap, size_t tileIdMapCount, ivec2& levelBounds, int levelLayerIndex);
    VkGuid                                     LoadTileSetVRAM(const char* tileSetPath);
    void                                       LoadLevelLayout(const char* levelLayoutPath);
    LevelTileSet                               LoadTileSetVRAM(const char* tileSetPath, const Material& material, const Texture& tileVramTexture);
    void                                       LoadTileSets(const char* tileSetPath, LevelTileSet& levelTileSet);
    LevelLayout                                LoadLevelInfo(const char* levelLayoutPath);
    Vector<Vector<uint>>                       LoadLevelLayout(const String& levelLayoutPath);
    void                                       LoadLevelMesh(VkGuid& tileSetId);
    void                                       LoadSkyBox();

public:

    entt::registry                             EntityRegistry;
    LevelLayout                                levelLayout;
    Vector<LevelLayer>                         LevelLayerList;
    Vector<Vector<uint>>                       LevelTileMapList;
    UnorderedMap<RenderPassGuid, LevelTileSet> LevelTileSetMap;

    SharedPtr<Camera>                          PerspectiveCamera;

    uint                                       SelectedGameObject = UINT32_MAX;

    int                                        UseHeightMap = 1;
    float                                      HeightScale = 0.079f;
    vec3                                       ViewDirection = vec3(-0.037f, -0.062f, 1.0f);
    RenderPassGuid                             environmentToCubeMapRenderPassId;
    RenderPassGuid                             brdfRenderPassId;
    RenderPassGuid                             irradianceMapRenderPassId;
    RenderPassGuid                             prefilterMapRenderPassId;
    RenderPassGuid                             gBufferRenderPassId;
    RenderPassGuid                             verticalGaussianBlurRenderPassId;
    RenderPassGuid                             horizontalGaussianBlurRenderPassId;
    RenderPassGuid                             bloomRenderPassId;
    RenderPassGuid                             hdrRenderPassId;
    RenderPassGuid                             frameBufferId;
    RenderPassGuid                             shadowDebugRenderPassId;

    RenderPassGuid                             levelWireFrameRenderPass2DId;
    RenderPassGuid                             spriteWireFrameRenderPass2DId;
    RenderPassGuid                             objectPickerRenderPassId;
    RenderPassGuid                             selectedObjectPickerRenderPassId;

    DLL_EXPORT void                            LoadLevel(const char* levelPath);
    DLL_EXPORT void                            Update(const float& deltaTime);
    DLL_EXPORT void                            RenderIrradianceMapRenderPass(VkCommandBuffer& commandBuffer, VkGuid& renderPassId, float deltaTime);
    DLL_EXPORT void                            RenderPrefilterMapRenderPass(VkCommandBuffer& commandBuffer, VkGuid& renderPassId, float deltaTime);
    DLL_EXPORT void                            RenderGBuffer(VkCommandBuffer& commandBuffer, VkGuid& renderPassId, VkGuid& levelId, const float deltaTime);
    DLL_EXPORT void                            RenderGaussianBlurPass(VkCommandBuffer& commandBuffer, VkGuid& renderPassId, uint blurDirection);
    DLL_EXPORT void                            RenderBloomPass(VkCommandBuffer& commandBuffer, VkGuid& renderPassId);
    DLL_EXPORT void                            RenderHdrPass(VkCommandBuffer& commandBuffer, VkGuid& renderPassId);
    DLL_EXPORT void                            RenderFrameBuffer(VkCommandBuffer& commandBuffer, VkGuid& renderPassId);
    DLL_EXPORT void                            RenderGameObjectPickerRenderPass(VkCommandBuffer& commandBuffer, VkGuid renderPassId);
    DLL_EXPORT void                            RenderSelectedGameObjectPickerRenderPass(VkCommandBuffer& commandBuffer, VkGuid renderPassId);

    DLL_EXPORT void                            Draw(VkCommandBuffer& commandBuffer, const float& deltaTime);
    DLL_EXPORT LevelLayout                     GetLevelLayout();
    DLL_EXPORT Vector<LevelLayer>              GetLevelLayerList();
    DLL_EXPORT Vector<Vector<uint>>            GetLevelTileMapList();
    DLL_EXPORT Vector<LevelTileSet>            GetLevelTileSetList();

    template <typename T>
    T* GetGameObjectComponent(uint gameObjectId)
    {
        GameObject& gameObject = gameObjectSystem.GameObjectList[gameObjectId];
        auto view = EntityRegistry.view<GameObjectComponentLinker, T>();
        for (auto [entity, gameObjectId, component] : view.each())
        {
            return &component;
        }
    }

    template <typename T>
    void CreateGameObjectComponent(uint32 gameObjectId, T* gameObjectComponent)
    {
        GameObject& gameObject = gameObjectSystem.GameObjectList[gameObjectId];
        EntityRegistry.emplace<T>(gameObject.GameObjectComponents, *gameObjectComponent);
    }
};
extern DLL_EXPORT LevelSystem& levelSystem;
inline LevelSystem& LevelSystem::Get()
{
    static LevelSystem instance;
    return instance;
}
