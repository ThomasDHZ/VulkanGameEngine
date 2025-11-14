#pragma once
#include "Platform.h"
#include "JsonStruct.h"
#include "from_json.h"

DLL_EXPORT RenderPassLoader JsonLoader_LoadRenderPassLoaderInfo(const char* renderPassLoaderJson, const ivec2& defaultRenderPassResoultion);
DLL_EXPORT RenderPipelineLoader JsonLoader_LoadRenderPipelineLoaderInfo(const char* renderPassLoaderJson, const ivec2& defaultRenderPassResoultion);