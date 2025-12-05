cmake_minimum_required(VERSION 3.22.1)
project(VulkanGameEngine LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
include(GenerateExportHeader)

# ===================================================================
# Platform defines
# ===================================================================
if(WIN32)
    add_compile_definitions(PLATFORM_WINDOWS UNICODE _UNICODE)
elseif(ANDROID)
    add_compile_definitions(PLATFORM_ANDROID)
elseif(UNIX AND NOT APPLE)
    add_compile_definitions(PLATFORM_LINUX)
endif()

# ===================================================================
# Compiler options
# ===================================================================
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-Wno-exceptions)
endif()
if(UNIX AND NOT APPLE AND NOT ANDROID)
    add_compile_options(-march=x86-64)
endif()

# ===================================================================
# External libraries
# ===================================================================
find_package(Vulkan REQUIRED)
if(NOT ANDROID)
    add_subdirectory(External/glfw)
endif()
add_subdirectory(External/glm)
add_subdirectory(External/nlohmann)

# ===================================================================
# Include directories (global)
# ===================================================================
include_directories(
    External
    External/glm
    External/nlohmann/include
    External/imgui
    External/imgui/backends
    External/ImPlot
    External/stb
    External/SPIRV-Reflect
    ${Vulkan_INCLUDE_DIRS}
)
if(NOT ANDROID)
    include_directories(External/glfw/include)
endif()

# ===================================================================
# Windows: copy GLFW headers trick
# ===================================================================
if(WIN32)
    add_custom_target(CopyGLFWHeaders ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/External/imgui/GLFW"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/External/glfw/include/GLFW/glfw3.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/External/glfw/include/GLFW/glfw3native.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/External/imgui/GLFW/"
        COMMENT "Windows: Enabling #include \"../GLFW/glfw3.h\""
        VERBATIM
    )
    set(GLFW_HEADER_DEPS CopyGLFWHeaders)
else()
    set(GLFW_HEADER_DEPS)
endif()

# ===================================================================
# DLLs
# ===================================================================
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

generate_export_header(ComponentDLL EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/componentdll_export.h)
target_precompile_headers(ComponentDLL PRIVATE ComponentDLL/pch.h)

target_include_directories(ComponentDLL PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/ComponentDLL
    ${CMAKE_CURRENT_SOURCE_DIR}/VulkanEngineDLL
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(ComponentDLL PRIVATE VulkanEngineDLL)

if(ANDROID)
    target_link_libraries(ComponentDLL PRIVATE android log vulkan) 
endif()

if(GLFW_HEADER_DEPS AND NOT ANDROID)
    add_dependencies(ComponentDLL ${GLFW_HEADER_DEPS})
endif()

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

if(ANDROID)
    target_link_libraries(Physics2DDLL PRIVATE android log vulkan)
endif()

if(GLFW_HEADER_DEPS AND NOT ANDROID)
    add_dependencies(Physics2DDLL ${GLFW_HEADER_DEPS})
endif()

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
generate_export_header(VulkanEngineDLL EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/vulkanenginedll_export.h)
target_precompile_headers(VulkanEngineDLL PRIVATE VulkanEngineDLL/Platform.h)
target_include_directories(VulkanEngineDLL PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/VulkanEngineDLL
    External/SPIRV-Reflect
    ${CMAKE_CURRENT_BINARY_DIR}
)
if(ANDROID)
    target_link_libraries(VulkanEngineDLL PRIVATE vulkan android log)
else()
    target_link_libraries(VulkanEngineDLL PRIVATE Vulkan::Vulkan glfw)
    if(UNIX AND NOT APPLE)
        target_link_libraries(VulkanEngineDLL PRIVATE X11 uuid pthread dl)
    endif()
endif()

# ===================================================================
# ImGui + ImPlot
# ===================================================================
set(IMGUI_SOURCES
    External/imgui/imgui.cpp
    External/imgui/imgui_draw.cpp
    External/imgui/imgui_tables.cpp
    External/imgui/imgui_widgets.cpp
    External/imgui/imgui_demo.cpp
    External/imgui/backends/imgui_impl_vulkan.cpp
)
if(ANDROID)
    list(APPEND IMGUI_SOURCES External/imgui/backends/imgui_impl_android.cpp)
else()
    list(APPEND IMGUI_SOURCES External/imgui/backends/imgui_impl_glfw.cpp)
endif()

set(IMPLOT_SOURCES
    External/ImPlot/implot.cpp
    External/ImPlot/implot_items.cpp
    External/ImPlot/implot_demo.cpp
)

# ===================================================================
# native_app_glue — ONLY FOR ANDROID
# ===================================================================
if(ANDROID)
    add_library(native_app_glue STATIC
        ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
    )
    target_include_directories(native_app_glue PUBLIC
        ${ANDROID_NDK}/sources/android/native_app_glue
    )
endif()

# ===================================================================
# Main target
# ===================================================================
set(MAIN_SOURCES
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
    ${IMPLOT_SOURCES}
)

if(ANDROID)
    add_library(VulkanGameEngine SHARED
        ${MAIN_SOURCES}
        VulkanGameEngine/AndroidMain.cpp
    )

    set_target_properties(VulkanGameEngine PROPERTIES
        OUTPUT_NAME "main"
    )

    target_link_libraries(VulkanGameEngine PRIVATE
        vulkan
        android
        log
        native_app_glue
        Physics2DDLL
        VulkanEngineDLL
        ComponentDLL
        glm::glm
        nlohmann_json::nlohmann_json
    )

    target_link_options(VulkanGameEngine PRIVATE
        "-Wl,--export-dynamic"
        "-u" "ANativeActivity_onCreate"
    )
else()
    add_executable(VulkanGameEngine ${MAIN_SOURCES})
endif()

target_include_directories(VulkanGameEngine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/VulkanGameEngine
    ${CMAKE_CURRENT_BINARY_DIR}
)

# ===================================================================
# Link everything — CLEAN AND FINAL
# ===================================================================
if(ANDROID)
    target_link_libraries(VulkanGameEngine PRIVATE
        vulkan
        android
        log
        native_app_glue       
        Physics2DDLL
        VulkanEngineDLL
        ComponentDLL
        glm::glm
        nlohmann_json::nlohmann_json
        "-Wl,--export-dynamic"
    )
else()
    target_link_libraries(VulkanGameEngine PRIVATE
        Vulkan::Vulkan
        glfw
        Physics2DDLL
        VulkanEngineDLL
        ComponentDLL
        glm::glm
        nlohmann_json::nlohmann_json
    )
    if(GLFW_HEADER_DEPS)
        add_dependencies(VulkanGameEngine ${GLFW_HEADER_DEPS})
    endif()
endif()

# ===================================================================
# Desktop post-build (unchanged)
# ===================================================================
if(NOT ANDROID)
    set(RUN_DIR "${CMAKE_INSTALL_PREFIX}/run")
    install(TARGETS VulkanGameEngine VulkanEngineDLL ComponentDLL Physics2DDLL
        RUNTIME DESTINATION "${RUN_DIR}"
        LIBRARY DESTINATION "${RUN_DIR}"
        ARCHIVE DESTINATION "${RUN_DIR}"
    )
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/Assets/" DESTINATION "${RUN_DIR}/Assets" PATTERN "Shaders/*" EXCLUDE)
    install(FILES "${CMAKE_SOURCE_DIR}/EngineConfig.json" DESTINATION "${RUN_DIR}/Config")
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
endif()  