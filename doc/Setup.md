# Gaemi development setup

## Toolchain

To develop around Gaemi, you will need:
- Vulkan
- SDL2
- CMake (to ease cross platform development)

Gaemi is meant to be used with a CMake compatible IDE. Visual Code configuration is present, and this document's author 
(Gaetz) uses Jetbrain CLion.

### Linux tools setup

This manual is an Ubuntu / PopOS manual. Adapt it to your distribution.

#### CMake
First, you need to install cmake. You can follow this tutorial: https://vitux.com/how-to-install-cmake-on-ubuntu/

#### Vulkan
The process is different if you use an AMD graphics card, or an NVidia graphics card.

##### Vulkan for AMD

It is best to enable a PPA for the latest Mesa drivers. There is a PPA that packages and releases the latest changes 
straight from Mesa's Git. Add the PPA to your system and update. Then, upgrade your system. It will automatically 
upgrade your existing Mesa packages.
```
sudo add-apt-repository ppa:oibaf/graphics-drivers
sudo apt update
sudo apt upgrade
```
When it's done, install the Vulkan packages.
```
apt install libvulkan1 mesa-vulkan-drivers vulkan-utils
```

##### Vulkan for NVidia
Ubuntu also has a great repository for the NVIDIA proprietary drivers. Add it to your system, and update Apt.
```
sudo add-apt-repository ppa:graphics-drivers/ppa
sudo apt upgrade
```
Now, install your drivers and Vulkan.
```
sudo apt install nvidia-graphics-drivers-396 nvidia-settings vulkan vulkan-utils
```

#### Vulkan from sources
If you want a specific vulkan version, you can get it from: https://vulkan.lunarg.com/sdk/home#linux

#### SDL2
The SDL2 version on Ubuntu distributions is often out of date. It is better to download SDL2 sources here:
https://www.libsdl.org/download-2.0.php

Then un-tar those sources and install them. This installation requires build-essential.

```
sudo apt install build-essential

// Now get into untared SDL2 dir and install it

cd SDL2-2.x.x
./configure
make all
sudo make install
```

### Windows tools setup

Under Windows, Gaemi is developed using Visual Code. Configuration files are already included.

Install Visual Code from here: https://code.visualstudio.com/

Install CMake from here: https://cmake.org/download/

Install Vulkan from here: https://vulkan.lunarg.com/sdk/home#windows

Note that the last vulkan version you can use without validation warnings is determined by your graphics card. 

SDL2 is already included in the `externals` folder. You do not need to install it.

### Visual code extensions

In case you use Visual Code, it is recommended to use:
- C++ extension
- CMake and CMakeTools extensions
- GLSL support extension


## Compiler

You are free to use any compiler, and CMake will help you in that. Nevertheless, it was chosen to use the same compiler
under Windows and Linux, namely the clang compiler.

### Use clang on Linux

#### Install
The process is straightforward:
```
sudo apt install clang
```

#### Configure CLion
Under linux, CLion uses g++ as a C++ compiler by default. To use clang, follow this path
- Go to File/Settings/Toolchains
- Create a new toolchain with the + button, name it Clang
- Change the C compiler to `/usr/bin/clang`
- Change the C++ compiler to `/usr/bin/clang++`
- Change the debugger to the bundled LLDB

### Install clang on windows

#### Install
Get the last LLVM version from https://releases.llvm.org/

After installation, add the clang executable directory to your PATH environment variable.

#### Ninja compiler
ninja replaces make to build your project. Here is the process to install it:

- Download from https://github.com/ninja-build/ninja/releases

OR 

- build from source:

```
git clone git://github.com/martine/ninja.git
cd ninja/
cmake .
```

- Then open ninja.sln with Visual Studio, set project to release, build solution. 
- Add to your path the `ninja/Release` folder. 
- Don't forget to restart your terminal / IDE to update its terminal path.

## Dependencies

The `externals` folder contains dependencies used by the game engine.

In this folder, the `dependencies.txt` file list those dependencies and versions. The `CMakeLists.txt` file exports
needed libs.

## Build

The `CMakeLists.txt` files series handles compilation and linking. For now this structure is too fluid to be documented.

Note the Gaemi game engine, in folder `GEngine`, is exported as a shared library (.dll / .so file). Exported functions
are preceded by the `GAPI` macro.

