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
# External Paths — FIXED TO MATCH YOUR CURRENT SOURCE INCLUDES
# ========================================
set(EXTERNAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/External")

set(GLFW_INCLUDE_DIR            "${EXTERNAL_DIR}/glfw/include")                    # for #include <GLFW/glfw3.h> and "../GLFW/glfw3.h"
set(GLFW_LIB                    "${EXTERNAL_DIR}/glfw/lib/glfw3.lib")

set(NLOHMANN_INCLUDE_DIR        "${EXTERNAL_DIR}/nlohmann/include")

set(SPIRV_REFLECT_INCLUDE_DIR   "${EXTERNAL_DIR}/SPIRV-Reflect/include")
set(SPIRV_REFLECT_SOURCE        "${EXTERNAL_DIR}/SPIRV-Reflect/spirv_reflect.cpp")

set(IMGUI_ROOT_DIR              "${EXTERNAL_DIR}/imgui")                           # for #include "imgui.h"
set(IMGUI_BACKENDS_DIR          "${EXTERNAL_DIR}/imgui/backends")                 # for #include "imgui/backends/imgui_impl_*.h"

set(IMPLOT_INCLUDE_DIR          "${EXTERNAL_DIR}/ImPlot")

# ========================================
# Output directories
# ========================================
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/x64/Debug)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/x64/Debug)

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
    ${IMGUI_ROOT_DIR}
    ${IMGUI_BACKENDS_DIR}
)

target_compile_definitions(VulkanEngineDLL PRIVATE VULKANENGINEDLL_EXPORTS)
target_link_libraries(VulkanEngineDLL PRIVATE Vulkan::Vulkan ${GLFW_LIB})

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
    ${CMAKE_CURRENT_SOURCE_DIR}/VulkanEngineDLL
    ${Vulkan_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIR}
    ${NLOHMANN_INCLUDE_DIR}
    ${SPIRV_REFLECT_INCLUDE_DIR}
    ${IMGUI_ROOT_DIR}
    ${IMGUI_BACKENDS_DIR}
)

target_compile_definitions(Physics2DDLL PRIVATE Physics2DDLL_EXPORTS)
target_link_libraries(Physics2DDLL PRIVATE Vulkan::Vulkan VulkanEngineDLL ${GLFW_LIB})

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
    ${CMAKE_CURRENT_SOURCE_DIR}/Physics2DDLL
    ${CMAKE_CURRENT_SOURCE_DIR}/VulkanEngineDLL
    ${Vulkan_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIR}
    ${NLOHMANN_INCLUDE_DIR}
    ${SPIRV_REFLECT_INCLUDE_DIR}
    ${IMGUI_ROOT_DIR}
    ${IMGUI_BACKENDS_DIR}
)

target_compile_definitions(ComponentDLL PRIVATE COMPONENTDLL_EXPORTS)
target_link_libraries(ComponentDLL PRIVATE
    Vulkan::Vulkan
    Physics2DDLL
    VulkanEngineDLL
    ${GLFW_LIB}
)

# ========================================
# MAIN EXECUTABLE — VulkanGameEngine
# ========================================
add_executable(VulkanGameEngine
    VulkanGameEngine/main.cpp
    VulkanGameEngine/GameController.cpp
    VulkanGameEngine/ImGuiRenderer.cpp
    VulkanGameEngine/Keyboard.cpp
    VulkanGameEngine/Mouse.cpp
    VulkanGameEngine/FrameTimer.cpp
    VulkanGameEngine/GameSystem.cpp
    VulkanGameEngine/InputSystem.cpp
    VulkanGameEngine/SystemClock.cpp
    VulkanGameEngine/VulkanWindow.cpp

    # ImGui source files
    ${EXTERNAL_DIR}/imgui/imgui.cpp
    ${EXTERNAL_DIR}/imgui/imgui_draw.cpp
    ${EXTERNAL_DIR}/imgui/imgui_widgets.cpp
    ${EXTERNAL_DIR}/imgui/imgui_tables.cpp
    ${EXTERNAL_DIR}/imgui/backends/imgui_impl_glfw.cpp
    ${EXTERNAL_DIR}/imgui/backends/imgui_impl_vulkan.cpp
)

target_include_directories(VulkanGameEngine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/VulkanGameEngine
    ${CMAKE_CURRENT_SOURCE_DIR}/VulkanEngineDLL
    ${CMAKE_CURRENT_SOURCE_DIR}/ComponentDLL
    ${CMAKE_CURRENT_SOURCE_DIR}/Physics2DDLL
    ${Vulkan_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIR}
    ${NLOHMANN_INCLUDE_DIR}
    ${SPIRV_REFLECT_INCLUDE_DIR}
    ${IMPLOT_INCLUDE_DIR}
    ${IMGUI_ROOT_DIR}
    ${IMGUI_BACKENDS_DIR}           # This line fixes all imgui/backends includes
)

target_link_libraries(VulkanGameEngine PRIVATE
    VulkanEngineDLL
    ComponentDLL
    Physics2DDLL
    Vulkan::Vulkan
    ${GLFW_LIB}
)

# ========================================
# Platform defines
# ========================================
if(WIN32)
    add_definitions(-DPLATFORM_WINDOWS)
endif()