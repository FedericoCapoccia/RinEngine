![RIN](./RinLogo.png)

## About

A personal game engine project created as a learning platform for exploring low-level graphics APIs (Vulkan, DirectX 12), physics and math for games, and software architecture principles.

## Building from Source
Ensure you have the required dependencies:
- [CMake](https://github.com/Kitware/CMake) and [Ninja](https://github.com/ninja-build/ninja)
- [VulkanSDK](https://vulkan.lunarg.com/) with $VULKAN_SDK env variable set
- On Windows: [w64devkit](https://github.com/skeeto/w64devkit) added to the path

In the root directory:
```
cmake --preset debug/release
cmake --build build
```

If you want to peek at the code and you are using clangd as the LSP, copy one of the .clangd-platform files to .clangd as the project automatically generates the compile_commands.json but on windows it cannot find w64devkit's system headers without specifying the target as 'x86_64-w64-windows-gnu'.