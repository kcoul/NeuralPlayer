#pragma once

#include <JuceHeader.h>
#include <utility>

#include "PlayerComponent.h"
#include "SettingsComponent.h"

#include "DataStructures.h"

#include "ConsoleViewComponent.h"
#include "SharedAudioDeviceManager.h"
#include "SourceSepMIDIRenderingThread.h"

class MainViewComponent : public juce::Component
{
public:
    MainViewComponent();
    ~MainViewComponent() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    juce::TabbedComponent tabComponent;
    std::unique_ptr<PlayerComponent> playerComponent;
    std::unique_ptr<SettingsComponent> settingsComponent;

    std::unique_ptr<ConsoleViewComponent> consoleViewComponent;
    std::unique_ptr<juce::AudioDeviceSelectorComponent> audioSettings;

    std::unique_ptr<SourceSepMIDIRenderingThread> renderingThread;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainViewComponent)
};
