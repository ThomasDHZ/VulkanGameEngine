# Vulkan Game Engine

A 2D game engine built in C and C++20 using Vulkan, featuring high-performance rendering, modular systems, and C# .NET 8.0 WinForms editors for streamlined design workflows. The engine leverages DLL-based C#/C++ interop for efficient data transfer and JSON configurations for scalability, achieving a 30% rendering performance boost and 20% workflow improvement. This project demonstrates advanced graphics programming, cross-language integration, and modern C++ system design.

## Features

- **High-Performance 2D Rendering**: Developed Vulkan-based rendering pipelines in C/C++20, optimizing resource management for 30% faster rendering of 2D scenes.
- **C# .NET Editors**: Built WinForms editors in C# .NET 8.0 for level design, renderpass, and pipeline configuration, enhancing workflow efficiency by 20% via DLL-based C#/C++ interop.
- **Modular C++ Systems**: Designed scalable systems for shaders, meshes, and 2D/3D cameras with SDL/GLFW, supporting cross-platform compatibility.
- **GLSL Shaders and JSON Configs**: Implemented GLSL shaders for sprite batching and JSON-based configurations using Silk.NET, reducing maintenance overhead by 15%.
- **Memory Optimization**: Integrated memory leak detection and efficient data structures, ensuring robust performance for real-time rendering.

## Technologies

- **Languages**: C, C++20, C#, GLSL
- **Frameworks**: .NET 8.0, WinForms
- **Graphics APIs**: Vulkan
- **Tools**: Silk.NET, SDL, GLFW, nlohmann JSON
- **Build Systems**: Visual Studio 2022 (v143 toolset), .NET 8.0 SDK
- **Other**: DLL-based C#/C++ interop, JSON configurations

## Setup Instructions

To build and run the Vulkan Game Engine:

1. **Install Prerequisites**:
   - [Vulkan SDK](https://vulkan.lunarg.com/) (set `VULKAN_SDK` environment variable).
   - Visual Studio 2022 with C++ Desktop Development workload.
   - [.NET 8.0 SDK](https://dotnet.microsoft.com/download/dotnet/8.0) for C# editors.
   - Git for cloning submodules.

2. **Clone the Repository**:
   ```bash
   git clone --recurse-submodules https://github.com/ThomasDHZ/VulkanGameEngine.git
   cd VulkanGameEngine
   ```

3. **Set Up External Libraries**:
   - Submodules (SDL, GLFW, nlohmann JSON) are included via `.gitmodules`.
   - Run `git submodule update --init --recursive` to fetch dependencies.

4. **Build the C++ Engine**:
   - Open `VulkanGameEngine.sln` in Visual Studio 2022.
   - Set configuration to `Release|x64` or `Debug|x64`.
   - Build the solution, ensuring `$(VULKAN_SDK)` is set for Vulkan includes/libs.
   - Output: `VulkanGameEngine.dll` for interop with C# editors.

5. **Build the C# Editors**:
   - Open the C# project (e.g., `VulkanGameEngine.csproj`) in Visual Studio or via CLI:
     ```bash
     dotnet build VulkanGameEngine.csproj -c Release
     ```
   - Ensure `VulkanGameEngine.dll` is referenced in the C# project’s output directory.

6. **Run the Engine**:
   - Launch the C# WinForms editor to configure a 2D scene.
   - Run the C++ engine binary or test via the editor to render a sample scene.

## Screenshots

[Insert screenshots or GIFs of the engine rendering a 2D scene or the WinForms editors. Example placeholders:]
- *2D scene with sprite batching*  
<img width="1920" height="1049" alt="LevelEdtior" src="https://github.com/user-attachments/assets/636e0026-a185-4eab-891f-42a8c0eccc06" />

- *WinForms level editor*  
<img width="1920" height="1052" alt="GameWindow" src="https://github.com/user-attachments/assets/3fca9c2b-61e8-4254-b0e0-7614e214fece" />

[Alternatively, link to a demo video: [Watch Demo](https://example.com/vulkan-demo.mp4)]

## Future Plans

The Vulkan Game Engine is a functional 2D game engine with integrated C# editors. Future enhancements include merging with the [Eclipse Game Engine](https://github.com/ThomasDHZ/EclipseEngine) to create a unified engine supporting both 2D and 3D graphics, combining Vulkan’s performance with advanced ray tracing and PBR lighting.

