# Vulkan Game Engine

A high-performance, cross-platform game engine built with **.NET 8** and native C++ Vulkan rendering.

Designed to demonstrate advanced .NET systems programming, C#/C++ interop, and modern rendering techniques while maintaining excellent performance and cross-platform compatibility.

## Key Features

- **.NET 8 + C++ Hybrid Architecture**  
  Core rendering and performance-critical systems run in native C++ through custom interop DLLs, while high-level architecture, ECS, and tools are built in C#.

- **Entity Component System (ECS)**  
  Built with C# reflection and recursion for dynamic component management and scalable system traversal.

- **Cross-Platform Support**  
  - Windows  
  - Linux (Ubuntu) via CMake  
  - Android (Vulkan Mobile + Android NDK)

- **High-Performance Rendering**  
  - Custom C# Vulkan bindings  
  - Physically Based Rendering (PBR) pipelines  
  - Sprite-based lighting (current 2D focus)  
  - Automated material baker with NVIDIA texture compression

- **Performance Optimizations**  
  - Memory pooling for efficient object caching and reduced GC pressure  
  - Dynamic properties panel (Unity-style inspector) for real-time editing

- **Build & Development**  
  - Fully configured Linux build environment on virtual server (CMake + Ninja)  
  - Clean separation between managed .NET code and native C++ renderer

## Tech Stack

- **Managed**: C# / .NET 8, Entity Component System, Reflection
- **Native**: C++, Vulkan, GLFW
- **Interop**: Custom C#/C++ DLL bindings with unsafe code and Marshal
- **Build**: CMake, Ninja
- **Platforms**: Windows, Linux, Android

## Screenshots

*(Add 2–4 screenshots here once you have good ones — especially the hall scene, material tests, and any in-engine editor views)*

## Build Instructions

**Linux (Ubuntu)**
```bash
mkdir build && cd build
cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=Release
ninja'''

Windows
Open the solution in Visual Studio 2022+ or use CMake with Visual Studio generator.
Android
See VulkanGameEngineAndroid folder (NDK + Gradle build).
Purpose
This project was created to deepen expertise in:

High-performance .NET systems development
C#/C++ interop and native integration
Cross-platform development and deployment
Modern rendering techniques and optimization

It serves as a practical demonstration of advanced .NET capabilities beyond typical web/API work.
