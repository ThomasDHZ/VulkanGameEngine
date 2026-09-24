MeshProperitiesBuffer GetMesh(uint index) 
{
    MeshProperitiesBuffer mesh;
    if (index >= bindlessBuffer.MeshCount) 
    {
        mesh.MaterialIndex = 0u;
        mesh.MeshTransform = mat4(0.0);
        return mesh;
    }

    uint baseByteLocation = (uint(bindlessBuffer.MeshOffset - bindlessBuffer.MeshOffset) / 4) + (index * (bindlessBuffer.MeshSize / 4));
    mesh.MaterialIndex = bindlessBuffer.Data[baseByteLocation++];
    mesh.MeshTransform = mat4(
        bindlessBuffer.Data[baseByteLocation++],  bindlessBuffer.Data[baseByteLocation++],  bindlessBuffer.Data[baseByteLocation++],  bindlessBuffer.Data[baseByteLocation++],
        bindlessBuffer.Data[baseByteLocation++],  bindlessBuffer.Data[baseByteLocation++],  bindlessBuffer.Data[baseByteLocation++],  bindlessBuffer.Data[baseByteLocation++],
        bindlessBuffer.Data[baseByteLocation++],  bindlessBuffer.Data[baseByteLocation++], bindlessBuffer.Data[baseByteLocation++], bindlessBuffer.Data[baseByteLocation++],
        bindlessBuffer.Data[baseByteLocation++], bindlessBuffer.Data[baseByteLocation++], bindlessBuffer.Data[baseByteLocation++], bindlessBuffer.Data[baseByteLocation++]);
    return mesh;
}

PackedMaterial GetMaterial(uint index)
{
    PackedMaterial mat;
    mat.AlbedoTextureId                                      = ~0u;
    mat.NormalTextureId                                      = ~0u;
    mat.MROTextureId                                         = ~0u;
    mat.ClearCoatOrTranslucentTextureId                      = ~0u;
    mat.SubSurfaceScatteringOrTranslucentPropertiesTextureId = ~0u;
    mat.SubSurfaceScatteringPropertiesTextureId              = ~0u;
    mat.SheenTextureId                                       = ~0u;
    mat.AnisotropyTextureId                                  = ~0u;
    mat.EmissionTextureId                                    = ~0u;
    mat.ShadingModel                                         = ~0u;
    mat.FeatureMask                                          = ~0u;
    mat.ClearcoatTint                                        = vec3(0.0f);
    mat.IOR                                                  = 0.0f;
    mat.AlphaCutOff                                          = 0.0f;
    if (index >= bindlessBuffer.MaterialCount) return mat;

    uint offset                                              = (uint(bindlessBuffer.MaterialOffset - bindlessBuffer.MeshOffset) / 4u) + index * (bindlessBuffer.MaterialSize / 4u);
    mat.AlbedoTextureId                                      = bindlessBuffer.Data[offset++];
    mat.NormalTextureId                                      = bindlessBuffer.Data[offset++];
    mat.MROTextureId                                         = bindlessBuffer.Data[offset++];
    mat.ClearCoatOrTranslucentTextureId                      = bindlessBuffer.Data[offset++];
    mat.SubSurfaceScatteringOrTranslucentPropertiesTextureId = bindlessBuffer.Data[offset++];
    mat.SubSurfaceScatteringPropertiesTextureId              = bindlessBuffer.Data[offset++];
    mat.SheenTextureId                                       = bindlessBuffer.Data[offset++];
    mat.AnisotropyTextureId                                  = bindlessBuffer.Data[offset++];
    mat.EmissionTextureId                                    = bindlessBuffer.Data[offset++];
    mat.TranslucentTextureId                                 = bindlessBuffer.Data[offset++];
    mat.TranslucentPropertiesTextureId                       = bindlessBuffer.Data[offset++];
    mat.ShadingModel                                         = bindlessBuffer.Data[offset++];
    mat.FeatureMask                                          = bindlessBuffer.Data[offset++];
    mat.ClearcoatTint                                        = vec3(uintBitsToFloat(bindlessBuffer.Data[offset++]), uintBitsToFloat(bindlessBuffer.Data[offset++]), uintBitsToFloat(bindlessBuffer.Data[offset++]));
    mat.IOR                                                  = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.AlphaCutOff                                          = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    return mat;
}

DirectionalLightBuffer GetDirectionalLight(uint index) 
{
    DirectionalLightBuffer light;
    if (index >= bindlessBuffer.DirectionalLightCount) 
    {
        light.LightColor     = vec3(0.0);
        light.LightDirection = vec3(0.0);
        light.LightIntensity = 0.0;
        light.ShadowStrength = 0.0;
        light.ShadowBias     = 0.0;
        light.ShadowSoftness = 0.0;
        return light;
    }

    uint baseByteLocation = (uint(bindlessBuffer.DirectionalLightOffset - bindlessBuffer.MeshOffset) / 4) + (index * (bindlessBuffer.DirectionalLightSize / 4));
    light.LightColor     = vec3(uintBitsToFloat(bindlessBuffer.Data[baseByteLocation++]), uintBitsToFloat(bindlessBuffer.Data[baseByteLocation++]), uintBitsToFloat(bindlessBuffer.Data[baseByteLocation++]));
    light.LightDirection = vec3(uintBitsToFloat(bindlessBuffer.Data[baseByteLocation++]), uintBitsToFloat(bindlessBuffer.Data[baseByteLocation++]), uintBitsToFloat(bindlessBuffer.Data[baseByteLocation++]));
    light.LightIntensity = uintBitsToFloat(bindlessBuffer.Data[baseByteLocation++]);
    light.ShadowStrength = uintBitsToFloat(bindlessBuffer.Data[baseByteLocation++]);
    light.ShadowBias     = uintBitsToFloat(bindlessBuffer.Data[baseByteLocation++]);
    light.ShadowSoftness = uintBitsToFloat(bindlessBuffer.Data[baseByteLocation++]);
    return light;
}

PointLightBuffer GetPointLight(uint index) 
{
    PointLightBuffer light;
    if (index >= bindlessBuffer.PointLightCount) 
    {
        light.LightPosition  = vec3(0.0);
        light.LightColor     = vec3(0.0);
        light.LightRadius    = 0.0;
        light.LightIntensity = 0.0;
        light.ShadowStrength = 0.0;
        light.ShadowBias     = 0.0;
        light.ShadowSoftness = 0.0;
        return light;
    }

    uint baseByteLocation = (uint(bindlessBuffer.PointLightOffset - bindlessBuffer.MeshOffset) / 4) + (index * (bindlessBuffer.PointLightSize / 4));
    light.LightPosition  = vec3(uintBitsToFloat(bindlessBuffer.Data[baseByteLocation++]), uintBitsToFloat(bindlessBuffer.Data[baseByteLocation++]), uintBitsToFloat(bindlessBuffer.Data[baseByteLocation++]));
    light.LightColor     = vec3(uintBitsToFloat(bindlessBuffer.Data[baseByteLocation++]), uintBitsToFloat(bindlessBuffer.Data[baseByteLocation++]), uintBitsToFloat(bindlessBuffer.Data[baseByteLocation++]));
    light.LightRadius    = uintBitsToFloat(bindlessBuffer.Data[baseByteLocation++]);
    light.LightIntensity = uintBitsToFloat(bindlessBuffer.Data[baseByteLocation++]);
    light.ShadowStrength = uintBitsToFloat(bindlessBuffer.Data[baseByteLocation++]);
    light.ShadowBias     = uintBitsToFloat(bindlessBuffer.Data[baseByteLocation++]);
    light.ShadowSoftness = uintBitsToFloat(bindlessBuffer.Data[baseByteLocation++]);
    return light;
}

TextureMetadata Get2DTextureMetadata(uint index)
{
    TextureMetadata textureMetaData;
    if (index >= bindlessBuffer.Texture2DCount) 
    {
        textureMetaData.Width       = 0;
        textureMetaData.Height      = 0;
        textureMetaData.Depth       = 0;
        textureMetaData.MipLevels   = 1;
        textureMetaData.LayerCount  = 1;
        textureMetaData.Format      = 0;
        textureMetaData.TextureType = 0;
        textureMetaData.ArrayIndex  = 0;
        return textureMetaData;
    }

    uint baseByteLocation = (uint(bindlessBuffer.Texture2DOffset - bindlessBuffer.MeshOffset) / 4) + (index * (bindlessBuffer.Texture2DSize / 4));
    textureMetaData.Width       = bindlessBuffer.Data[baseByteLocation++];
    textureMetaData.Height      = bindlessBuffer.Data[baseByteLocation++];
    textureMetaData.Depth       = bindlessBuffer.Data[baseByteLocation++];
    textureMetaData.MipLevels   = bindlessBuffer.Data[baseByteLocation++];
    textureMetaData.LayerCount  = bindlessBuffer.Data[baseByteLocation++];
    textureMetaData.Format      = bindlessBuffer.Data[baseByteLocation++];
    textureMetaData.TextureType = bindlessBuffer.Data[baseByteLocation++];
    textureMetaData.ArrayIndex  = bindlessBuffer.Data[baseByteLocation++];
    return textureMetaData;
}

TextureMetadata Get3DTextureMetadata(uint index)
{
    TextureMetadata textureMetaData;
    if (index >= bindlessBuffer.Texture3DCount) 
    {
        textureMetaData.Width       = 0;
        textureMetaData.Height      = 0;
        textureMetaData.Depth       = 0;
        textureMetaData.MipLevels   = 1;
        textureMetaData.LayerCount  = 1;
        textureMetaData.Format      = 0;
        textureMetaData.TextureType = 0;
        textureMetaData.ArrayIndex  = 0;
        return textureMetaData;
    }

    uint baseByteLocation = (uint(bindlessBuffer.Texture3DOffset - bindlessBuffer.MeshOffset) / 4) + (index * (bindlessBuffer.Texture3DSize / 4));
    textureMetaData.Width       = bindlessBuffer.Data[baseByteLocation++];
    textureMetaData.Height      = bindlessBuffer.Data[baseByteLocation++];
    textureMetaData.Depth       = bindlessBuffer.Data[baseByteLocation++];
    textureMetaData.MipLevels   = bindlessBuffer.Data[baseByteLocation++];
    textureMetaData.LayerCount  = bindlessBuffer.Data[baseByteLocation++];
    textureMetaData.Format      = bindlessBuffer.Data[baseByteLocation++];
    textureMetaData.TextureType = bindlessBuffer.Data[baseByteLocation++];
    textureMetaData.ArrayIndex  = bindlessBuffer.Data[baseByteLocation++];
    return textureMetaData;
}

TextureMetadata GetCubeMapTextureMetadata(uint index)
{
    TextureMetadata textureMetaData;
    if (index >= bindlessBuffer.TextureCubeMapCount) 
    {
        textureMetaData.Width       = 0;
        textureMetaData.Height      = 0;
        textureMetaData.Depth       = 0;
        textureMetaData.MipLevels   = 1;
        textureMetaData.LayerCount  = 1;
        textureMetaData.Format      = 0;
        textureMetaData.TextureType = 0;
        textureMetaData.ArrayIndex  = 0;
        return textureMetaData;
    }

    uint baseByteLocation = (uint(bindlessBuffer.TextureCubeMapOffset - bindlessBuffer.MeshOffset) / 4) + (index * (bindlessBuffer.TextureCubeMapSize / 4));
    textureMetaData.Width       = bindlessBuffer.Data[baseByteLocation++];
    textureMetaData.Height      = bindlessBuffer.Data[baseByteLocation++];
    textureMetaData.Depth       = bindlessBuffer.Data[baseByteLocation++];
    textureMetaData.MipLevels   = bindlessBuffer.Data[baseByteLocation++];
    textureMetaData.LayerCount  = bindlessBuffer.Data[baseByteLocation++];
    textureMetaData.Format      = bindlessBuffer.Data[baseByteLocation++];
    textureMetaData.TextureType = bindlessBuffer.Data[baseByteLocation++];
    textureMetaData.ArrayIndex  = bindlessBuffer.Data[baseByteLocation++];
    return textureMetaData;
}