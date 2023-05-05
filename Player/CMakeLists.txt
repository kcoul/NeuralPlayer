project (Player VERSION 0.0.1)
set(TargetName ${PROJECT_NAME})

juce_add_gui_app(${TargetName} PRODUCT_NAME "AudioMIDIPlayer" VERSION 0.1)
target_compile_features(${TargetName} PUBLIC cxx_std_17)

juce_generate_juce_header(${TargetName})

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake)
find_package(RTNeural)
find_package(OnnxRuntime)

add_library(onnxruntime SHARED IMPORTED)

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    if(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo") #libonnxruntime can't seem to build in this configuration, ironically
        message("Using alternative libonnxruntime.dylib path")
        set_property(TARGET onnxruntime PROPERTY IMPORTED_LOCATION
                ${OnnxRuntime_SOURCE_DIR}/../../../cmake-build-release/_deps/onnxruntime-src/build/MacOS/RelWithDebInfo/libonnxruntime.dylib)
    else()
        message("Using in-tree libonnxruntime.dylib path")
        set_property(TARGET onnxruntime PROPERTY IMPORTED_LOCATION
                ${OnnxRuntime_SOURCE_DIR}/build/MacOS/RelWithDebInfo/libonnxruntime.dylib)
    endif()
endif()

#Binary data
file(GLOB RESOURCES_FILES
        ${CMAKE_CURRENT_LIST_DIR}/Lib/ModelData/*.json
        ${CMAKE_CURRENT_LIST_DIR}/Lib/ModelData/*.ort)

juce_add_binary_data(Player-BasicPitchCNN-Assets SOURCES ${RESOURCES_FILES})

add_library(Player-BasicPitchCNN STATIC ${CMAKE_CURRENT_LIST_DIR}/Lib/Model/BasicPitchCNN.cpp)
target_compile_features(Player-BasicPitchCNN PUBLIC cxx_std_17)
target_include_directories(Player-BasicPitchCNN PRIVATE "${RTNeural_SOURCE_DIR}")
target_link_libraries(Player-BasicPitchCNN PUBLIC RTNeural Player-BasicPitchCNN-Assets)

target_include_directories(${TargetName} PRIVATE
        ${OnnxRuntime_SOURCE_DIR}/include/onnxruntime/core/session
        ${CMAKE_CURRENT_LIST_DIR}/Lib/DSP
        ${CMAKE_CURRENT_LIST_DIR}/Lib/MidiFile
        ${CMAKE_CURRENT_LIST_DIR}/Lib/MidiPostProcessing
        ${CMAKE_CURRENT_LIST_DIR}/Lib/Model
        ${CMAKE_CURRENT_LIST_DIR}/Lib/Utils
        )

target_sources(${TargetName} PRIVATE
        Lib/DSP/DownSampler.cpp
        Lib/MidiFile/MidiFileWriter.cpp
        Lib/MidiPostProcessing/NoteOptions.cpp
        Lib/MidiPostProcessing/RhythmOptions.cpp
        Lib/Model/BasicPitch.cpp
        Lib/Model/Features.cpp
        Lib/Model/Notes.cpp
        Source/AudioThumbnailComponent.cpp
        Source/ConsoleViewComponent.cpp
        Source/Main.cpp
        Source/MainViewComponent.cpp
        Source/Player.cpp
        Source/PlayerComponent.cpp
        Source/SettingsComponent.cpp
        Source/SharedAudioDeviceManager.cpp)

if (CMAKE_SYSTEM_NAME MATCHES "^(Darwin|Linux|Windows)")
    #enables chooser->browseForFileToOpen()
    target_compile_definitions(${TargetName} PRIVATE JUCE_MODAL_LOOPS_PERMITTED=1)
elseif (CMAKE_SYSTEM_NAME MATCHES "^(iOS|Android)")
    target_compile_definitions(${TargetName} PRIVATE JUCE_CONTENT_SHARING=1)
endif()

target_compile_definitions(${TargetName} PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:${TargetName},JUCE_PROJECT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:${TargetName},JUCE_VERSION>"
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_BUNDLE_ID="com.kcoul.AudioMIDIPlayer"
        )

if (CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set_xcode_property(${TargetName} PRODUCT_BUNDLE_IDENTIFIER "com.kcoul.AudioMIDIPlayer" All)
endif()

target_link_libraries(${TargetName}
        PRIVATE
        Player-BasicPitchCNN
        onnxruntime
        #Player-Assets #TODO: Add GUI resources separately
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_dsp
        roli_blocks_basics
        PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
        )