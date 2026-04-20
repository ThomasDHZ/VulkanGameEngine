cmake_minimum_required(VERSION 3.22.1)
project(VulkanGameEngine LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Output everything to bin/ folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(GenerateExportHeader)

# ===================================================================
# Platform defines
# ===================================================================
if(WIN32)
    add_compile_definitions(PLATFORM_WINDOWS UNICODE _UNICODE _CRT_SECURE_NO_WARNINGS)
elseif(UNIX AND NOT APPLE)
    add_compile_definitions(PLATFORM_LINUX)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-Wno-exceptions -march=x86-64)
endif()

# ===================================================================
# External libraries + Includes
# ===================================================================
find_package(Vulkan REQUIRED)

if(NOT ANDROID)
    add_subdirectory(External/glfw)
endif()
add_subdirectory(External/glm)

include_directories(
    External
    External/glm
    "${CMAKE_CURRENT_SOURCE_DIR}/External/nlohmann/include"
    External/imgui
    External/imgui/backends
    External/ImPlot
    External/stb
    External/SPIRV-Reflect
    "${CMAKE_CURRENT_SOURCE_DIR}/External/entt/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/External/ktx/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/External/xxHash"
    "${CMAKE_CURRENT_SOURCE_DIR}/External/LodePNG"
    "${CMAKE_CURRENT_SOURCE_DIR}/External/VulkanMemoryAllocator/include"
    ${Vulkan_INCLUDE_DIRS}
)

if(NOT ANDROID)
    include_directories(External/glfw/include)
endif()

# ===================================================================
# VulkanEngineDLL
# ===================================================================
add_library(VulkanEngineDLL SHARED
    VulkanEngineDLL/Platform.cpp
    VulkanEngineDLL/VulkanSystem.cpp
    VulkanEngineDLL/RenderSystem.cpp
    VulkanEngineDLL/TextureSystem.cpp
    VulkanEngineDLL/MeshSystem.cpp
    VulkanEngineDLL/MaterialSystem.cpp
    VulkanEngineDLL/BufferSystem.cpp
    VulkanEngineDLL/FileSystem.cpp
    VulkanEngineDLL/JsonLoader.cpp
    VulkanEngineDLL/from_json.cpp
    VulkanEngineDLL/VkGuid.cpp
    VulkanEngineDLL/DebugSystem.cpp
    VulkanEngineDLL/EngineConfigSystem.cpp
    VulkanEngineDLL/CHelper.cpp
    VulkanEngineDLL/MemorySystem.cpp
    VulkanEngineDLL/MemoryPoolSystem.cpp
    VulkanEngineDLL/ShaderSystem.cpp
    VulkanEngineDLL/Pixel.cpp

    VulkanEngineDLL/CameraSystem.cpp
    VulkanEngineDLL/LightSystem.cpp

    # External implementations
    External/LodePNG/lodepng.cpp
    External/SPIRV-Reflect/spirv_reflect.cpp
    External/VulkanMemoryAllocator/src/VmaUsage.cpp
    External/xxHash/xxhash.c
    External/xxHash/xxh_x86dispatch.c
)

generate_export_header(VulkanEngineDLL EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/vulkanenginedll_export.h)

target_include_directories(VulkanEngineDLL PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/VulkanEngineDLL
    External/SPIRV-Reflect
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(VulkanEngineDLL PRIVATE
    Vulkan::Vulkan
    glfw
    X11 uuid pthread dl
    ktx
)

# ===================================================================
# Physics2DDLL
# ===================================================================
add_library(Physics2DDLL SHARED
    Physics2DDLL/AABB.cpp
    Physics2DDLL/BoxCollider2D.cpp
    Physics2DDLL/CircleCollider.cpp
    Physics2DDLL/Collider2D.cpp
    Physics2DDLL/IntersectionDetector2D.cpp
    Physics2DDLL/RigidBody.cpp
    Physics2DDLL/pch.cpp
)

generate_export_header(Physics2DDLL EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/physics2ddll_export.h)
target_precompile_headers(Physics2DDLL PRIVATE Physics2DDLL/pch.h)

target_include_directories(Physics2DDLL PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Physics2DDLL
    ${CMAKE_CURRENT_SOURCE_DIR}/VulkanEngineDLL
    ${CMAKE_CURRENT_BINARY_DIR}
)

# ===================================================================
# ComponentDLL
# ===================================================================
add_library(ComponentDLL SHARED
    ComponentDLL/Component.cpp
    ComponentDLL/GameObjectSystem.cpp
    ComponentDLL/LevelSystem.cpp
    ComponentDLL/MegaManObject.cpp
    ComponentDLL/MegaManShot.cpp
    ComponentDLL/Camera.cpp
    ComponentDLL/SpriteSystem.cpp
    ComponentDLL/pch.cpp
)

generate_export_header(ComponentDLL EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/componentdll_export.h)
target_precompile_headers(ComponentDLL PRIVATE ComponentDLL/pch.h)

target_include_directories(ComponentDLL PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/ComponentDLL
    ${CMAKE_CURRENT_SOURCE_DIR}/VulkanEngineDLL
    ${CMAKE_CURRENT_SOURCE_DIR}/Physics2DDLL
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(ComponentDLL PRIVATE VulkanEngineDLL Physics2DDLL)

# ===================================================================
# Main Executable
# ===================================================================
set(IMGUI_SOURCES
    External/imgui/imgui.cpp
    External/imgui/imgui_draw.cpp
    External/imgui/imgui_tables.cpp
    External/imgui/imgui_widgets.cpp
    External/imgui/imgui_demo.cpp
    External/imgui/backends/imgui_impl_vulkan.cpp
    External/imgui/backends/imgui_impl_glfw.cpp
)

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
    ${IMGUI_SOURCES}
)

target_include_directories(VulkanGameEngine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/VulkanGameEngine
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(VulkanGameEngine PRIVATE
    Vulkan::Vulkan
    glfw
    Physics2DDLL
    VulkanEngineDLL
    ComponentDLL
    glm::glm
    X11 uuid pthread dl
)

# ===================================================================
# Post-build: Copy DLLs + Assets + EngineConfig.json
# ===================================================================
add_custom_command(TARGET VulkanGameEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:VulkanEngineDLL>
        $<TARGET_FILE:ComponentDLL>
        $<TARGET_FILE:Physics2DDLL>
        $<TARGET_FILE_DIR:VulkanGameEngine>
    COMMENT "Copying shared libraries"
)

# Copy Assets folder (forced every build)
add_custom_command(TARGET VulkanGameEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -rf "$<TARGET_FILE_DIR:VulkanGameEngine>/Assets"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/Assets" "$<TARGET_FILE_DIR:VulkanGameEngine>/Assets"
    COMMENT "Copying Assets folder"
)

# Copy EngineConfig.json
add_custom_command(TARGET VulkanGameEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/EngineConfig.json"
        "$<TARGET_FILE_DIR:VulkanGameEngine>/EngineConfig.json"
    COMMENT "Copying EngineConfig.json"
)