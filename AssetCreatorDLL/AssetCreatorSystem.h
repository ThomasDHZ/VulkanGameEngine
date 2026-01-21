#pragma once
#include "pch.h"
#include <DLL.h>
#include <Platform.h>
#include <JsonStruct.h>
#include <VulkanSystem.h>
#include <TextureSystem.h>
#include <MaterialSystem.h>

class AssetCreatorSystem
{
public:
    static AssetCreatorSystem& Get();

private:
    AssetCreatorSystem() = default;
    ~AssetCreatorSystem() = default;
    AssetCreatorSystem(const AssetCreatorSystem&) = delete;
    AssetCreatorSystem& operator=(const AssetCreatorSystem&) = delete;
    AssetCreatorSystem(AssetCreatorSystem&&) = delete;
    AssetCreatorSystem& operator=(AssetCreatorSystem&&) = delete;

public:
    DLL_EXPORT void Run();
};
extern DLL_EXPORT AssetCreatorSystem& assetCreatorSystem;
inline AssetCreatorSystem& AssetCreatorSystem::Get()
{
    static AssetCreatorSystem instance;
    return instance;
}

