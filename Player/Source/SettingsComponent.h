#pragma once

#include <roli_blocks_basics/roli_blocks_basics.h>

#include "ConsoleViewComponent.h"
#include "DataStructures.h"
#include "SharedAudioDeviceManager.h"
#include "SourceSepMIDIRenderingThread.h"

class SettingsComponent : public juce::Component,
                          private roli::TopologySource::Listener
{
public:
    SettingsComponent(WatchedVars& consoleVars, std::function<void(roli::Block::Ptr)>& lumiCallbackFn);
    ~SettingsComponent() override;
    void paint(juce::Graphics&) override;
    void resized() override;

    void topologyChanged() override;
private:
    std::function<void(roli::Block::Ptr)>& lumiCallback;
    juce::AudioDeviceManager& audioDeviceManager { getSharedAudioDeviceManager(0, 2) };
    std::unique_ptr<ConsoleViewComponent> consoleViewComponent;
    std::unique_ptr<juce::AudioDeviceSelectorComponent> audioMIDISettings;
    //LookAndFeel_V4 settingsLaf;

    // The PhysicalTopologySource member variable which reports BLOCKS changes.
    roli::PhysicalTopologySource pts;

    juce::ToggleButton lumiDetectedButton {"Lumi Detected"};
    juce::TextButton xmlWriteTestButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsComponent)
};


