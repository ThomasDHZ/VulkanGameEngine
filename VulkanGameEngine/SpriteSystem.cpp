#include "SpriteSystem.h"
#include "RenderSystem.h"
#include "BufferSystem.h"
#include "TextureSystem.h"
#include "GameObjectSystem.h"
#include "LevelSystem.h"
#include "MaterialSystem.h"
#include "MeshSystem.h"
#include <limits>
#include <algorithm>

uint NextSpriteIndex = 0;
SpriteSystem spriteSystem = SpriteSystem();

SpriteSystem::SpriteSystem()
{
    SpriteList.reserve(10000);
    SpriteInstanceList.reserve(10000);
    SpriteLayerList.reserve(10000);
}

SpriteSystem::~SpriteSystem()
{

}

void SpriteSystem::UpdateSprites(const float& deltaTime)
{
    for (auto& sprite : SpriteList)
    {
        const auto& transform2D = gameObjectSystem.FindTransform2DComponent(sprite.GameObjectId);
        const auto& vram = FindSpriteVram(sprite.SpriteVramId);
        const auto& animation = FindSpriteAnimation(vram.VramSpriteID, sprite.CurrentAnimationID);
        const auto& material = materialSystem.FindMaterial(vram.SpriteMaterialID);
        const auto& currentFrame = animation.FrameList[sprite.CurrentFrame];
        SpriteInstanceList[sprite.SpriteInstance] = Sprite_UpdateSprites(transform2D, vram, animation, material, currentFrame, sprite, animation.FrameCount, deltaTime);
    }
}

void SpriteSystem::UpdateSpriteBatchLayers(const float& deltaTime)
{
    for (auto& spriteLayer : SpriteLayerList)
    {
        Vector<SpriteInstance> spriteInstanceList = FindSpriteInstancesByLayer(spriteLayer);
        bufferSystem.UpdateBufferMemory(renderSystem.renderer, spriteLayer.SpriteLayerBufferId, spriteInstanceList);
    }
}

void SpriteSystem::AddSprite(GameObjectID gameObjectId, VkGuid& spriteVramId)
{
    Sprite sprite;
    sprite.SpriteID = NextSpriteId++;
    sprite.GameObjectId = gameObjectId;
    sprite.SpriteVramId = spriteVramId;
    sprite.SpriteLayer = FindSpriteVram(spriteVramId).SpriteLayer + 1;
    sprite.SpriteInstance = SpriteInstanceList.size();
    SpriteList.emplace_back(sprite);
    SpriteInstanceList.emplace_back(SpriteInstance());
}

void SpriteSystem::AddSpriteBatchLayer(RenderPassGuid& renderPassId)
{
    SpriteLayer spriteLayer = SpriteLayer
    {
        .RenderPassId = renderPassId,
        .SpriteLayerId = ++NextSpriteLayerID,
        .SpriteLayerMeshId = meshSystem.CreateSpriteLayerMesh(gameObjectSystem.SpriteVertexList, gameObjectSystem.SpriteIndexList)
    };
    Vector<SpriteInstance> spriteInstanceList = FindSpriteInstancesByLayer(spriteLayer);
    spriteLayer.SpriteLayerBufferId = bufferSystem.CreateVulkanBuffer<SpriteInstance>(renderSystem.renderer, spriteInstanceList, MeshBufferUsageSettings, MeshBufferPropertySettings, false);
    SpriteLayerList.emplace_back(spriteLayer);
}

void SpriteSystem::Update(const float& deltaTime)
{
    VkCommandBuffer commandBuffer = renderSystem.BeginSingleTimeCommands();
    UpdateSprites(deltaTime);
    UpdateSpriteBatchLayers(deltaTime);
    renderSystem.EndSingleTimeCommands(commandBuffer);
}

void SpriteSystem::SetSpriteAnimation(Sprite* sprite, Sprite::SpriteAnimationEnum spriteAnimation)
{

    Sprite_SetSpriteAnimation(*sprite, spriteAnimation);
}

Sprite* SpriteSystem::FindSprite(GameObjectID gameObjectId)
{
    auto it = std::find_if(SpriteList.begin(), SpriteList.end(), [gameObjectId](const Sprite& sprite) { return sprite.GameObjectId == gameObjectId; });
    return it != SpriteList.end() ? &(*it) : nullptr;
}

Vector<std::reference_wrapper<Sprite>> SpriteSystem::FindSpritesByLayer(const SpriteLayer& spriteLayer) 
{
    Vector<std::reference_wrapper<Sprite>> matches;
    auto predicate = [spriteLayer](const Sprite& sprite) 
        {
            return sprite.SpriteLayer == spriteLayer.SpriteLayerId;
        };
    auto it = SpriteList.begin();
    while ((it = std::find_if(it, SpriteList.end(), predicate)) != SpriteList.end()) {
        matches.emplace_back(std::ref(*it)); 
        ++it;  
    }
    return matches;
}

const Vector<SpriteInstance> SpriteSystem::FindSpriteInstancesByLayer(const SpriteLayer& spriteLayer)
{
    Vector<SpriteInstance> spriteInstanceList;
    Vector<std::reference_wrapper<Sprite>> spriteList = FindSpritesByLayer(spriteLayer);
    for (auto& sprite : spriteList)
    {
        spriteInstanceList.emplace_back(SpriteInstanceList[sprite.get().SpriteInstance]);
    }
    return spriteInstanceList;
}

const SpriteVram& SpriteSystem::FindSpriteVram(VkGuid vramSpriteId)
{
    return *std::find_if(SpriteVramList.begin(), SpriteVramList.end(), [vramSpriteId](const SpriteVram& sprite) { return sprite.VramSpriteID == vramSpriteId; });
}

const Animation2D& SpriteSystem::FindSpriteAnimation(const VramSpriteGuid& vramId, const UM_AnimationListID& animationId)
{
    return SpriteAnimationMap.at(vramId)[animationId];
}

Vector<SpriteLayer> SpriteSystem::FindSpriteLayer(RenderPassGuid& guid)
{
    Vector<SpriteLayer> matchingLayers;
    std::copy_if(SpriteLayerList.begin(), SpriteLayerList.end(),
        std::back_inserter(matchingLayers),
        [guid](const SpriteLayer& sprite) { return sprite.RenderPassId == guid; });
    return matchingLayers;
}

VkGuid SpriteSystem::LoadSpriteVRAM(const String& spriteVramPath)
{

    nlohmann::json json = Json::ReadJson(spriteVramPath);
    VkGuid vramId = VkGuid(json["VramSpriteId"].get<String>().c_str());

    auto it = std::find_if(SpriteVramList.begin(), SpriteVramList.end(),
        [vramId](const SpriteVram& sprite)
        {
            return sprite.VramSpriteID == vramId;
        });
    if (it != SpriteVramList.end())
    {
        return vramId;
    }

    size_t animationListCount = 0;
    VkGuid materialId = VkGuid(json["MaterialId"].get<String>().c_str());
    const Material& material = materialSystem.FindMaterial(materialId);
    const Texture& texture = textureSystem.FindTexture(material.AlbedoMapId);
    Animation2D* animationListPtr = VRAM_LoadSpriteAnimations(spriteVramPath.c_str(), animationListCount);
    SpriteAnimationMap[vramId] = Vector<Animation2D>(animationListPtr, animationListPtr + animationListCount);
    SpriteVramList.emplace_back(VRAM_LoadSpriteVRAM(spriteVramPath.c_str(), material, texture));
    memorySystem.RemovePtrBuffer(animationListPtr);
    return vramId;
}

void SpriteSystem::Destroy()
{
    for (auto& spriteAnimationList : SpriteAnimationMap)
    {
        for (auto& spriteAnimation : spriteAnimationList.second)
        {
            memorySystem.RemovePtrBuffer(spriteAnimation.FrameList);
        }
    }
}