#pragma once
#include "TypeDef.h"

typedef uint UM_SpriteLayerID;
typedef uint UM_RenderPassID;
typedef uint32 UM_GameObjectID;
typedef uint32 UM_TextureID;
typedef uint32 UM_MaterialID;
typedef uint32 UM_RenderPassID;
typedef uint32 UM_PipelineID;
typedef uint32 UM_SpriteSheetID;
typedef uint32 UM_SpriteVRAMID;
typedef uint32 UM_AnimationFrameId;
typedef uint32 UM_AnimationListID;


struct AnimationFrameID
{
    uint id = 0;

    AnimationFrameID() = default;
    explicit AnimationFrameID(uint32_t id) : id(id) {}

    bool operator==(const AnimationFrameID& other) const
    {
        return id == other.id;
    }

    bool operator!=(const AnimationFrameID& other) const
    {
        return !(*this == other);
    }
};

struct AnimationListID
{
    uint id = 0;

    AnimationListID() = default;
    explicit AnimationListID(uint32_t id) : id(id) {}

    bool operator==(const AnimationListID& other) const
    {
        return id == other.id;
    }

    bool operator!=(const AnimationListID& other) const
    {
        return !(*this == other);
    }
};

struct RenderPassID
{
    uint id = 0;

    RenderPassID() = default;
    explicit RenderPassID(uint32_t id) : id(id) {}

    bool operator==(const RenderPassID& other) const
    {
        return id == other.id;
    }

    bool operator!=(const RenderPassID& other) const
    {
        return !(*this == other);
    }
};

struct SpriteBatchID
{
    uint id = 0;

    SpriteBatchID() = default;
    explicit SpriteBatchID(uint32_t id) : id(id) {}

    bool operator==(const SpriteBatchID& other) const
    {
        return id == other.id;
    }

    bool operator!=(const SpriteBatchID& other) const
    {
        return !(*this == other);
    }
};

struct GameObjectID
{
    uint id = 0;

    GameObjectID() = default;
    explicit GameObjectID(uint32_t id) : id(id) {}

    bool operator==(const GameObjectID& other) const
    {
        return id == other.id;
    }

    bool operator!=(const GameObjectID& other) const
    {
        return !(*this == other);
    }
};

struct SpriteMeshID
{
    uint id = 0;

    SpriteMeshID() = default;
    explicit SpriteMeshID(uint32_t id) : id(id) {}

    bool operator==(const SpriteMeshID& other) const
    {
        return id == other.id;
    }

    bool operator!=(const SpriteMeshID& other) const
    {
        return !(*this == other);
    }
};

namespace std
{
    template <>
    struct hash<RenderPassID> {
        size_t operator()(const RenderPassID& key) const {
            return std::hash<int>()(key.id);
        }
    };

    template <>
    struct hash<SpriteMeshID> {
        size_t operator()(const SpriteMeshID& key) const {
            return std::hash<int>()(key.id);
        }
    };

    template <>
    struct hash<GameObjectID> {
        size_t operator()(const GameObjectID& key) const {
            return std::hash<int>()(key.id);
        }
    };

    template <>
    struct hash<SpriteBatchID> {
        size_t operator()(const SpriteBatchID& key) const {
            return std::hash<int>()(key.id);
        }
    };
}