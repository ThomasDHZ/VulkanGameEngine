#include "LightSystem.h"
#include "FileSystem.h"
#include "BufferSystem.h"
#include "ShaderSystem.h"

LightSystem& lightSystem = LightSystem::Get();

void LightSystem::Update(const float& deltaTime)
{
    for (auto& directionalLight : DirectionalLightList)
    {
        ShaderStructDLL& shaderStruct = shaderSystem.FindShaderStruct(directionalLight.DirectionalLightBufferId);
        shaderSystem.UpdateShaderStructValue<vec3>(shaderStruct,  "LightColor", directionalLight.LightColor);
        shaderSystem.UpdateShaderStructValue<vec3>(shaderStruct,  "LightDirection", directionalLight.LightDirection);
        shaderSystem.UpdateShaderStructValue<float>(shaderStruct, "LightIntensity", directionalLight.LightIntensity);
        shaderSystem.UpdateShaderBuffer(shaderStruct, directionalLight.DirectionalLightBufferId);
    }

    for (auto& pointLight : PointLightList)
    {
        ShaderStructDLL& shaderStruct = shaderSystem.FindShaderStruct(pointLight.PointLightBufferId);
        shaderSystem.UpdateShaderStructValue<vec3>(shaderStruct, "LightPosition", pointLight.LightPosition);
        shaderSystem.UpdateShaderStructValue<vec3>(shaderStruct, "LightColor", pointLight.LightColor);
        shaderSystem.UpdateShaderStructValue<float>(shaderStruct, "LightIntensity", pointLight.LightIntensity);
        shaderSystem.UpdateShaderStructValue<float>(shaderStruct, "LightRadius", pointLight.LightRadius);
        shaderSystem.UpdateShaderBuffer(shaderStruct, pointLight.PointLightBufferId);
    }
}

void LightSystem::LoadSceneLights(const String& directionalLightPath)
{
    nlohmann::json json = fileSystem.LoadJsonFile(directionalLightPath.c_str());
    uint lightType = json["LightType"];
    switch (lightType)
    {
        case kDirectionalLight:
        {
            ShaderStructDLL shaderStruct = shaderSystem.CopyShaderStructProtoType("DirectionalLightBuffer");
            uint directionalLightBufferId = bufferSystem.VMACreateDynamicBuffer(&shaderStruct, shaderStruct.ShaderBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
            shaderSystem.PipelineShaderStructMap[directionalLightBufferId] = shaderStruct;

            DirectionalLightList.emplace_back(DirectionalLight
                {
                    .DirectionalLightBufferId = directionalLightBufferId,
                    .LightColor = vec3(json["LightColor"][0], json["LightColor"][1], json["LightColor"][2]),
                    .LightDirection = vec3(json["LightDirection"][0], json["LightDirection"][1], json["LightDirection"][2]),
                    .LightIntensity = json["LightIntensity"]
                });
            break;
        }
        case kPointLight:
        {
            ShaderStructDLL shaderStruct = shaderSystem.CopyShaderStructProtoType("PointLightBuffer");
            uint pointLightBufferId = bufferSystem.VMACreateDynamicBuffer(&shaderStruct, shaderStruct.ShaderBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
            shaderSystem.PipelineShaderStructMap[pointLightBufferId] = shaderStruct;

            PointLightList.emplace_back(PointLight
                {
                    .PointLightBufferId = pointLightBufferId,
                    .LightPosition = vec3(json["LightPosition"][0], json["LightPosition"][1], json["LightPosition"][2]),
                    .LightColor = vec3(json["LightColor"][0], json["LightColor"][1], json["LightColor"][2]),
                    .LightRadius = json["LightRadius"],
                    .LightIntensity = json["LightIntensity"]
                });
            break;
        }
    }
}

void LightSystem::UpdateDirectionalLightOrthographicView(const DirectionalLight& directionalLight)
{
    vec3 lightDir = normalize(directionalLight.LightDirection);
    mat4 lightProjection = glm::ortho(-1000.0f, 1000.0f, -1000.0f, 1000.0f);
    mat4 lightView = inverse(lookAt(vec3(0), lightDir, vec3(0, 1, 0)));

    auto it = std::find_if(DirectionalLightList.begin(), DirectionalLightList.end(),
        [directionalLight](const DirectionalLight& obj) {
            return obj == directionalLight;
        });
    int lightIndex = std::distance(DirectionalLightList.begin(), it);

    shaderSystem.UpdatePushConstantValue<int>("spfDirectionalLightPushConstant",  "LightBufferIndex", lightIndex);
    shaderSystem.UpdatePushConstantValue<mat4>("spfDirectionalLightPushConstant", "LightProjection", lightProjection);
    shaderSystem.UpdatePushConstantValue<mat4>("spfDirectionalLightPushConstant", "LightView", lightView);
    shaderSystem.UpdatePushConstantValue<vec3>("spfDirectionalLightPushConstant", "LightDirection", directionalLight.LightDirection);
    shaderSystem.UpdatePushConstantBuffer("spfDirectionalLightPushConstant");
}

void LightSystem::UpdatePointLightOrthographicView(const PointLight& pointLight)
{
    float size = pointLight.LightRadius * 2.0f;

    mat4 lightProjection = glm::ortho(
        pointLight.LightPosition.x - size * 0.5f,
        pointLight.LightPosition.x + size * 0.5f, 
        pointLight.LightPosition.y - size * 0.5f,
        pointLight.LightPosition.y + size * 0.5f,
        -100.0f, 100.0f
    );


    //float size = 2000.0f;
    //mat4 lightProjection = glm::ortho(-size, size, -size, size, -100.0f, 100.0f);
    mat4 lightView = mat4(1.0f);

    auto it = std::find_if(PointLightList.begin(), PointLightList.end(),
        [pointLight](const PointLight& obj) {
            return obj == pointLight;
        });
    int lightIndex = std::distance(PointLightList.begin(), it);

    shaderSystem.UpdatePushConstantValue<int>("spfPointLightPushConstant", "LightBufferIndex", lightIndex);
    shaderSystem.UpdatePushConstantValue<mat4>("spfPointLightPushConstant", "LightProjection", lightProjection);
    shaderSystem.UpdatePushConstantValue<mat4>("spfPointLightPushConstant", "LightView", lightView);
    shaderSystem.UpdatePushConstantBuffer("spfPointLightPushConstant");
}

const Vector<VkDescriptorBufferInfo> LightSystem::GetDirectionalLightPropertiesBuffer()
{
    Vector<VkDescriptorBufferInfo> directionalLightPropertiesBuffer;
    if (DirectionalLightList.empty())
    {
        directionalLightPropertiesBuffer.emplace_back(VkDescriptorBufferInfo
            {
                .buffer = VK_NULL_HANDLE,
                .offset = 0,
                .range = VK_WHOLE_SIZE
            });
    }
    else
    {
        for (auto& directionalLight : DirectionalLightList)
        {
            VkDescriptorBufferInfo directionalLightBufferInfo =
            {
                .buffer = bufferSystem.FindVulkanBuffer(directionalLight.DirectionalLightBufferId).Buffer,
                .offset = 0,
                .range = VK_WHOLE_SIZE
            };
            directionalLightPropertiesBuffer.emplace_back(directionalLightBufferInfo);
        }
    }
    return directionalLightPropertiesBuffer;
}

const Vector<VkDescriptorBufferInfo> LightSystem::GetPointLightPropertiesBuffer()
{
    Vector<VkDescriptorBufferInfo> pointLightPropertiesBuffer;
    if (PointLightList.empty())
    {
        pointLightPropertiesBuffer.emplace_back(VkDescriptorBufferInfo
            {
                .buffer = VK_NULL_HANDLE,
                .offset = 0,
                .range = VK_WHOLE_SIZE
            });
    }
    else
    {
        for (auto& pointLight : PointLightList)
        {
            VkDescriptorBufferInfo pointLightBufferInfo =
            {
                .buffer = bufferSystem.FindVulkanBuffer(pointLight.PointLightBufferId).Buffer,
                .offset = 0,
                .range = VK_WHOLE_SIZE
            };
            pointLightPropertiesBuffer.emplace_back(pointLightBufferInfo);
        }
    }
    return pointLightPropertiesBuffer;
}
