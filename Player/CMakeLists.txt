project (Player VERSION 0.0.1)
set(TargetName ${PROJECT_NAME})

juce_add_gui_app(${TargetName}
        PRODUCT_NAME "NeuralPlayer"
        VERSION 0.1
        ICON_BIG "${CMAKE_SOURCE_DIR}/Player/Resources/NP3_HiQ.png"
        ICON_SMALL "${CMAKE_SOURCE_DIR}/Player/Resources/NP3_HiQ.png")
target_compile_features(${TargetName} PUBLIC cxx_std_17)

juce_generate_juce_header(${TargetName})

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake)
find_package(RTNeural)
find_package(OnnxRuntime)

add_library(onnxruntime SHARED IMPORTED)

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    #TODO: Compile onnxruntime in Release mode instead and use that unless you need to debug into it
    #NOTE: Looking for libonnxruntime.dylib in the most likely location, change path if your use-case is different
    set_property(TARGET onnxruntime PROPERTY IMPORTED_LOCATION
            ${CMAKE_SOURCE_DIR}/cmake-build-debug/_deps/onnxruntime-src/build/MacOS/RelWithDebInfo/libonnxruntime.dylib)
endif()

#Binary data
file(GLOB RESOURCES_FILES
        ${CMAKE_CURRENT_LIST_DIR}/Lib/ModelData/*.json
        ${CMAKE_CURRENT_LIST_DIR}/Lib/ModelData/*.ort
        ${CMAKE_CURRENT_LIST_DIR}/Resources/three_dots.png)

juce_add_binary_data(Player-Assets SOURCES ${RESOURCES_FILES})

add_library(Player-BasicPitchCNN STATIC ${CMAKE_CURRENT_LIST_DIR}/Lib/Model/BasicPitchCNN.cpp)
target_compile_features(Player-BasicPitchCNN PUBLIC cxx_std_17)
target_include_directories(Player-BasicPitchCNN PRIVATE "${RTNeural_SOURCE_DIR}")
target_link_libraries(Player-BasicPitchCNN PUBLIC RTNeural Player-Assets)

target_include_directories(${TargetName} PRIVATE
        ${CMAKE_SOURCE_DIR}/cmake-build-debug/_deps/onnxruntime-src/include/onnxruntime/core/session
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
        Source/SharedAudioDeviceManager.cpp
        Source/SidePanelHeader.cpp)

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
        JUCE_BUNDLE_ID="com.kcoul.NeuralPlayer"
        )

if (CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set_xcode_property(${TargetName} PRODUCT_BUNDLE_IDENTIFIER "com.kcoul.NeuralPlayer" All)
endif()

target_link_libraries(${TargetName}
        PRIVATE
        Player-BasicPitchCNN
        onnxruntime
        Player-Assets
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_dsp
        roli_blocks_basics
        PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
        )