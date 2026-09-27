# Vulkan Game Engine

A high-performance, cross-platform game engine built with **.NET 8** and native **C++ Vulkan** rendering.

The engine uses a hybrid architecture: high-level systems, ECS, and tools are written in C#, while performance-critical rendering runs in native C++ through custom interop DLLs.

## Key Features

- **Hybrid .NET 8 + C++ Architecture**  
  Clean managed/native boundaries with custom interop DLLs. High-level logic in C#, critical paths in native C++.

- **Entity Component System (ECS)**  
  Implemented in C# using reflection for dynamic component management.

- **Cross-Platform Support**  
  - Windows  
  - Linux (Ubuntu) via CMake + Ninja  
  - Android (Vulkan + Android NDK)

- **Rendering**  
  - Custom C# Vulkan bindings  
  - Physically Based Rendering (PBR) pipelines  
  - Sprite-based lighting (current focus)  
  - Automated material baker with NVIDIA texture compression

- **Performance**  
  - Memory pooling to reduce GC pressure  
  - Efficient C#/C++ interop layer

- **Tools**  
  - Dynamic properties panel (Unity-style inspector)  
  - Supporting editor systems

## Tech Stack

- **Managed**: C# / .NET 8, ECS, Reflection
- **Native**: C++, Vulkan, GLFW
- **Interop**: Custom DLLs with unsafe code and Marshal
- **Build**: CMake, Ninja, Visual Studio
- **Platforms**: Windows, Linux, Android

## Build Instructions

**Windows**  
Open `VulkanGameEngine.sln` in Visual Studio 2022 or later.

**Linux (Ubuntu)**  
```bash
mkdir build && cd build
cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=Release
ninja
