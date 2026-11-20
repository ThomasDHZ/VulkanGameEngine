struct SpriteInstance
{
    vec2  SpritePosition;
    vec4  UVOffset;
    vec2  SpriteSize;
    ivec2 FlipSprite;
    vec4  Color;
    mat4  InstanceTransform;
    uint  MaterialId;
    uint  SpriteId;
    uint  SpriteLayer;
};

SpriteInstance GetSpriteInstance(MemoryPoolHeader memoryPoolHeader, uint index, uint data[])
{
    SpriteInstance sprite;
    if (index >= memoryPoolHeader.ObjectCount)
    {
        sprite.SpritePosition = vec2(0.0);
        sprite.UVOffset = vec4(0.0);
        sprite.SpriteSize = vec2(0.0);
        sprite.FlipSprite = ivec2(0);
        sprite.Color = vec4(0.0);
        sprite.InstanceTransform = mat4(1.0);
        sprite.MaterialId = 0u;
        sprite.SpriteId = 0u;
        sprite.SpriteLayer = 0u;
        return sprite;
    }

    uint offset = uint(memoryPoolHeader.FirstObjectOffset / 4u) + index * (memoryPoolHeader.ObjectSize / 4u);
    sprite.SpritePosition = Vec2Data(offset, data);
    sprite.UVOffset = Vec4Data(offset, data);
    sprite.SpriteSize = Vec2Data(offset, data);
    sprite.FlipSprite = IVec2Data(offset, data);
    sprite.Color = Vec4Data(offset, data);
    sprite.InstanceTransform = Mat4Data(offset, data);
    sprite.MaterialId = UIntData(offset, data);
    sprite.SpriteId = UIntData(offset, data);
    sprite.SpriteLayer = UIntData(offset, data);
    return sprite;
}