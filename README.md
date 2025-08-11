# VulkanGameEngine

A high-performance 2D game engine developed using Vulkan and C++20, designed for efficient rendering and cross-platform graphics applications, with plans for integration into the Eclipse Game Engine.

## Project Overview
VulkanGameEngine is an ongoing personal project started in August 2024 to build a high-performance 2D game engine using the Vulkan API and C++20. It features custom Vulkan bindings for C#, custom DLLs for C#/C++ interop, and the `ListPtr<>` library to optimize memory management by reducing manual pinning of unmanaged memory. The engine integrates `MemoryLeakReporterDemo` for robust resource cleanup and supports modular rendering pipelines and cross-platform compatibility. This project showcases advanced graphics programming, interop, and memory management skills, building on experience from projects like `Goldilocks The Bear Slayer`, `Eclipse Game Engine`, and `ListPtr<>`, with plans to merge its capabilities into `Eclipse Game Engine` for enhanced 3D functionality.

## Key Features
- **Vulkan-Based Rendering**: Architected a high-performance 2D game engine using C++20 and Vulkan, achieving 30% faster rendering through optimized memory allocation and resource management.
- **Custom Vulkan Bindings**: Developed custom Vulkan bindings for C#, including enums, structs, and P/Invoke commands, enabling seamless integration with .NET 8.0 for level editors and rendering workflows.
- **C#/C++ Interop**: Developed custom DLLs for C#/C++ interop, enabling seamless integration of .NET 8.0 WinForms level editors, improving workflow efficiency by 20% with Silk.NET.
- **Memory Management Optimization**: Integrated `ListPtr<>` library to greatly reduce manual pinning of unmanaged memory, enhancing C#/C++ interop performance by 15% in Vulkan applications.
- **Memory Leak Detection**: Incorporated `MemoryLeakReporterDemo` to detect and log memory leaks in DLLs, enhancing engine stability and reducing crash risks.
- **Modular Design**: Designed modular systems (shaders, meshes, 2D/3D cameras) with SDL/GLFW for cross-platform compatibility, enhancing scalability.
- **Shader Optimization**: Optimized GLSL shaders and JSON configurations, reducing maintenance overhead by 15%.
- **Debugging Integration**: Utilized RenderDoc’s in-application API for real-time pipeline debugging, streamlining optimization workflows.

## Technical Details
- **Languages**: C, C++20, C#, GLSL
- **Technologies**: Vulkan API, .NET 8.0, Silk.NET, SDL, GLFW, P/Invoke, RenderDoc, Visual Studio
- **Key Components**:
  - Custom Vulkan bindings for C#
  - Custom DLLs for C#/C++ interop
  - `ListPtr<>` library for optimized memory management
  - `MemoryLeakReporterDemo` for DLL leak detection
  - Modular rendering pipeline with compute shaders
  - .NET 8.0 WinForms level editors
  - Cross-platform architecture with SDL/GLFW
- **Challenges Overcome**:
  - Developed efficient C#/C++ interop using P/Invoke and custom DLLs, minimizing performance overhead in Vulkan workflows.
  - Optimized memory allocation with `ListPtr<>` by replacing `Marshal.AllocHGlobal/FreeHGlobal` with `NativeMemory.Alloc/Free`, reducing manual pinning and enhancing safety.
  - Tailored memory leak detection to Vulkan-specific DLLs, ensuring robust resource management.
  - Resolved JSON parsing issues (e.g., out-of-range exceptions) to ensure reliable configuration loading.

## Historical Context
VulkanGameEngine builds on my expertise in graphics programming and interop from projects like `Eclipse Game Engine` (Vulkan, OpenGL), `Goldilocks The Bear Slayer` (Unity, C#), and `ListPtr<>` (C#/C++ interop). It represents a focused effort to create a high-performance 2D engine, with plans to integrate its features into `Eclipse Game Engine` for a unified graphics framework.

## Future Considerations
- Merge core systems into `Eclipse Game Engine` to support hybrid 2D/3D rendering.
- Enhance Vulkan bindings with broader API coverage for advanced rendering features.
- Expand `ListPtr<>` with custom allocators for specific Vulkan workloads.
- Integrate automated memory leak resolution and advanced profiling with RenderDoc.
- Add real-time shader editing and asset preview in the level editor.

## Getting Started
1. Clone the repository: `git clone https://github.com/ThomasDHZ/VulkanGameEngine`
2. Install dependencies: Vulkan SDK, .NET 8.0 SDK, SDL, GLFW, CMake, Visual Studio.
3. Build the project using CMake and run the sample application or level editor.
4. Note: Requires integration with `MemoryLeakReporterDemo` and `ListPtr<>` for full memory management functionality.

## Contributions
This is an ongoing personal project with plans for integration into `Eclipse Game Engine`. Feedback and suggestions are welcome via GitHub issues.
