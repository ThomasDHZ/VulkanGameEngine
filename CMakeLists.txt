cmake_minimum_required(VERSION 3.22.1)
project(VulkanGameEngine LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Output to bin/ for easy debugging
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(GenerateExportHeader)

# Platform
if(WIN32)
    add_compile_definitions(PLATFORM_WINDOWS UNICODE _UNICODE _CRT_SECURE_NO_WARNINGS)
elseif(UNIX AND NOT APPLE)
    add_compile_definitions(PLATFORM_LINUX)
endif()

find_package(Vulkan REQUIRED)

# Safe subdirectories
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/External/glfw/CMakeLists.txt")
    add_subdirectory(External/glfw)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/External/glm/CMakeLists.txt")
    add_subdirectory(External/glm)
endif()

# ===================================================================
# INCLUDE DIRECTORIES - FIXED FOR CLEAN CLONE
# ===================================================================
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}

    # Your source folders
    ${CMAKE_CURRENT_SOURCE_DIR}/VulkanEngineDLL
    ${CMAKE_CURRENT_SOURCE_DIR}/ComponentDLL
    ${CMAKE_CURRENT_SOURCE_DIR}/Physics2DDLL

    # Externals - try both common nlohmann structures
    ${CMAKE_CURRENT_SOURCE_DIR}/External
    ${CMAKE_CURRENT_SOURCE_DIR}/External/glm
    ${CMAKE_CURRENT_SOURCE_DIR}/External/nlohmann/include          # Common full structure
    ${CMAKE_CURRENT_SOURCE_DIR}/External/nlohmann                   # Single header structure
    ${CMAKE_CURRENT_SOURCE_DIR}/External/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/External/imgui/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/External/entt/src
    ${CMAKE_CURRENT_SOURCE_DIR}/External/ktx/include
    ${CMAKE_CURRENT_SOURCE_DIR}/External/xxHash
    ${CMAKE_CURRENT_SOURCE_DIR}/External/VulkanMemoryAllocator/include

    ${Vulkan_INCLUDE_DIRS}   # vulkan/vulkan_core.h + GLFW if needed
)

# ===================================================================
# Libraries (minimal for now - add external sources later if needed)
# ===================================================================
file(GLOB_RECURSE VULKAN_ENGINE_SOURCES "VulkanEngineDLL/*.cpp")
add_library(VulkanEngineDLL SHARED ${VULKAN_ENGINE_SOURCES})
target_link_libraries(VulkanEngineDLL PUBLIC Vulkan::Vulkan glm::glm)
generate_export_header(VulkanEngineDLL)

file(GLOB_RECURSE COMPONENT_SOURCES "ComponentDLL/*.cpp")
add_library(ComponentDLL SHARED ${COMPONENT_SOURCES})
target_link_libraries(ComponentDLL PUBLIC VulkanEngineDLL glm::glm)
generate_export_header(ComponentDLL)

file(GLOB_RECURSE PHYSICS_SOURCES "Physics2DDLL/*.cpp")
add_library(Physics2DDLL SHARED ${PHYSICS_SOURCES})
target_link_libraries(Physics2DDLL PUBLIC glm::glm)
generate_export_header(Physics2DDLL)

# Main executable
file(GLOB_RECURSE MAIN_SOURCES "VulkanGameEngine/*.cpp")
add_executable(VulkanGameEngine ${MAIN_SOURCES})

target_link_libraries(VulkanGameEngine PRIVATE
    Vulkan::Vulkan
    glfw
    Physics2DDLL
    VulkanEngineDLL
    ComponentDLL
    glm::glm
)

# Post-build
add_custom_command(TARGET VulkanGameEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:VulkanEngineDLL>
        $<TARGET_FILE:ComponentDLL>
        $<TARGET_FILE:Physics2DDLL>
        $<TARGET_FILE_DIR:VulkanGameEngine>
    COMMENT "Copying DLLs"
)

add_custom_command(TARGET VulkanGameEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -rf "$<TARGET_FILE_DIR:VulkanGameEngine>/Assets"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/Assets" "$<TARGET_FILE_DIR:VulkanGameEngine>/Assets"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/EngineConfig.json" "$<TARGET_FILE_DIR:VulkanGameEngine>/EngineConfig.json"
    COMMENT "Copying Assets + Config"
)