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

SpriteSystem spriteSystem = SpriteSystem();

SpriteSystem::SpriteSystem()
{
    SpriteList.reserve(10000);
    SpriteInstanceList.reserve(10000);
    SpriteIdToListIndexMap.reserve(10000);
    SpriteInstanceBufferIdMap.reserve(10000);
    SpriteBatchLayerList.reserve(10000);
}

SpriteSystem::~SpriteSystem()
{

}

void SpriteSystem::UpdateBatchSprites(const float& deltaTime)
{
    const size_t count = SpriteInstanceList.size();
    Vector<Transform2DComponent> transform2D(count);
    Vector<SpriteVram> vram(count);
    Vector<Animation2D> animation(count);
    Vector<AnimationFrames> frameList(count);
    Vector<Material> material(count);

    for (size_t x = 0; x < count; ++x)
    {
        const auto& sprite = SpriteList[x];
        transform2D[x] = gameObjectSystem.FindTransform2DComponent(sprite.GameObjectId);
        vram[x] = FindVramSprite(sprite.SpriteVramId);
        animation[x] = FindSpriteAnimation(sprite.SpriteVramId, sprite.CurrentAnimationID);
        material[x] = materialSystem.FindMaterial(vram[x].SpriteMaterialID);
    }
    Sprite_UpdateBatchSprites(SpriteInstanceList.data(), SpriteList.data(), transform2D.data(), vram.data(), animation.data(), material.data(), count, deltaTime);
}

void SpriteSystem::UpdateSprites(const float& deltaTime)
{
    for (size_t x = 0; x < SpriteList.size(); ++x)
    {
        Sprite& sprite = SpriteList[x];
        const auto& transform2D = gameObjectSystem.FindTransform2DComponent(sprite.GameObjectId);
        const auto& vram = FindVramSprite(sprite.SpriteVramId);
        const auto& animation = FindSpriteAnimation(vram.VramSpriteID, sprite.CurrentAnimationID);
        const auto& material = materialSystem.FindMaterial(vram.SpriteMaterialID);
        const auto& currentFrame = animation.FrameList[sprite.CurrentFrame];
        SpriteInstanceList[x] = Sprite_UpdateSprites(transform2D, vram, animation, material, currentFrame, sprite, animation.FrameCount, deltaTime);
    }
}

void SpriteSystem::UpdateSpriteBatchLayers(const float& deltaTime)
{
    for (auto& spriteBatchLayer : SpriteBatchLayerList)
    {
        Vector<SpriteInstanceStruct>& spriteInstanceStructList = FindSpriteInstanceList(spriteBatchLayer.SpriteBatchLayerID);
        Vector<GameObjectID> spriteBatchObjectList = FindSpriteBatchObjectListMap(spriteBatchLayer.SpriteBatchLayerID);

        spriteInstanceStructList.clear();
        spriteInstanceStructList.reserve(spriteBatchObjectList.size());
        for (auto& gameObjectID : spriteBatchObjectList)
        {
            SpriteInstanceStruct spriteInstanceStruct = *FindSpriteInstance(gameObjectID);
            spriteInstanceStructList.emplace_back(spriteInstanceStruct);
        }

        if (spriteBatchObjectList.size())
        {
            uint bufferId = FindSpriteInstanceBufferId(spriteBatchLayer.SpriteBatchLayerID);
            bufferSystem.UpdateBufferMemory(renderSystem.renderer, bufferId, spriteInstanceStructList);
        }
    }
}

void SpriteSystem::AddSprite(GameObjectID gameObjectId, VkGuid& spriteVramId)
{
    Sprite sprite;
    sprite.GameObjectId = gameObjectId;
    sprite.SpriteVramId = spriteVramId;
    SpriteList.emplace_back(sprite);
    SpriteInstanceList.emplace_back(SpriteInstanceStruct());
    SpriteIdToListIndexMap[gameObjectId] = SpriteList.size() - 1;
}

void SpriteSystem::AddSpriteBatchLayer(RenderPassGuid& renderPassId)
{
    SpriteBatchLayerList.emplace_back(SpriteBatchLayer(renderPassId));
}

void SpriteSystem::AddSpriteInstanceBufferId(UM_SpriteBatchID spriteInstanceBufferId, int BufferId)
{
    SpriteInstanceBufferIdMap[spriteInstanceBufferId] = BufferId;
}

void SpriteSystem::AddSpriteInstanceLayerList(UM_SpriteBatchID spriteBatchId, Vector<SpriteInstanceStruct>& spriteInstanceList)
{
    SpriteInstanceListMap[spriteBatchId] = spriteInstanceList;
}

void SpriteSystem::AddSpriteBatchObjectList(UM_SpriteBatchID spriteBatchId, GameObjectID spriteBatchObject)
{
    SpriteBatchObjectListMap[spriteBatchId].emplace_back(spriteBatchObject);
}

void SpriteSystem::Update(const float& deltaTime)
{
    if (SpriteList.size() > 100)
    {
        UpdateBatchSprites(deltaTime);
    }
    else
    {
        UpdateSprites(deltaTime);
    }

    VkCommandBuffer commandBuffer = renderSystem.BeginSingleTimeCommands();
    UpdateSpriteBatchLayers(deltaTime);
    renderSystem.EndSingleTimeCommands(commandBuffer);
}

void SpriteSystem::SetSpriteAnimation(Sprite* sprite, Sprite::SpriteAnimationEnum spriteAnimation)
{

    Sprite_SetSpriteAnimation(*sprite, spriteAnimation);
}

Sprite* SpriteSystem::FindSprite(GameObjectID gameObjectId)
{
    if (SpriteList.size() <= 200)
    {
        auto it = std::find_if(SpriteList.begin(), SpriteList.end(),
            [gameObjectId](const Sprite& sprite) { return sprite.GameObjectId == gameObjectId; });
        return it != SpriteList.end() ? &(*it) : nullptr;
    }
    else
    {
        auto it = SpriteIdToListIndexMap.find(gameObjectId);
        return it != SpriteIdToListIndexMap.end() ? &SpriteList[it->second] : nullptr;
    }
}

const SpriteVram& SpriteSystem::FindVramSprite(VkGuid vramSpriteId)
{
    auto it = std::find_if(SpriteVramList.begin(), SpriteVramList.end(),
        [vramSpriteId](const SpriteVram& sprite) { return sprite.VramSpriteID == vramSpriteId; });
    return *it;
}

const Animation2D& SpriteSystem::FindSpriteAnimation(const VramSpriteGuid& vramId, const UM_AnimationListID& animationId)
{
    return SpriteAnimationMap.at(vramId)[animationId];
}

const SpriteInstanceStruct* SpriteSystem::FindSpriteInstance(GameObjectID gameObjectId)
{
    if (SpriteInstanceList.size() <= 200)
    {
        size_t spriteInstanceIndex = FindSpriteIndex(gameObjectId);
        return &SpriteInstanceList[spriteInstanceIndex];
    }
    else
    {
        auto it = SpriteIdToListIndexMap.find(gameObjectId);
        return it != SpriteIdToListIndexMap.end() ? &SpriteInstanceList[it->second] : nullptr;
    }
}

const int SpriteSystem::FindSpriteInstanceBufferId(UM_SpriteBatchID spriteInstanceBufferId)
{
    return SpriteInstanceBufferIdMap.at(spriteInstanceBufferId);
}

Vector<SpriteInstanceStruct>& SpriteSystem::FindSpriteInstanceList(UM_SpriteBatchID spriteBatchId)
{
    return SpriteInstanceListMap.at(spriteBatchId);
}

const Vector<GameObjectID>& SpriteSystem::FindSpriteBatchObjectListMap(UM_SpriteBatchID spriteBatchObjectListId)
{
    return SpriteBatchObjectListMap.at(spriteBatchObjectListId);
}

Vector<SpriteBatchLayer> SpriteSystem::FindSpriteBatchLayer(RenderPassGuid& guid)
{
    std::vector<SpriteBatchLayer> matchingLayers;
    std::copy_if(SpriteBatchLayerList.begin(), SpriteBatchLayerList.end(),
        std::back_inserter(matchingLayers),
        [guid](const SpriteBatchLayer& sprite) { return sprite.RenderPassId == guid; });
    return matchingLayers;
}

size_t SpriteSystem::FindSpriteIndex(GameObjectID gameObjectId)
{
    auto it = std::find_if(SpriteList.begin(), SpriteList.end(),
        [gameObjectId](const Sprite& sprite) { return sprite.GameObjectId == gameObjectId; });
    return it != SpriteList.end() ? std::distance(SpriteList.begin(), it) : (std::numeric_limits<size_t>::max)();
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