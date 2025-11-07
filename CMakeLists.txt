cmake_minimum_required(VERSION 3.20)
project(VulkanGameEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Vulkan
find_package(Vulkan REQUIRED)

# GLFW (from External)
set(GLFW_DIR "${CMAKE_SOURCE_DIR}/External/glfw")
add_subdirectory(${GLFW_DIR} EXCLUDE_FROM_ALL)
include_directories(${GLFW_DIR}/include)

# nlohmann JSON
include_directories(External/nlohmann/include)

# SPIRV-Reflect
include_directories(External/SPIRV-Reflect/include)
add_library(spirv-reflect STATIC External/SPIRV-Reflect/spirv_reflect.cpp)

# ImGui
file(GLOB IMGUI_SOURCES
    External/imgui/imgui*.cpp
    External/imgui/backends/imgui_impl_glfw.cpp
    External/imgui/backends/imgui_impl_vulkan.cpp
)
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC External/imgui External/imgui/backends)

# ImPlot
file(GLOB IMPLOT_SOURCES External/ImPlot/implot*.cpp)
add_library(implot STATIC ${IMPLOT_SOURCES})
target_include_directories(implot PUBLIC External/ImPlot)

# VulkanEngineDLL
file(GLOB VULKAN_DLL_SRC "VulkanEngineDLL/*.cpp")
add_library(VulkanEngineDLL SHARED ${VULKAN_DLL_SRC})
target_link_libraries(VulkanEngineDLL Vulkan::Vulkan glfw spirv-reflect)

# ComponentDLL
file(GLOB COMPONENT_DLL_SRC "ComponentDLL/*.cpp")
add_library(ComponentDLL SHARED ${COMPONENT_DLL_SRC})
target_link_libraries(ComponentDLL VulkanEngineDLL)

# Physics2DDLL
file(GLOB PHYSICS_DLL_SRC "Physics2DDLL/*.cpp")
add_library(Physics2DDLL SHARED ${PHYSICS_DLL_SRC})
target_link_libraries(Physics2DDLL VulkanEngineDLL)

# Main EXE
file(GLOB MAIN_SOURCES "*.cpp")
add_executable(VulkanGameEngine ${MAIN_SOURCES})
target_link_libraries(VulkanGameEngine
    VulkanEngineDLL ComponentDLL Physics2DDLL
    imgui implot glfw Vulkan::Vulkan
)

# Copy Assets
add_custom_command(TARGET VulkanGameEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/Assets $<TARGET_FILE_DIR:VulkanGameEngine>/Assets
)