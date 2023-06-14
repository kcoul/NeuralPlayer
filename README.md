# NeuralPlayer

# Building From Source

## Prerequisites
1. Download Intel OneAPI's Online Installer:
 
https://www.intel.com/content/www/us/en/developer/tools/oneapi/base-toolkit-download.html

2. Run it, and select Custom Installation. De-select all components except Intel OneAPI Math Kernel Library.
3. Allow for the installation to complete.

### Linux

1. Build OpenBLAS and move all output libraries and symlinks to ~/OpenBLAS/lib": https://github.com/xianyi/OpenBLAS. 
This should result in there being valid libopenblas.a and libopenblas.so symlinks available at that location for SpleeterRT to link to.

```bash
git clone https://github.com/xianyi/OpenBLAS.git
cd OpenBLAS
make
mkdir -p ~/OpenBLAS/lib
cp *.a *.so *.so.0 ~/OpenBLAS/lib
```

2. Install JUCE dependencies listed here: https://github.com/kcoul/JUCE4Pi

## SpleeterRT Notes

1. Anytime new CMake build config is set up, SpleeterRTBin should be compiled for that build type (i.e. Debug, Release, RelWithDebInfo) before NeuralPlayer is run with that config
2. You should set a "HOME" user environment variable on Windows for SpleeterRT resource management
3. To get separated audio by Spleeter, run `SpleeterRTBin` build configuration with _program arguments_
`spawnNthreads timeStep analyseBinLimit stems audioFile`, e.g. by clicking _Run_ -> _Edit Configuration_ -> choosing _SpleeterRTBin_ and under _Program arguments_ inserting `3 512 1024 3 ../../../Player/Resources/TrueSurvivor.wav`.
After running it, you get separated audio channels in `cmake-build-debug/SpleeterRT/Executable/` folder.


## Run-time Limitations

1. Currently only supports 44.1kHz ```.wav``` format input, more formats and sample rates will be added over time

## Additional Prerequisites for workbenching (Use workbench2 branch)

1. Stock ```spleeter``` and ```basic-pitch```, which combined require python@3.10 and tensorflow@2.11
2. You may need to uninstall python@3.11 and/or downgrade tensorflow. 

```
pip install basic-pitch
pip install spleeter==2.3
```

## Additional Prerequisites for Visualizers (Milkdrop) 

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

## How To Pass Arguments to ProjectM / MSDL2 via NeuralPlayer
1. Determine which arguments are applicable to your usage (see below)
2. Extend the ```arguments``` StringArray at L.22 of VisualizerThread.h accordingly
```
    "help", "h", "Display this help text and exit."

    "listAudioDevices", "l", "Output a list of available audio recording devices on startup."

    "audioDevice", "d", "Select an audio device to record from initially. Can be the numerical ID or the full device name. If the device is not found, the default device will be used instead."

    "presetPath", "p", "Base directory to search for presets."

    "texturePath", "", "Additional path with textures/images."

    "enableSplash", "s", "If true, initially displays the built-in projectM logo preset."

    "fullscreen", "f", "Start in fullscreen mode."

    "exclusive", "e", "Use exclusive fullscreen mode. If true, this will change display resolution on most platforms to best match the window resolution."

    "monitor", "", "Displays the window on the given monitor. 0 uses OS default window position, 1 is the primary display and so on."

    "vsync", "", "If true, waits for vertical sync to avoid tearing, but limits max FPS to the vsync interval."

    "width", "", "Initial window width."

    "height", "", "Initial window height."

    "fullscreenWidth", "", "Fullscreen horizontal resolution."

    "fullscreenHeight", "", "Fullscreen vertical resolution."

    "left", "", "Initial window X position."

    "top", "", "Initial window Y position."

    "fps", "", "Target frames per second rate."

    "shuffleEnabled", "", "Shuffle enabled."

    "presetDuration", "", "Preset duration. Any number > 1, default 30."

    "transitionDuration", "", "Transition duration. Any number >= 0, default 3."

    "hardCutsEnabled", "", "Hard cuts enabled."

    "hardCutDuration", "", "Hard cut duration. Any number > 1, default 20."

    "hardCutSensitivity", "", "Hard cut sensitivity. Between 0.0 and 5.0. Default 1.0."

    "beatSensitivity", "", "Beat sensitivity. Between 0.0 and 5.0. Default 1.0."
```

## (Optional) Building OnnxRuntime from Source

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


