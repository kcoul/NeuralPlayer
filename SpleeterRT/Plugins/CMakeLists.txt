cmake_minimum_required(VERSION 3.23.2)

set(TargetName "SpleeterRTPlug")

# Valid formats: AAX Unity VST AU AUv3 Standalone LV2
set(FORMATS AU VST3 LV2 Standalone)

project(${TargetName} VERSION 0.0.1)

# No ZERO_CHECK target on XCode (it helps bust cache for cmake)
if(CMAKE_GENERATOR STREQUAL Xcode)
    set(CMAKE_SUPPRESS_REGENERATION true)
endif()

# Not building universals: we maintain different build machines for each due to tensorflow's issues
# set(CMAKE_OSX_ARCHITECTURES arm64 x86_64)

# clang-tidy needs this to be happy
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Adds all the module sources so they appear correctly in the IDE
# Must be set before JUCE is added as a sub-dir (or any targets are made)
# https://github.com/juce-framework/JUCE/commit/6b1b4cf7f6b1008db44411f2c8887d71a3348889
set_property(GLOBAL PROPERTY USE_FOLDERS YES)

# This is a failed attempt to bury ALL_BUILD in Targets/
# This should be called before any target is made
# Bug in Xcode? https://gitlab.kitware.com/cmake/cmake/-/issues/21383
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "Targets")

# Create a /Modules directory in the IDE with the JUCE Module code
option(JUCE_ENABLE_MODULE_SOURCE_GROUPS "Show all module sources in IDE projects" ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake")
find_package(JUCE)

# Check the readme at `docs/CMake API.md` in the JUCE repo for full config
juce_add_plugin("${TargetName}"
        # VERSION ...                               # Set this if the plugin version is different to the project version
        # ICON_BIG ...                              # ICON_* arguments specify a path to an image file to use as an icon for the Standalone
        # ICON_SMALL ...
        COMPANY_NAME james34602
        BUNDLE_ID com.james34602.SpleeterRT
        # IS_SYNTH TRUE/FALSE                       # Is this a synth or an effect?
        # NEEDS_MIDI_INPUT TRUE/FALSE               # Does the plugin need midi input?
        # NEEDS_MIDI_OUTPUT TRUE/FALSE              # Does the plugin need midi output?
        # IS_MIDI_EFFECT TRUE/FALSE                 # Is this plugin a MIDI effect?
        # EDITOR_WANTS_KEYBOARD_FOCUS TRUE/FALSE    # Does the editor need keyboard focus?
        LV2URI "http://com.james34602.SpleeterRT"
        COPY_PLUGIN_AFTER_BUILD TRUE # On MacOS, plugin will be copied to /Users/username/Library/Audio/Plug-Ins/
        PLUGIN_MANUFACTURER_CODE Jamz
        PLUGIN_CODE Sprt                            # A unique four-character plugin id with at least one upper-case character
        FORMATS "${FORMATS}"
        PRODUCT_NAME "${TargetName}")        # The name of the final executable, which can differ from the target name


target_include_directories(${TargetName} PRIVATE "")

juce_generate_juce_header(${TargetName})

# Enable C++20
#target_compile_features("${NFTargetName}" PRIVATE cxx_std_20)

# Manually list all .h and .cpp files for the plugin, don't use globs:
set(SourceFiles
        Source/codelet.c
        Source/codelet.h
        Source/conv_util.h
        Source/cpthread.c
        Source/cpthread.h
        Source/gemm.c
        Source/im2col_dilated.c
        Source/PluginProcessor.cpp
        Source/spleeter.c
        Source/spleeter.h
        Source/Spleeter4Stems.c
        Source/Spleeter4Stems.h
        Source/stb_image_write.h)
target_sources("${TargetName}" PRIVATE ${SourceFiles})

#Resources
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Resources/accompaniment4stems.dat OR
   NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Resources/bass4stems.dat OR
   NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Resources/drum4stems.dat OR
   NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Resources/vocal4stems.dat)
    set(resource_source_path "https://github.com/james34602/SpleeterRT/releases/download/v0.2-alpha/coefficients.7z")
    file(DOWNLOAD
            ${resource_source_path}
            ${CMAKE_CURRENT_SOURCE_DIR}/Resources/coefficients.7z
            SHOW_PROGRESS
            )
    execute_process(
            COMMAND ${CMAKE_COMMAND} -E tar xf ${CMAKE_CURRENT_SOURCE_DIR}/Resources/coefficients.7z
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Resources
    )
endif()
execute_process(
        COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_CURRENT_SOURCE_DIR}/Resources/coefficients.7z
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Resources
)

# We don't want our source buried in extra nested folders
set_target_properties("${TargetName}" PROPERTIES FOLDER "")

# The source tree should still look like the source tree
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/Source PREFIX "" FILES ${SourceFiles})

# Setup our binary data as a target
juce_add_binary_data(SpleeterRTPlug-Assets SOURCES JuceIcon.jpeg)

# Required for Linux happiness:
# See https://forum.juce.com/t/loading-pytorch-model-using-binarydata/39997/2
set_target_properties(SpleeterRTPlug-Assets PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

# This cleans up the folder organization, especially on Xcode.
# It tucks the Plugin varieties into a "Targets" folder and generate an Xcode Scheme manually
# Xcode scheme generation is turned off globally to limit noise from other targets
# The non-hacky way of doing this is via the global PREDEFINED_TARGETS_FOLDER property
# However that doesn't seem to be working in Xcode
# Not all plugin types (au, vst) available on each build type (win, macos, linux)
foreach(target ${FORMATS} "All")
    if(TARGET ${TargetName}_${target})
        set_target_properties(${TargetName}_${target} PROPERTIES
                # Tuck the actual plugin targets into a folder where they won't bother us
                FOLDER "Targets"

                # MacOS only: Sets the default executable that Xcode will open on build
                # For this exact path to to work, manually build the AudioPluginHost.xcodeproj in the JUCE subdir
                XCODE_SCHEME_EXECUTABLE "~/repos/JUCE/extras/AudioPluginHost/Builds/MacOSX/build/Debug/AudioPluginHost.app"

                # Let us build the target in Xcode
                XCODE_GENERATE_SCHEME ON)
    endif()
endforeach()
set_target_properties(SpleeterRTPlug-Assets PROPERTIES FOLDER "Targets")

# We'll need to link to these from our plugin as well as our tests
set(JUCE_DEPENDENCIES
        juce::juce_audio_utils)

target_compile_definitions("${TargetName}"
        PUBLIC
        # JUCE_WEB_BROWSER and JUCE_USE_CURL are enabled by default
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_include_directories(${TargetName} PUBLIC "~/intel/oneapi/mkl/latest/include")
    target_link_directories(${TargetName} PUBLIC
            "~/intel/oneapi/mkl/latest/lib/intel64"
            "~/OpenBLAS/lib")
#elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
#    target_include_directories(${TargetName} PUBLIC "/opt/intel/oneapi/mkl/latest/include")
#    target_link_directories(${TargetName} PUBLIC "/opt/intel/oneapi/mkl/latest/lib")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_include_directories(${TargetName} PUBLIC "C:\\Program Files (x86)\\Intel\\oneAPI\\mkl\\latest\\include")
    target_link_directories(${TargetName} PUBLIC
            "C:\\Program Files (x86)\\Intel\\oneAPI\\mkl\\latest\\lib\\intel64"
            "C:\\Program Files (x86)\\Intel\\oneAPI\\mkl\\latest\\lib")
endif()

set(COEFFICIENT_RESOURCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Resources")

target_compile_definitions("${TargetName}" PUBLIC
        "COEFFICIENT_RESOURCE_PATH=\"${COEFFICIENT_RESOURCE_PATH}\"")

target_link_libraries("${TargetName}"
        PRIVATE
        SpleeterRTPlug-Assets
        ${JUCE_DEPENDENCIES}
        PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
        )

if (CMAKE_SYSTEM_NAME STREQUAL "Linux") #TODO: OpenBLAS and MKL should be options
    target_link_libraries(${TargetName} PUBLIC
            openblas
            #Linking order matters! Alphabetic often will not work here.
            mkl_intel_lp64 #Depends on mkl_core
            mkl_core
            mkl_sequential)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(${TargetName} PUBLIC
            userenv
            mkl_intel_lp64 #Depends on mkl_core
            mkl_core
            mkl_sequential)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_link_libraries(${TargetName} PUBLIC
            "-framework Accelerate")
endif()

# BUILD_TESTING is ON by default
# include(CTest) does this too, but adds tons of targets we don't want
#enable_testing()

# All test .cpp files should be listed here
#set(TestFiles
#        test/CatchMain.cpp
#        test/PluginBasics.cpp)

# Download the tagged version of Catch2
#Include(FetchContent)
#FetchContent_Declare(
#        Catch2
#        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
#        GIT_TAG        v2.13.3)
#FetchContent_MakeAvailable(Catch2)

# Setup the test executable, again C++ 20
#add_executable(Tests ${TestFiles})
#target_compile_features(Tests PRIVATE cxx_std_20)

# Pull in our plugin code for tests
#target_include_directories(Tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Source)
#target_link_libraries(Tests PRIVATE Catch2::Catch2 "${NFTargetName}" ${JUCE_DEPENDENCIES})

# Make an Xcode Scheme for the test executable so we can run tests in the IDE
#set_target_properties(Tests PROPERTIES XCODE_GENERATE_SCHEME ON
#        MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/test/Info.plist)

# Organize the test source in the Tests/ folder in the IDE
#source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/test PREFIX "" FILES ${TestFiles})

# Load and use the .cmake file provided by Catch2
# https://github.com/catchorg/Catch2/blob/devel/docs/cmake-integration.md#parseandaddcatchtestscmake
# We have to manually provide the source directory here for now
# https://github.com/catchorg/Catch2/issues/2026
#include(${Catch2_SOURCE_DIR}/contrib/Catch.cmake)
#catch_discover_tests(Tests)
