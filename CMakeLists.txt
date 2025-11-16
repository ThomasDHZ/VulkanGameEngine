cmake_minimum_required(VERSION 3.20)
project(VulkanGameEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Vulkan SDK
set(VULKAN_SDK "C:/VulkanSDK/1.4.328.1")
set(ENV{VULKAN_SDK} ${VULKAN_SDK})
find_package(Vulkan REQUIRED)

# External
set(EXTERNAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/External")
set(NLOHMANN_INCLUDE_DIR "${EXTERNAL_DIR}/nlohmann/include")
set(STB_INCLUDE_DIR "${EXTERNAL_DIR}/stb")
set(SPIRV_REFLECT_INCLUDE_DIR "${EXTERNAL_DIR}/SPIRV-Reflect/include")
set(SPIRV_REFLECT_SOURCE "${EXTERNAL_DIR}/SPIRV-Reflect/spirv_reflect.cpp")
set(GLFW_INCLUDE_DIR "${EXTERNAL_DIR}/glfw/include")
set(SDL2_INCLUDE_DIR "${EXTERNAL_DIR}/sdl/include")
set(GLFW_LIB_DIR "${EXTERNAL_DIR}/glfw/lib-vc2022")
set(SDL2_LIB_DIR "${EXTERNAL_DIR}/sdl/lib/x64")
set(VulkanEngineDLL_DIR "VulkanEngineDLL")
set(ComponentDLL_DIR "ComponentDLL")
set(Physics2DDLL_DIR "Physics2DDLL")

# ComponentDLL
add_library(ComponentDLL SHARED
    ComponentDLL/Component.cpp
    ComponentDLL/GameObjectSystem.cpp
    ComponentDLL/LevelSystem.cpp
    ComponentDLL/MegaManObject.cpp
    ComponentDLL/MegaManShot.cpp
    ComponentDLL/Camera.cpp
    ComponentDLL/SpriteSystem.cpp
    ComponentDLL/VRAM.cpp
    ComponentDLL/pch.cpp
)
target_precompile_headers(ComponentDLL PRIVATE ComponentDLL/pch.h)
target_include_directories(ComponentDLL PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/ComponentDLL
    ${Vulkan_INCLUDE_DIRS}
    ${STB_INCLUDE_DIR}
    ${SPIRV_REFLECT_INCLUDE_DIR}
    ${NLOHMANN_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIR}
    ${SDL2_INCLUDE_DIR}
    ${VulkanEngineDLL_DIR}
)
target_link_libraries(ComponentDLL Vulkan::Vulkan ${GLFW_LIB_DIR}/glfw3.lib ${SDL2_LIB_DIR}/SDL2.lib ${SDL2_LIB_DIR}/SDL2main.lib stdc++fs)

# Physics2DDLL
add_library(Physics2DDLL SHARED
    Physics2DDLL/AABB.cpp
    Physics2DDLL/BoxCollider2D.cpp
    Physics2DDLL/CircleCollider.cpp
    Physics2DDLL/Collider2D.cpp
    Physics2DDLL/IntersectionDetector2D.cpp
    Physics2DDLL/RigidBody.cpp
    Physics2DDLL/pch.cpp
)
target_precompile_headers(Physics2DDLL PRIVATE Physics2DDLL/pch.h)
target_include_directories(Physics2DDLL PRIVATE 
${CMAKE_CURRENT_SOURCE_DIR}/Physics2DDLL 
${Vulkan_INCLUDE_DIRS}
${VulkanEngineDLL_DIR}
)

# VulkanEngineDLL
add_library(VulkanEngineDLL SHARED
    VulkanEngineDLL/Platform.cpp
    VulkanEngineDLL/VulkanRenderer.cpp
    VulkanEngineDLL/RenderSystem.cpp
    VulkanEngineDLL/TextureSystem.cpp
    VulkanEngineDLL/MeshSystem.cpp
    VulkanEngineDLL/MaterialSystem.cpp
    VulkanEngineDLL/BufferSystem.cpp
    VulkanEngineDLL/GPUSystem.cpp
    VulkanEngineDLL/FileSystem.cpp
    VulkanEngineDLL/JsonLoader.cpp
    VulkanEngineDLL/from_json.cpp
    VulkanEngineDLL/VkGuid.cpp
    VulkanEngineDLL/DebugSystem.cpp
    VulkanEngineDLL/EngineConfigSystem.cpp
    VulkanEngineDLL/CHelper.cpp
    VulkanEngineDLL/MemorySystem.cpp
    VulkanEngineDLL/ShaderSystem.cpp
    VulkanEngineDLL/VulkanPipeline.cpp
    VulkanEngineDLL/VulkanRenderPass.cpp
    ${SPIRV_REFLECT_SOURCE}
)
target_precompile_headers(VulkanEngineDLL PRIVATE VulkanEngineDLL/Platform.h)
target_include_directories(VulkanEngineDLL PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/VulkanEngineDLL
    ${Vulkan_INCLUDE_DIRS}
    ${SPIRV_REFLECT_INCLUDE_DIR}
    ${NLOHMANN_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIR}
    ${SDL2_INCLUDE_DIR}
    ${VulkanEngineDLL_DIR}
    ${ComponentDLL_DIR}
    ${Physics2DDLL_DIR}

)
target_link_libraries(VulkanEngineDLL Vulkan::Vulkan ${GLFW_LIB_DIR}/glfw3.lib ${SDL2_LIB_DIR}/SDL2.lib ${SDL2_LIB_DIR}/SDL2main.lib stdc++fs)

# Main
add_executable(VulkanGameEngine main.cpp pch.cpp)
target_precompile_headers(VulkanGameEngine PRIVATE pch.h)
target_include_directories(VulkanGameEngine PRIVATE 
${Vulkan_INCLUDE_DIRS} 
${NLOHMANN_INCLUDE_DIR}
)
target_link_libraries(VulkanGameEngine VulkanEngineDLL ComponentDLL Physics2DDLL Vulkan::Vulkan ${GLFW_LIB_DIR}/glfw3.lib ${SDL2_LIB_DIR}/SDL2.lib ${SDL2_LIB_DIR}/SDL2main.lib)

# Platform
if(WIN32)
    add_definitions(-DPLATFORM_WINDOWS)
endif()