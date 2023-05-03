project (Player VERSION 0.0.1)
set(TargetName ${PROJECT_NAME})

juce_add_gui_app(${TargetName} PRODUCT_NAME "AudioMIDIPlayer" VERSION 0.1)
juce_generate_juce_header(${TargetName})

target_compile_features(${TargetName} PUBLIC cxx_std_17) #std::filesystem

target_sources(${TargetName} PRIVATE
        Source/ConsoleViewComponent.cpp
        Source/Main.cpp
        Source/MainViewComponent.cpp
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

target_link_libraries(${TargetName} PRIVATE
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_dsp
        )