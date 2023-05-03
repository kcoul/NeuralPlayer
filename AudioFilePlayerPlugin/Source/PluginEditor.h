/*
    PluginEditor.h
    Copyright (C) 2017 Jonathon Racz, ROLI Ltd.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <JuceHeader.h>
#include "AudioThumbnailComp.h"
#include "PluginProcessor.h"
#include <roli_blocks_basics/roli_blocks_basics.h>

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

    std::function<void()> relayAllNotesOff = [this]()
    {
        processor.sendAllNotesOff();
        keyboardState.allNotesOff(1);
    };

    juce::ToggleButton lumiDetectedButton {"Lumi Detected"};
    std::unique_ptr<juce::TextButton> buttonLoadMIDIFile;
    std::unique_ptr<AudioThumbnailComp> thumbnail;
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioFilePlayerEditor)
};
