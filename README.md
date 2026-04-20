# Vulkan Game Engine

A high-performance, cross-platform engine built with **.NET 8** and native **C++ Vulkan** rendering.

The architecture uses C# for high-level systems (ECS, tools, editor) and native C++ for performance-critical rendering via custom interop DLLs. Current focus is 2D rendering with sprite-based lighting, while PBR pipelines are also implemented.

## Key Features

- **Hybrid .NET 8 + C++ Architecture**  
  Core rendering runs in native C++ through custom interop DLLs, while high-level architecture, ECS, and tools are built in C#.

- **Entity Component System (ECS)**  
  Implemented in C# using reflection and recursion for dynamic component management and scalable system traversal.

- **Cross-Platform Support**  
  - Windows  
  - Linux (Ubuntu) via CMake + Ninja  
  - Android (Vulkan Mobile + Android NDK)

- **High-Performance Rendering**  
  - Custom C# Vulkan bindings  
  - Physically Based Rendering (PBR) pipelines  
  - Sprite-based lighting (current focus)  
  - Automated material baker with NVIDIA texture compression

- **Performance Optimizations**  
  - Memory pooling to reduce GC pressure and improve runtime speed  
  - Dynamic properties panel (Unity-style inspector) for real-time editing

- **Build & Development**  
  - Fully configured Linux build environment on a virtual server (CMake + Ninja)  
  - Clean separation between managed .NET code and native C++ renderer

## Tech Stack

- **Managed**: C# / .NET 8, Entity Component System, Reflection  
- **Native**: C++, Vulkan, GLFW  
- **Interop**: Custom C#/C++ DLL bindings with unsafe code and Marshal  
- **Build**: CMake, Ninja  
- **Platforms**: Windows, Linux, Android

## Build Instructions

**Linux (Ubuntu)**
mkdir build && cd build
cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=Release
ninja

## Windows
Open VulkanGameEngine.sln in Visual Studio 2022 or later.

## Purpose
This project demonstrates advanced .NET systems programming skills, including:

High-performance C#/C++ interop and native integration
Cross-platform development and deployment
Modern rendering techniques and performance optimization
Clean architecture in a mixed-language environment
