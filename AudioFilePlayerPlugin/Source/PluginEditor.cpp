#include "PluginProcessor.h"
#include "PluginEditor.h"

AudioFilePlayerEditor::AudioFilePlayerEditor(AudioFilePlayerProcessor& p) :
    AudioProcessorEditor(&p),
    processor(p),
    keyboardComponent (keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    loadedFilenameLabel.setJustificationType(Justification::right);
    addAndMakeVisible(loadedFilenameLabel);

    loadButton = std::make_unique<juce::TextButton>("Load audio file");
    addAndMakeVisible(loadButton.get());
    loadButton->onClick = [this]
    {
        juce::FileChooser fileChooser("Load audio file...",
                                      juce::File::getSpecialLocation(juce::File::userDesktopDirectory),
                                      "*.wav");
        
        if (fileChooser.browseForFileToOpen())
        {
            auto result = fileChooser.getResult();
            if (result.getFileExtension() == ".wav")
            {
                loadFiles(result);
            }
        }
    };

    thumbnail = std::make_unique<AudioThumbnailComponent>(processor.formatManager,
                                                          processor.transportSource,
                                                          processor.thumbnailCache,
                                                          thumbnailPlayheadPositionChanged,
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

    keyboardComponent.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(keyboardComponent);

    setOpaque(true);
    setSize(1225, 800);
    
    // Register to receive topologyChanged() callbacks from pts.
    pts.addListener (this);

    startTimer(20);
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
    auto margin = 4;
    Rectangle<int> r(getLocalBounds().reduced(margin));

    auto heightDivUnit = r.getHeight() / 12; //Divide available height into 12 equal parts to work with

    auto halfWidth = r.getWidth() / 2;

    auto topCell = r.removeFromTop(heightDivUnit);

    lumiDetectedButton.setBounds(topCell.removeFromLeft(halfWidth));
    loadedFilenameLabel.setBounds(topCell);

    loadButton->setBounds(r.removeFromTop(heightDivUnit));

    auto spacer = 6;
    r.removeFromTop(spacer);
    thumbnail->setBounds(r.removeFromTop(heightDivUnit * 6));
    r.removeFromTop(spacer);

    startStopButton.setBounds(r.removeFromTop(heightDivUnit));

    abButton.setBounds(r.removeFromTop(heightDivUnit));

    keyboardComponent.setBounds(r.removeFromBottom(heightDivUnit * 2));
}

void AudioFilePlayerEditor::buttonClicked (Button* buttonThatWasClicked) 
{
    if (buttonThatWasClicked == &startStopButton)
    {
        if (processor.transportSource.isPlaying())
        {
            processor.transportSource.stop();
            processor.sendAllNotesOff();
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
        auto file = thumbnail->getLastDroppedFile();
        loadFiles(file);
    }
}

void AudioFilePlayerEditor::loadFiles(juce::File wavFile)
{
    processor.loadAudioFileIntoTransport(wavFile);
    thumbnail->setFile(wavFile);

    //Try loading a MIDI file of same path/name to MIDI player
    auto possibleMIDIFile = File(wavFile.getFullPathName().substring(0,
                                                                     wavFile.getFullPathName().length() - 4)
                                                                             + ".mid");
    if (possibleMIDIFile.existsAsFile())
        processor.loadMIDIFile(possibleMIDIFile);

    loadedFilenameLabel.setText(wavFile.getFileName(), juce::dontSendNotification);
}

void AudioFilePlayerEditor::timerCallback()
{
    if (processor.transportSource.isPlaying() && processor.newMIDIBufferAvailable)
    {
        keyboardState.processNextMidiBuffer(processor.latestMIDIBuffer,
                                            0,
                                            processor.numSamplesPerBuffer,
                                            false);
        processor.newMIDIBufferAvailable = false;
    }
    if (processor.pendingMIDIFlush)
        keyboardState.allNotesOff(1);
}

void AudioFilePlayerEditor::topologyChanged()
{
    // We have a new topology, so find out what it is and store it in a local
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
