# NeuralPlayer

## Prerequisites
1. Download Intel OneAPI's Online Installer (Shortcuts to latest installers, May 2023):
 
(macOS) https://www.intel.com/content/www/us/en/developer/tools/oneapi/base-toolkit-download.html?operatingsystem=mac&distributions=online&version=2023.1

(Linux) https://www.intel.com/content/www/us/en/developer/tools/oneapi/base-toolkit-download.html?operatingsystem=linux&distributions=online&version=2023.1

(Windows) https://www.intel.com/content/www/us/en/developer/tools/oneapi/base-toolkit-download.html?operatingsystem=window&distributions=online&version=2023.1

2. Run it, and select Custom Installation. De-select all components except Intel OneAPI Math Kernel Library.
3. Allow for the installation to complete.

### macOS
```
brew install python3
brew install cmake
```

### Linux
```
apt install python3
apt install cmake
```
### Windows
https://www.python.org/downloads/windows/

https://cmake.org/download/

## Build-time Dependencies 
1. Clone NeuralPlayer and configure CMake in Debug mode once using ```cmake-build-debug``` as build directory (CLion does this automatically)
2. Build onnxruntime manually once to generate sub-build CMake Cache (see https://onnxruntime.ai/docs/build/inferencing.html):

``` cd ~/repos/NeuralPlayer/cmake-build-debug/_deps/onnxruntime-src ```

Example: macOS (Intel): 

``` ./build.sh --config Release --build_shared_lib --parallel --cmake_extra_defines CMAKE_OSX_ARCHITECTURES=x86_64 ```

Example: macOS (M1+): 

``` ./build.sh --config Release --build_shared_lib --parallel --cmake_extra_defines CMAKE_OSX_ARCHITECTURES=arm64 ```

3. Thereafter CMake should be able to rebuild onnxruntime anytime it changes, unless the Debug build directory is wiped
4. Anytime new CMake build config is set up, SpleeterRTBin should be compiled for that build type (i.e. Debug, Release, RelWithDebInfo) before NeuralPlayer is run with that config

## Run-time Limitations
1. Only tested on Intel macOS so far
2. Currently only supports 44.1kHz .wav format input
