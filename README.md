Overview: “A 2D game engine built with Vulkan in C/C++20 and C# .NET 8.0 editors, featuring high-performance rendering, modular systems, and C#/C++ interop.”
Features:

High-performance Vulkan rendering with GLSL shaders for 2D graphics.
C# .NET 8.0 WinForms editors for level design, renderpass, and pipeline configuration.
DLL-based C#/C++ interop for efficient data transfer.
Modular systems (shaders, meshes, cameras) with SDL/GLFW for cross-platform support.
JSON-based configuration for scalable editor workflows.

Technologies: C, C++20, C#, .NET 8.0, Vulkan, SDL, GLFW, Silk.NET, GLSL, JSON.
Setup Instructions:

Install Vulkan SDK ($(VULKAN_SDK)).
Clone external libraries (SDL, GLFW, nlohmann JSON) via .gitmodules.
Build the C++ project (VulkanGameEngine.sln) with Visual Studio 2022 (v143 toolset).
Build the C# project with .NET 8.0 SDK.

Future Plans: Plaining on merging with Eclipse Game Engine.

Render Pass Editor View:
<img width="1920" height="1049" alt="LevelEdtior" src="https://github.com/user-attachments/assets/636e0026-a185-4eab-891f-42a8c0eccc06" />

Game Window View:
<img width="1920" height="1052" alt="GameWindow" src="https://github.com/user-attachments/assets/3fca9c2b-61e8-4254-b0e0-7614e214fece" />

