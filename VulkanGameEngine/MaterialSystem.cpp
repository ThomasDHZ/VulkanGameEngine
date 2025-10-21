#include "MaterialSystem.h"
#include "TextureSystem.h"
#include <Material.h>
#include "BufferSystem.h"
#include <Filesystem.h>

MaterialSystem materialSystem = MaterialSystem();

MaterialSystem::MaterialSystem()
{
}

MaterialSystem::~MaterialSystem()
{
}

void MaterialSystem::Update(const float& deltaTime)
{
    Material_Update(renderer, deltaTime);
}

VkGuid MaterialSystem::LoadMaterial(const String& materialPath)
{
    return Material_LoadMaterial(renderer, materialPath);
}

const bool MaterialSystem::MaterialMapExists(const VkGuid& renderPassId)  const
{
    return Material_MaterialMapExists(renderPassId);
}

const Material& MaterialSystem::FindMaterial(const RenderPassGuid& guid)
{
    return Material_FindMaterial(guid);
}

const Vector<Material>& MaterialSystem::MaterialList()
{
    return Material_MaterialList();
}

const Vector<VkDescriptorBufferInfo> MaterialSystem::GetMaterialPropertiesBuffer()
{
    return Material_GetMaterialPropertiesBuffer();
}


void MaterialSystem::Destroy(const VkGuid& guid)
{
    Material_Destroy(guid);
}

void MaterialSystem::DestroyAllMaterials()
{
    Material_DestroyAllMaterials();
}