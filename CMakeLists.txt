cmake_minimum_required(VERSION 3.22)
project(VulkanGameEngine LANGUAGES CXX C)

if(UNIX AND NOT APPLE)
    add_compile_options(-Wno-exceptions)
    add_compile_options(-Wno-error=everything)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wno-error=float-conversion -Wno-error=everything)
endif()

include(GenerateExportHeader)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ------------------------------------------------------------------
# Global compile definitions
# ------------------------------------------------------------------
add_compile_definitions(
    _CRT_SECURE_NO_WARNINGS
    _CONSOLE
)

if(WIN32)
    add_compile_definitions(PLATFORM_WINDOWS UNICODE _UNICODE)
endif()

# ------------------------------------------------------------------
# External libraries
# ------------------------------------------------------------------
add_subdirectory(External/glfw)
add_subdirectory(External/glm)
add_subdirectory(External/nlohmann)
 
include_directories(
    External
    External/glfw/include
    External/glm
    External/nlohmann/include
    External/imgui
    External/imgui/backends
    External/ImPlot
    External/stb
    External/glfw/include/GLFW
    C:/VulkanSDK/1.4.328.1/Include
)
 
find_package(Vulkan REQUIRED)

# ------------------------------------------------------------------
# ComponentDLL
# ------------------------------------------------------------------
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

generate_export_header(ComponentDLL
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/componentdll_export.h
)

target_precompile_headers(ComponentDLL PRIVATE ComponentDLL/pch.h)

target_include_directories(ComponentDLL PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/ComponentDLL
    ${CMAKE_CURRENT_SOURCE_DIR}/VulkanEngineDLL
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(ComponentDLL PRIVATE
    Vulkan::Vulkan
    glfw
    VulkanEngineDLL
)

# ------------------------------------------------------------------
# Physics2DDLL
# ------------------------------------------------------------------
add_library(Physics2DDLL SHARED
    Physics2DDLL/AABB.cpp
    Physics2DDLL/BoxCollider2D.cpp
    Physics2DDLL/CircleCollider.cpp
    Physics2DDLL/Collider2D.cpp
    Physics2DDLL/IntersectionDetector2D.cpp
    Physics2DDLL/RigidBody.cpp
    Physics2DDLL/pch.cpp
)

generate_export_header(Physics2DDLL
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/physics2ddll_export.h
)

target_precompile_headers(Physics2DDLL PRIVATE Physics2DDLL/pch.h)

target_include_directories(Physics2DDLL PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Physics2DDLL
    ${CMAKE_CURRENT_SOURCE_DIR}/VulkanEngineDLL
    ${CMAKE_CURRENT_BINARY_DIR} 
)

# ------------------------------------------------------------------
# VulkanEngineDLL
# ------------------------------------------------------------------
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
    VulkanEngineDLL/Pixel.cpp
    External/SPIRV-Reflect/spirv_reflect.cpp
)

generate_export_header(VulkanEngineDLL
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/vulkanenginedll_export.h
)

target_precompile_headers(VulkanEngineDLL PRIVATE VulkanEngineDLL/Platform.h)

target_include_directories(VulkanEngineDLL PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/VulkanEngineDLL
    External/SPIRV-Reflect
    ${CMAKE_CURRENT_BINARY_DIR} 
)

target_link_libraries(VulkanEngineDLL PRIVATE
    Vulkan::Vulkan
    glfw
)

if(UNIX AND NOT APPLE)
    target_link_libraries(VulkanEngineDLL PRIVATE X11 uuid pthread dl)
endif()

set_target_properties(VulkanEngineDLL PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)

# ------------------------------------------------------------------
# ImGui + ImPlot
# ------------------------------------------------------------------
set(IMGUI_SOURCES
    External/imgui/imgui.cpp
    External/imgui/imgui_draw.cpp
    External/imgui/imgui_tables.cpp
    External/imgui/imgui_widgets.cpp
    External/imgui/imgui_demo.cpp
    External/imgui/backends/imgui_impl_glfw.cpp
    External/imgui/backends/imgui_impl_vulkan.cpp
)

set(IMPLOT_SOURCES
    External/ImPlot/implot.cpp
    External/ImPlot/implot_items.cpp
    External/ImPlot/implot_demo.cpp
)

# ------------------------------------------------------------------
# Main Executable
# ------------------------------------------------------------------
add_executable(VulkanGameEngine
    VulkanGameEngine/Main.cpp
    VulkanGameEngine/GameController.cpp
    VulkanGameEngine/ImGuiRenderer.cpp
    VulkanGameEngine/Keyboard.cpp
    VulkanGameEngine/Mouse.cpp
    VulkanGameEngine/FrameTimer.cpp
    VulkanGameEngine/GameSystem.cpp
    VulkanGameEngine/InputSystem.cpp
    VulkanGameEngine/SystemClock.cpp
    VulkanGameEngine/VulkanWindow.cpp
    VulkanGameEngine/Tile.cpp
    VulkanGameEngine/VulkanFileSystem.cpp
    ${IMGUI_SOURCES}
    ${IMPLOT_SOURCES}
)
 
target_include_directories(VulkanGameEngine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/VulkanGameEngine
    ${CMAKE_CURRENT_SOURCE_DIR}/VulkanEngineDLL
    ${CMAKE_CURRENT_SOURCE_DIR}/ComponentDLL
    ${CMAKE_CURRENT_SOURCE_DIR}/Physics2DDLL
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(VulkanGameEngine PRIVATE
    Vulkan::Vulkan
    glfw 
    Physics2DDLL
    VulkanEngineDLL
    ComponentDLL
)

add_custom_command(TARGET VulkanGameEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:VulkanEngineDLL>
        $<TARGET_FILE:ComponentDLL>
        $<TARGET_FILE:Physics2DDLL>
        $<TARGET_FILE_DIR:VulkanGameEngine>
)

if(UNIX AND NOT APPLE)
    add_custom_command(TARGET VulkanGameEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Copying Assets/Config from shared folder..."
        COMMAND ${CMAKE_COMMAND} -E rm -rf "${CMAKE_CURRENT_BINARY_DIR}/Assets"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "/media/sf_VulkanGameEngine/Assets" "${CMAKE_CURRENT_BINARY_DIR}/Assets"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "/media/sf_VulkanGameEngine/EngineConfig.json" "${CMAKE_CURRENT_BINARY_DIR}/Config/EngineConfig.json"
        COMMENT "Copying Assets/Config from VirtualBox shared folder"
    )
endif()
 