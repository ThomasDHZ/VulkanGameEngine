// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include <Platform.h>
#include <BufferSystem.h>
#include <FileSystem.h>
#include <MaterialSystem.h>
#include <MeshSystem.h>
#include <RenderSystem.h>
#include <ShaderSystem.h>
#include <TextureSystem.h>
#include <InputEnum.h>
#include <GLFW/glfw3.h>
#include <memory.h>
#include <DLL.h>
#include <Typedef.h>
#include <VulkanSystem.h>
#include <LightSystem.h>
#include <EngineConfigSystem.h>
#include "GameObjectBehavior.h"
#endif //PCH_H
