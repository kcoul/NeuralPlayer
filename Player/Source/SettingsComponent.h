#pragma once

#include <roli_blocks_basics/roli_blocks_basics.h>

#include "ConsoleViewComponent.h"
#include "SharedAudioDeviceManager.h"
#include "SourceSepMIDIRenderingThread.h"

#define SHOW_XML_WRITE_TEXT_BUTTON 0

class SettingsComponent : public juce::Component,
                          private roli::TopologySource::Listener
{
public:
    SettingsComponent(std::function<void(std::vector<roli::Block::Ptr>)>& lumiCallbackFn);
    ~SettingsComponent() override;
    void paint(juce::Graphics&) override;
    void resized() override;

    void topologyChanged() override;
private:
    std::function<void(std::vector<roli::Block::Ptr>)>& lumiCallback;
    juce::AudioDeviceManager& audioDeviceManager { getSharedAudioDeviceManager(0, 2) };
    std::unique_ptr<juce::AudioDeviceSelectorComponent> audioMIDISettings;

    juce::Label consoleViewLabel {"", "Hardware Debug Console"};
    std::unique_ptr<ConsoleViewComponent> consoleViewComponent;

    // The PhysicalTopologySource member variable which reports BLOCKS changes.
    roli::PhysicalTopologySource pts;

    juce::Label lumiDetectedLabel { "", "Lumi Detected:" };
    juce::ToggleButton lumiDetectedButton {""};
    juce::TextButton xmlWriteTestButton;

    void xmlWriteTest();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsComponent)
};


