#pragma once

#include <JuceHeader.h>

#include <utility>

#include "ConsoleViewComponent.h"
#include "SharedAudioDeviceManager.h"

enum class FolderSelectResult
{
    ok,
    cancelled
};

class MainViewComponent : public juce::Component
{
public:
    MainViewComponent();
    ~MainViewComponent() override;
    void paint(juce::Graphics&) override {}
    void resized() override;

private:
    std::unique_ptr<ConsoleViewComponent> consoleViewComponent;
    std::unique_ptr<juce::AudioDeviceSelectorComponent> audioSettings;

    juce::AudioDeviceManager& audioDeviceManager { getSharedAudioDeviceManager(0, 1) };

    juce::TextButton loadButton {"Load Audio"};

    std::unique_ptr<juce::FileChooser> loadChooser;

    std::pair<FolderSelectResult, juce::File> selectInputFolder();
    juce::File selectedOutputFolder;

    std::unique_ptr<juce::Array<juce::URL>> filesToShare;


};
