# NeuralPlayer

## Pre-requisites
1. Download Intel OneAPI's Online Installer (Shortcuts to latest installers, May 2023):
 
(macOS) https://www.intel.com/content/www/us/en/developer/tools/oneapi/base-toolkit-download.html?operatingsystem=mac&distributions=online&version=2023.1

(Linux) https://www.intel.com/content/www/us/en/developer/tools/oneapi/base-toolkit-download.html?operatingsystem=linux&distributions=online&version=2023.1

(Windows) https://www.intel.com/content/www/us/en/developer/tools/oneapi/base-toolkit-download.html?operatingsystem=window&distributions=online&version=2023.1

2. Run it, and select Custom Installation. De-select all components except Intel OneAPI Math Kernel Library.
3. Allow for the installation to complete.

### macOS
```
brew install python3
```

### Linux
```
apt install python3
```
### Windows
https://www.python.org/downloads/windows/

## Build-time Dependencies 
1. Clone NeuralPlayer once and run CMake once using ```cmake-build-debug``` as build directory (CLion does this automatically)
2. Build onnxruntime manually once to generate CMake Cache (see https://onnxruntime.ai/docs/build/inferencing.html):
``` cd <path-to-NeuralPlayer/cmake-build-debug/_deps/onnxruntime-src ```
macOS(Intel): ``` ./build.sh --config RelWithDebInfo --build_shared_lib --parallel --cmake_extra_defines CMAKE_OSX_ARCHITECTURES=x86_64 ```
3. Thereafter CMake should be able to rebuild onnxruntime anytime it changes, unless the build directory is wiped
