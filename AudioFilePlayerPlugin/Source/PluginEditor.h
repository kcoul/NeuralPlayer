#pragma once

#include <JuceHeader.h>
#include <roli_blocks_basics/roli_blocks_basics.h>

#include "AudioThumbnailComponent.h"
#include "PluginProcessor.h"

class AudioFilePlayerEditor :
    public AudioProcessorEditor,
    private Button::Listener,
    private ChangeListener,
    private roli::TopologySource::Listener,
    private juce::Timer
{
public:
    AudioFilePlayerEditor(AudioFilePlayerProcessor&);
    ~AudioFilePlayerEditor();

    void paint(Graphics&) override;
    void resized() override;
private:
    AudioFilePlayerProcessor& processor;

    std::function<void()> thumbnailPlayheadPositionChanged = [this]()
    {
        processor.sendAllNotesOff();
    };

    juce::ToggleButton lumiDetectedButton {"Lumi Detected"};
    std::unique_ptr<juce::TextButton> loadButton;
    std::unique_ptr<AudioThumbnailComponent> thumbnail;
    juce::ToggleButton abButton {"Hear Extracted Vocal"};

    TextButton startStopButton;

    juce::MidiKeyboardState keyboardState;
    juce::MidiKeyboardComponent keyboardComponent;

    void buttonClicked(Button* buttonThatWasClicked) override;

    void changeListenerCallback(ChangeBroadcaster* source) override;

    // Called by the PhysicalTopologySource when the BLOCKS topology changes.
    void topologyChanged() override;

    // The PhysicalTopologySource member variable which reports BLOCKS changes.
    roli::PhysicalTopologySource pts;

    void timerCallback() override;

    void loadFiles(File wavFile);
    juce::Label loadedFilenameLabel {"", ""};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioFilePlayerEditor)
};
