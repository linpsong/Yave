# Yave
Yet another C++17 Vulkan engine.

## Status

![Current status](https://i.imgur.com/fLydq3W.png)
![Current status](https://im3.ezgif.com/tmp/ezgif-3-fd5d083cba.gif)

## Building
You need:
 * CMake
 * A C++17 compiler (GCC 7.1)
 * [Vulkan SDK](https://lunarg.com/vulkan-sdk/)
 * [Vulkan hpp](https://github.com/KhronosGroup/Vulkan-Hpp)
 * [Spriv_cross](https://github.com/KhronosGroup/SPIRV-Cross)
 * [y](https://github.com/gan74/y)
 * For tools:
   * [Rust](https://www.rust-lang.org/en-US/)
   * [Assimp](http://assimp.sourceforge.net/)

Currently a mess, should smooth out as I am learning Vulkan.

Implemented features:
 * Buffers
 * Images
   * Arrays
   * Cubemaps
     * IBL probes
 * Descriptor sets
 * Basic pipelines
 * Compute shaders
 * Swapchain
 * Framebuffers
 * Rendering pipeline
   * Tiled deferred shader
     * Physically based lighting
     * [IBL](https://i.imgur.com/fLydq3W.png)
   * Basic scenes
 * Meshes
   * Static
   * [Skeletal](https://im3.ezgif.com/tmp/ezgif-3-fd5d083cba.gif) 


### Licence:
MIT
