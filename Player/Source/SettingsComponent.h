#pragma once

#include "ConsoleViewComponent.h"
#include "DataStructures.h"
#include "SharedAudioDeviceManager.h"
#include "SourceSepMIDIRenderingThread.h"

class SettingsComponent : public juce::Component
{
public:
    SettingsComponent(WatchedVars& consoleVars);
    ~SettingsComponent() override;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    juce::AudioDeviceManager& audioDeviceManager { getSharedAudioDeviceManager(0, 2) };
    std::unique_ptr<ConsoleViewComponent> consoleViewComponent;
    std::unique_ptr<juce::AudioDeviceSelectorComponent> audioSettings;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsComponent)
};


