#pragma once
#include <Typedef.h>
#include <Material.h>
#include <VulkanShader.h>

class MaterialSystem
{
private:
    UnorderedMap<RenderPassGuid, Material> MaterialMap;

public:
    MaterialSystem();
    ~MaterialSystem();

    void Update(const float& deltaTime);
    VkGuid LoadMaterial(const String& materialPath);

    const bool MaterialMapExists(const VkGuid& renderPassId)  const;

    const Material& FindMaterial(const RenderPassGuid& guid);
    const Vector<Material>& MaterialList();

    const Vector<VkDescriptorBufferInfo> GetMaterialPropertiesBuffer();

    void Destroy(const VkGuid& guid);
    void DestroyAllMaterials();
};
extern MaterialSystem materialSystem;