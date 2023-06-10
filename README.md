# NeuralPlayer

# Building From Source

## Prerequisites
1. Download Intel OneAPI's Online Installer:
 
https://www.intel.com/content/www/us/en/developer/tools/oneapi/base-toolkit-download.html

2. Run it, and select Custom Installation. De-select all components except Intel OneAPI Math Kernel Library.
3. Allow for the installation to complete.

### Linux

Additional JUCE dependencies listed here: https://github.com/kcoul/JUCE4Pi

## SpleeterRT Notes

1. Anytime new CMake build config is set up, SpleeterRTBin should be compiled for that build type (i.e. Debug, Release, RelWithDebInfo) before NeuralPlayer is run with that config
2. You should set a "HOME" user environment variable on Windows for SpleeterRT resource management

## Run-time Limitations

1. Currently only supports 44.1kHz ```.wav``` format input, more formats and sample rates will be added over time

## Additional Prerequisites for workbenching (Use workbench2 branch)

1. Stock ```spleeter``` and ```basic-pitch```, which combined require python@3.10 and tensorflow@2.11
2. You may need to uninstall python@3.11 and/or downgrade tensorflow. 

```
pip install basic-pitch
pip install spleeter==2.3
```

## Additional Prerequisites for visuals (Use visualizer1 branch)

### macOS (homebrew)
```
brew install sdl2
brew install poco
brew install boost
```

### Linux

```
sudo apt install libsdl2-dev
sudo apt install libpoco-dev
sudo apt install libboost-all-dev
```

### Windows

1. Install GLEW, SDL2, Poco, and Boost using vcpkg in CLion


## Building OnnxRuntime from Source

Prebuilt binaries of OnnxRuntime (v1.14.1) are provided for all platforms. You should only build OnnxRuntime from source if necessary.

## Prerequisites

### macOS (homebrew)
```
brew install python@3.10
brew install cmake
```

### Linux
```
apt install python@3.10
apt install cmake 
```

### Windows
https://www.python.org/downloads/windows/

https://cmake.org/download/

## Instructions

1. Clone NeuralPlayer and configure CMake in Debug mode once using ```cmake-build-debug``` as build directory (CLion does this automatically)
2. Add ```-DBUILD_ONNXRUNTIME_FROM_SOURCE=1``` to your CMake build flags
3. Build onnxruntime manually once (CMake 3.24 or higher is required, use ```snap``` on Linux) to generate sub-build CMake Cache (see https://onnxruntime.ai/docs/build/inferencing.html):

``` cd ~/repos/NeuralPlayer/cmake-build-debug/_deps/onnxruntime-src ```

Example: macOS (Intel): 

``` ./build.sh --config RelWithDebInfo --build_shared_lib --parallel --cmake_extra_defines CMAKE_OSX_ARCHITECTURES=x86_64 ```

Example: macOS (M1+): 

``` ./build.sh --config RelWithDebInfo --build_shared_lib --parallel --cmake_extra_defines CMAKE_OSX_ARCHITECTURES=arm64 ```

Example: Linux:

``` ./build.sh --config RelWithDebInfo --build_shared_lib --parallel ```

Example: Windows 10/11:

``` .\build.bat --config RelWithDebInfo --build_shared_lib --parallel ```

Thereafter CMake should be able to rebuild onnxruntime anytime it changes, unless the Debug build directory is wiped


