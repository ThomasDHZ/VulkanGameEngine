cmake_minimum_required(VERSION 3.22.1)
project(VulkanGameEngine LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Output everything to bin/ (important for both Windows and Linux debugging)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(GenerateExportHeader)

# ===================================================================
# Platform defines
# ===================================================================
if(WIN32)
    add_compile_definitions(PLATFORM_WINDOWS UNICODE _UNICODE _CRT_SECURE_NO_WARNINGS)
elseif(UNIX AND NOT APPLE)
    add_compile_definitions(PLATFORM_LINUX)
endif()

# Compiler options
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-Wno-exceptions)
    if(UNIX AND NOT APPLE)
        add_compile_options(-march=x86-64)
    endif()
endif()

# ===================================================================
# External libraries
# ===================================================================
find_package(Vulkan REQUIRED)

if(NOT ANDROID)
    add_subdirectory(External/glfw)
endif()

add_subdirectory(External/glm)
# nlohmann is header-only → no add_subdirectory needed if you include the path

# ===================================================================
# Include directories (global)
# ===================================================================
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/External
    ${CMAKE_CURRENT_SOURCE_DIR}/External/glm
    ${CMAKE_CURRENT_SOURCE_DIR}/External/nlohmann/include
    ${CMAKE_CURRENT_SOURCE_DIR}/External/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/External/imgui/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/External/ImPlot
    ${CMAKE_CURRENT_SOURCE_DIR}/External/stb
    ${CMAKE_CURRENT_SOURCE_DIR}/External/entt/src
    ${CMAKE_CURRENT_SOURCE_DIR}/External/ktx/include
    ${CMAKE_CURRENT_SOURCE_DIR}/External/xxHash
    ${CMAKE_CURRENT_SOURCE_DIR}/External/VulkanMemoryAllocator/include
    ${CMAKE_CURRENT_SOURCE_DIR}/External/VulkanMemoryAllocator/src
)

# ===================================================================
# External source files that need compilation
# ===================================================================
set(EXTERNAL_SOURCES
    External/LodePNG/lodepng.cpp
    External/SPIRV-Reflect/spirv_reflect.cpp
    External/VulkanMemoryAllocator/src/VmaUsage.cpp
    External/xxHash/xxhash.c
    External/xxHash/xxh_x86dispatch.c
)

# ===================================================================
# Libraries
# ===================================================================

# VulkanEngineDLL
file(GLOB_RECURSE VULKAN_ENGINE_SOURCES VulkanEngineDLL/*.cpp)
add_library(VulkanEngineDLL SHARED ${VULKAN_ENGINE_SOURCES} ${EXTERNAL_SOURCES})
target_link_libraries(VulkanEngineDLL PUBLIC Vulkan::Vulkan glm::glm)
if(UNIX AND NOT APPLE)
    target_link_libraries(VulkanEngineDLL PUBLIC X11 uuid pthread dl)
endif()
generate_export_header(VulkanEngineDLL)

# ComponentDLL
file(GLOB_RECURSE COMPONENT_SOURCES ComponentDLL/*.cpp)
add_library(ComponentDLL SHARED ${COMPONENT_SOURCES})
target_link_libraries(ComponentDLL PUBLIC VulkanEngineDLL glm::glm)
generate_export_header(ComponentDLL)

# Physics2DDLL
file(GLOB_RECURSE PHYSICS_SOURCES Physics2DDLL/*.cpp)
add_library(Physics2DDLL SHARED ${PHYSICS_SOURCES})
target_link_libraries(Physics2DDLL PUBLIC glm::glm)
generate_export_header(Physics2DDLL)

# Main executable
file(GLOB_RECURSE MAIN_SOURCES VulkanGameEngine/*.cpp)
add_executable(VulkanGameEngine ${MAIN_SOURCES})

target_link_libraries(VulkanGameEngine PRIVATE
    Vulkan::Vulkan
    glfw
    Physics2DDLL
    VulkanEngineDLL
    ComponentDLL
    glm::glm
)

if(UNIX AND NOT APPLE)
    target_link_libraries(VulkanGameEngine PRIVATE X11 uuid pthread dl)
endif()

# ===================================================================
# Post-build: Copy everything needed
# ===================================================================
add_custom_command(TARGET VulkanGameEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:VulkanEngineDLL>
        $<TARGET_FILE:ComponentDLL>
        $<TARGET_FILE:Physics2DDLL>
        $<TARGET_FILE_DIR:VulkanGameEngine>
    COMMENT "Copying DLLs / SOs to bin/"
)

# Force copy Assets + Config (works on both Windows and Linux)
add_custom_command(TARGET VulkanGameEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -rf "$<TARGET_FILE_DIR:VulkanGameEngine>/Assets"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/Assets" "$<TARGET_FILE_DIR:VulkanGameEngine>/Assets"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/EngineConfig.json" "$<TARGET_FILE_DIR:VulkanGameEngine>/EngineConfig.json"
    COMMENT "Copying Assets folder + EngineConfig.json"
)

# Optional: Print where things end up (helpful during debugging)
add_custom_command(TARGET VulkanGameEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Build finished. Executable in: $<TARGET_FILE_DIR:VulkanGameEngine>"
)