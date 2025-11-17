cmake_minimum_required(VERSION 3.20)
project(VulkanGameEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ========================================
# Vulkan SDK
# ========================================
set(VULKAN_SDK "C:/VulkanSDK/1.4.328.1")
set(ENV{VULKAN_SDK} ${VULKAN_SDK})
find_package(Vulkan REQUIRED)

# ========================================
# Paths
# ========================================
set(EXTERNAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/External")
set(GAME_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/VulkanGameEngine")

# ========================================
# External Libraries — NO SDL2
# ========================================
# GLFW
set(GLFW_INCLUDE_DIR "${EXTERNAL_DIR}/glfw/include")
set(GLFW_LIB_DIR     "${EXTERNAL_DIR}/glfw/lib")
set(GLFW3_LIB        "${GLFW_LIB_DIR}/glfw3.lib")

# nlohmann JSON
set(NLOHMANN_INCLUDE_DIR "${EXTERNAL_DIR}/nlohmann/include")

# SPIRV-Reflect
set(SPIRV_REFLECT_INCLUDE_DIR "${EXTERNAL_DIR}/SPIRV-Reflect/include")
set(SPIRV_REFLECT_SOURCE      "${EXTERNAL_DIR}/SPIRV-Reflect/spirv_reflect.cpp")

# ImGui
set(IMGUI_DIR           "${EXTERNAL_DIR}/imgui")
set(IMGUI_BACKENDS_DIR  "${IMGUI_DIR}/backends")

# ========================================
# Output
# ========================================
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/x64/Debug)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/x64/Debug)

# ========================================
# ComponentDLL
# ========================================
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
    ${GLFW_INCLUDE_DIR}
    ${NLOHMANN_INCLUDE_DIR}
    ${SPIRV_REFLECT_INCLUDE_DIR}
)
target_compile_definitions(ComponentDLL PRIVATE COMPONENTDLL_EXPORTS)
target_link_libraries(ComponentDLL
    Vulkan::Vulkan
    "${GLFW3_LIB}"
)

# ========================================
# Physics2DDLL
# ========================================
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
)

# ========================================
# VulkanEngineDLL
# ========================================
add_library(VulkanEngineDLL SHARED
    VulkanEngineDLL/Platform.cpp
    VulkanEngineDLL/InputEnum.cpp
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
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/VulkanEngineDLL
    ${Vulkan_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIR}
    ${NLOHMANN_INCLUDE_DIR}
    ${SPIRV_REFLECT_INCLUDE_DIR}
    ${IMGUI_DIR}
    ${IMGUI_BACKENDS_DIR}
)
target_compile_definitions(VulkanEngineDLL PRIVATE VULKANENGINEDLL_EXPORTS)
target_link_libraries(VulkanEngineDLL
    Vulkan::Vulkan
    "${GLFW3_LIB}"
)

# ========================================
# MAIN EXECUTABLE — IN VulkanGameEngine/
# ========================================
add_executable(VulkanGameEngine
    ${GAME_SRC_DIR}/main.cpp
    ${GAME_SRC_DIR}/GameController.cpp
    ${GAME_SRC_DIR}/ImGuiRenderer.cpp
    ${GAME_SRC_DIR}/Keyboard.cpp
    ${GAME_SRC_DIR}/Mouse.cpp
    ${GAME_SRC_DIR}/FrameTimer.cpp
    ${GAME_SRC_DIR}/GameSystem.cpp
    ${GAME_SRC_DIR}/InputSystem.cpp
    ${GAME_SRC_DIR}/SystemClock.cpp
    ${GAME_SRC_DIR}/VulkanWindow.cpp
    # ImGui
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_BACKENDS_DIR}/imgui_impl_glfw.cpp
    ${IMGUI_BACKENDS_DIR}/imgui_impl_vulkan.cpp
)

target_include_directories(VulkanGameEngine PRIVATE
    ${GAME_SRC_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/VulkanEngineDLL
    ${CMAKE_CURRENT_SOURCE_DIR}/ComponentDLL
    ${Vulkan_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIR}
    ${NLOHMANN_INCLUDE_DIR}
    ${SPIRV_REFLECT_INCLUDE_DIR}
    ${IMGUI_DIR}
    ${IMGUI_BACKENDS_DIR}
)

target_link_libraries(VulkanGameEngine
    VulkanEngineDLL
    ComponentDLL
    Physics2DDLL
    Vulkan::Vulkan
    "${GLFW3_LIB}"
)

# ========================================
# Platform
# ========================================
if(WIN32)
    add_definitions(-DPLATFORM_WINDOWS)
endif()