# Vulkan Game Engine

Hybrid **.NET 8 + native C++ Vulkan** runtime: C# for systems, ECS, and tools; native C++ for rendering and hot memory.

The C# side loads native DLLs through an explicit interop layer (P/Invoke, ownership rules, memory pools). The desktop editor lives in a sibling repo and hosts this runtime inside WinForms.

## Screenshots

C# editor hosting this runtime (object list, embedded viewport, component inspector):

![Editor hosting the runtime](https://github.com/user-attachments/assets/f01fb2c7-f7ed-456c-acf6-230501243db8)

Point lights / HDRI driven from the C# property panel:

![Lighting from the editor](https://github.com/user-attachments/assets/1b533940-0f0c-461e-a481-b1fc21c60a0d)

Full editor: [VulkanGameEngineLevelEditor](https://github.com/ThomasDHZ/VulkanGameEngineLevelEditor)

## Architecture

```text
Level editor (C# WinForms)
  → C# wrappers + ECS / config
    → ListPtr<> + P/Invoke
      → native Vulkan runtime + material baker
```

Core types and the interop boundary: [VulkanEngineCore](https://github.com/ThomasDHZ/VulkanEngineCore)

## Features

- Hybrid .NET 8 + C++ with explicit DLL boundaries
- ECS on the C# side
- Memory pooling to cut managed-heap / GC pressure
- Custom C# Vulkan bindings + native Vulkan renderer
- PBR path and sprite lighting
- Material baker (packed textures + JSON)
- Windows, Linux (CMake + Ninja), Android (NDK)

## Tech stack

| Layer | Tech |
|---|---|
| Managed | C# / .NET 8, ECS |
| Native | C++, Vulkan, GLFW |
| Interop | Custom DLLs, unsafe, Marshal, ListPtr<> |
| Build | Visual Studio, CMake, Ninja |
| Platforms | Windows, Linux, Android |

## Related repos

- [VulkanEngineCore](https://github.com/ThomasDHZ/VulkanEngineCore) — interop + memory core
- [VulkanGameEngineLevelEditor](https://github.com/ThomasDHZ/VulkanGameEngineLevelEditor) — C# WinForms editor
- [ListPtr](https://github.com/ThomasDHZ/ListPtr) — dense C# ↔ native buffers
- [MemoryLeakReporterDemo](https://github.com/ThomasDHZ/MemoryLeakReporterDemo) — leak reports for native DLLs called from C#

## Build

**Windows**  
Open `VulkanGameEngine.sln` in Visual Studio 2022 or later.

**Linux (Ubuntu)**

```bash
mkdir build && cd build
cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=Release
ninja
```

Sibling repos (`VulkanEngineCore`, editor) need to sit next to this tree if you are building the full editor + runtime path.
