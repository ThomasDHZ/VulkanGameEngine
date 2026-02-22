//MeshProperitiesBuffer GetMesh(uint index) 
//{
//    MeshProperitiesBuffer mesh;
//    if (index >= meshBuffer.MeshCount) 
//    {
//        mesh.MaterialIndex = 0u;
//        mesh.MeshTransform = mat4(0.0);
//        return mesh;
//    }
//
//    const uint baseByteLocation = (bindlessBuffer.MeshOffset/4)+ (index * (bindlessBuffer.MeshSize / 4));
//    mesh.MaterialIndex = bindlessBuffer.Data[baseByteLocation + 0u];
//    mesh.MeshTransform = mat4(
//        bindlessBuffer.Data[baseByteLocation + 1u],  bindlessBuffer.Data[baseByteLocation + 2u],  bindlessBuffer.Data[baseByteLocation + 3u],  bindlessBuffer.Data[baseByteLocation + 4u],
//        bindlessBuffer.Data[baseByteLocation + 5u],  bindlessBuffer.Data[baseByteLocation + 6u],  bindlessBuffer.Data[baseByteLocation + 7u],  bindlessBuffer.Data[baseByteLocation + 8u],
//        bindlessBuffer.Data[baseByteLocation + 9u],  bindlessBuffer.Data[baseByteLocation + 10u], bindlessBuffer.Data[baseByteLocation + 11u], bindlessBuffer.Data[baseByteLocation + 12u],
//        bindlessBuffer.Data[baseByteLocation + 13u], bindlessBuffer.Data[baseByteLocation + 14u], bindlessBuffer.Data[baseByteLocation + 15u], bindlessBuffer.Data[baseByteLocation + 16u]);
//
//    return mesh;
//}
//
//PackedMaterial GetMaterial(uint index)
//{
//    PackedMaterial mat;
//    mat.AlbedoDataId          = ~0u;
//    mat.NormalDataId          = ~0u;
//    mat.PackedMRODataId       = ~0u;
//    mat.PackedSheenSSSDataId  = ~0u;
//    mat.UnusedDataId          = ~0u;
//    mat.EmissionDataId        = ~0u;
//    if (index >= materialBuffer.MaterialCount)
//    {
//        return mat;
//    }
//
//    const uint baseByteLocation = (bindlessBuffer.MaterialOffset/4)+ (index * (bindlessBuffer.MaterialSize / 4));
//    mat.AlbedoDataId          = bindlessBuffer.Data[baseByteLocation + 0u];
//    mat.NormalDataId          = bindlessBuffer.Data[baseByteLocation + 1u];
//    mat.PackedMRODataId       = bindlessBuffer.Data[baseByteLocation + 2u];
//    mat.PackedSheenSSSDataId  = bindlessBuffer.Data[baseByteLocation + 3u];
//    mat.UnusedDataId          = bindlessBuffer.Data[baseByteLocation + 4u];
//    mat.EmissionDataId        = bindlessBuffer.Data[baseByteLocation + 5u];
//    return mat;
//}
//
//DirectionalLightBuffer GetDirectionalLight(uint index) 
//{
//    DirectionalLightBuffer light;
//    if (index >= directionalLightBuffer.LightCount) 
//    {
//        light.LightColor     = vec3(0.0);
//        light.LightDirection = vec3(0.0);
//        light.LightIntensity = 0.0;
//        light.ShadowStrength = 0.0;
//        light.ShadowBias     = 0.0;
//        light.ShadowSoftness = 0.0;
//        return light;
//    }
//
//    const uint baseByteLocation = (bindlessBuffer.DirectionalLightOffset/4)+ (index * (bindlessBuffer.DirectionalLightSize / 4));
//    light.LightColor     = vec3(uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 0u]), uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 1u]), uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 2u]));
//    light.LightDirection = vec3(uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 3u]), uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 4u]), uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 5u]));
//    light.LightIntensity = uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 6u]);
//    light.ShadowStrength = uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 7u]);
//    light.ShadowBias     = uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 8u]);
//    light.ShadowSoftness = uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 9u]);
//    return light;
//}
//
//PointLightBuffer GetPointLight(uint index) 
//{
//    PointLightBuffer light;
//    if (index >= pointLightBuffer.LightCount) 
//    {
//        light.LightPosition  = vec3(0.0);
//        light.LightColor     = vec3(0.0);
//        light.LightRadius    = 0.0;
//        light.LightIntensity = 0.0;
//        light.ShadowStrength = 0.0;
//        light.ShadowBias     = 0.0;
//        light.ShadowSoftness = 0.0;
//        return light;
//    }
//
//    const uint baseByteLocation = (bindlessBuffer.PointLightOffset/4)+ (index * (bindlessBuffer.PointLightSize / 4));
//    light.LightPosition  = vec3(uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 0u]), uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 1u]), uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 2u]));
//    light.LightColor     = vec3(uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 3u]), uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 4u]), uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 5u]));
//    light.LightRadius    = uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 6u]);
//    light.LightIntensity = uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 7u]);
//    light.ShadowStrength = uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 8u]);
//    light.ShadowBias     = uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 9u]);
//    light.ShadowSoftness = uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 10u]);
//    return light;
//}