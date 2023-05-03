/*
    PluginEditor.cpp
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

#include "PluginProcessor.h"
#include "PluginEditor.h"

AudioFilePlayerEditor::AudioFilePlayerEditor(AudioFilePlayerProcessor& p) :
    AudioProcessorEditor(&p),
    processor(p)
{
    buttonLoadMIDIFile = std::make_unique<juce::TextButton>("Load a MIDI file");
    addAndMakeVisible(buttonLoadMIDIFile.get());
    buttonLoadMIDIFile->onClick = [this]
    {
        juce::FileChooser fileChooser("Find a MIDI file", juce::File(), "*.mid");
        
        if (fileChooser.browseForFileToOpen())
        {
            processor.loadMIDIFile(fileChooser.getResult());
        }
    };

    thumbnail = std::make_unique<AudioThumbnailComp>(processor.formatManager,
                                                     processor.transportSource,
                                                     processor.thumbnailCache,
                                                     relayAllNotesOff,
                                                     processor.currentlyLoadedFile);
    addAndMakeVisible(thumbnail.get());
    thumbnail->addChangeListener(this);

    addAndMakeVisible(startStopButton);
    startStopButton.setButtonText("Play/Stop");
    startStopButton.addListener(this);
    startStopButton.setColour(TextButton::buttonColourId, Colour(0xff79ed7f));

    abButton.onClick = [this] { processor.hearExtractedVocal = abButton.getToggleState(); };
    addAndMakeVisible(abButton);

    lumiDetectedButton.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(lumiDetectedButton);

    setOpaque(true);

    setSize(512, 334);
    
    // Register to receive topologyChanged() callbacks from pts.
    pts.addListener (this);
}

AudioFilePlayerEditor::~AudioFilePlayerEditor()
{
    thumbnail->removeChangeListener(this);
    pts.removeListener(this);
}

void AudioFilePlayerEditor::paint(Graphics& g)
{
    g.fillAll(Colours::grey);
}

void AudioFilePlayerEditor::resized()
{
    Rectangle<int> r(getLocalBounds().reduced(4));

    abButton.setBounds(r.removeFromBottom(32));

    startStopButton.setBounds(r.removeFromBottom(32));

    r.removeFromBottom(6);
    thumbnail->setBounds(r.removeFromBottom(180));
    r.removeFromBottom(6);
    
    buttonLoadMIDIFile->setBounds(r.removeFromBottom(32));

    lumiDetectedButton.setBounds(r.removeFromBottom(32));
}

void AudioFilePlayerEditor::buttonClicked (Button* buttonThatWasClicked) 
{
    if (buttonThatWasClicked == &startStopButton)
    {
        if (processor.transportSource.isPlaying())
        {
            processor.transportSource.stop();
            processor.sendAllNotesOffToLumi();
        }
        else
        {
            processor.transportSource.setPosition(0);
            processor.transportSource.start();
        }
    }
}

void AudioFilePlayerEditor::changeListenerCallback(ChangeBroadcaster* source) 
{
    if (source == thumbnail.get())
    {
        processor.loadAudioFileIntoTransport(thumbnail->getLastDroppedFile());
        thumbnail->setFile(thumbnail->getLastDroppedFile());
    }
}

void AudioFilePlayerEditor::topologyChanged()
{
    // We have a new topology, so find out what it isand store it in a local
    // variable.
    auto currentTopology = pts.getCurrentTopology();
    Logger::writeToLog ("\nNew BLOCKS topology.");

    // The blocks member of a BlockTopology contains an array of blocks. Here we
    // loop over them and print some information.
    Logger::writeToLog ("Detected " + String (currentTopology.blocks.size()) + " blocks:");

    for (auto& block : currentTopology.blocks)
    {
        Logger::writeToLog ("");
        Logger::writeToLog ("    Description:   " + block->getDeviceDescription());
        Logger::writeToLog ("    Battery level: " + String (block->getBatteryLevel()));
        Logger::writeToLog ("    UID:           " + String (block->uid));
        Logger::writeToLog ("    Serial number: " + block->serialNumber);
        Logger::writeToLog ("    TypeID:        " + String (block->getType()));
        switch (block->getType())
        {
            case roli::Block::unknown:
                Logger::writeToLog ("    Type:          unknown");
                break;
            case roli::Block::lightPadBlock:
                Logger::writeToLog ("    Type:          lightPadBlock");
                break;
            case roli::Block::liveBlock:
                Logger::writeToLog ("    Type:          liveBlock");
                break;
            case roli::Block::loopBlock:
                Logger::writeToLog ("    Type:          loopBlock");
                break;
            case roli::Block::developerControlBlock:
                Logger::writeToLog ("    Type:          developerControlBlock");
                break;
            case roli::Block::touchBlock:
                Logger::writeToLog ("    Type:          touchBlock");
                break;
            case roli::Block::seaboardBlock:
                Logger::writeToLog ("    Type:          seaboardBlock");
                break;
            case roli::Block::lumiKeysBlock:
                Logger::writeToLog ("    Type:          lumiKeysBlock");
                processor.lumi = block;
                lumiDetectedButton.setToggleState(true, juce::dontSendNotification);
                break;
        };
    }
}
