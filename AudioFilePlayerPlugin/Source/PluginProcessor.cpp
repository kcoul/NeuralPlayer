#include "PluginProcessor.h"
#include "PluginEditor.h"

AudioFilePlayerProcessor::AudioFilePlayerProcessor() :
    AudioProcessor(BusesProperties().withOutput("Output", AudioChannelSet::stereo())),
    thumbnailCache(1),
    readAheadThread("transport read ahead")
{
    formatManager.registerBasicFormats();
    readAheadThread.startThread();
    numTracks.store(0);
    
    for (auto i = 0; i <= 127; i++)
    {
        allNoteOffMessages.addEvent(juce::MidiMessage::noteOff(1, i), 0);
    }
}

AudioFilePlayerProcessor::~AudioFilePlayerProcessor()
{
    transportSource.setSource(nullptr);
}

const String AudioFilePlayerProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioFilePlayerProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AudioFilePlayerProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

double AudioFilePlayerProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioFilePlayerProcessor::getNumPrograms()
{
    return 1;
}

int AudioFilePlayerProcessor::getCurrentProgram()
{
    return 0;
}

void AudioFilePlayerProcessor::setCurrentProgram(int index)
{
}

const String AudioFilePlayerProcessor::getProgramName(int index)
{
    return String();
}

void AudioFilePlayerProcessor::changeProgramName(int index, const String& newName)
{
}

void AudioFilePlayerProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    transportSource.prepareToPlay(samplesPerBlock, sampleRate);
    numSamplesPerBuffer = samplesPerBlock;
}

void AudioFilePlayerProcessor::releaseResources()
{
    transportSource.releaseResources();
}

void AudioFilePlayerProcessor::loadMIDIFile(const File& file)
{
    MIDIFile.clear();
    
    juce::FileInputStream stream(file);
    MIDIFile.readFrom(stream);

    /** This function call means that the MIDI file is going to be played with the
        original tempo and signature.
    */
    MIDIFile.convertTimestampTicksToSeconds();
    
    numTracks.store(MIDIFile.getNumTracks());
    currentTrack.store(0);
    trackHasChanged = false;
}

void AudioFilePlayerProcessor::lumiMIDIEvent(const void* message, size_t size)
{
    if (lumi)
        lumi->sendMessage(message, size);
}

void AudioFilePlayerProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    latestMIDIBuffer.clear();

    if (pendingMIDIFlush)
        sendAllNotesOffToMIDIOut(midiMessages);

    juce::ScopedNoDenormals noDenormals;
    
    for(int i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    transportSource.getNextAudioBlock(AudioSourceChannelInfo(buffer));

    if (getTotalNumOutputChannels() >= 2)
    {
        if (hearExtractedVocal.load())
            buffer.copyFrom(0, 0, buffer.getReadPointer(1), buffer.getNumSamples());
        else
            buffer.copyFrom(1, 0, buffer.getReadPointer(0), buffer.getNumSamples());
    }

    const juce::ScopedTryLock myScopedLock(processLock);
    if (myScopedLock.isLocked())
    {
        if (numTracks.load() > 0 && transportSource.isPlaying())
        {
            const juce::MidiMessageSequence* midiSequence = MIDIFile.getTrack(currentTrack.load());

            auto startTime = transportSource.getCurrentPosition();
            auto endTime = startTime + buffer.getNumSamples() / getSampleRate();

            // If we've reached the end of the sequence
            if (isPlayingSomething && startTime >= midiSequence->getEndTime())
            {
                sendAllNotesOff();
            }
            else
            {
                // Called when the user changes the track during playback
                if (trackHasChanged)
                {
                    trackHasChanged = false;
                    sendAllNotesOff();
                }
                                
                // Iterating through the MIDI file contents and trying to find an event that
                // needs to be called in the current time frame
                for (auto i = 0; i < midiSequence->getNumEvents(); i++)
                {
                    juce::MidiMessageSequence::MidiEventHolder* event = midiSequence->getEventPointer(i);

                    if (event->message.getTimeStamp() >= startTime && event->message.getTimeStamp() < endTime)
                    {
                        //auto ts = event->message.getTimeStamp();
                        if (event->message.isNoteOn() || event->message.isNoteOff())
                        {
                            //auto noteNumber = event->message.getNoteNumber();
                            auto samplePosition = juce::roundToInt((event->message.getTimeStamp() - startTime) * getSampleRate());
                            midiMessages.addEvent(event->message, samplePosition);
                            latestMIDIBuffer.addEvent(event->message, samplePosition);

                            lumiMIDIEvent((void*)event->message.getRawData(), event->message.getRawDataSize());
                            
                            isPlayingSomething = true;
                        }
                    }
                }
                newMIDIBufferAvailable = true;
            }
        }
        else
        {
            // If the transport isn't active anymore
            if (isPlayingSomething)
            {
                sendAllNotesOff();
            }
        }
    }
    else
    {
        // If we have just opened a MIDI file with no content
        if (isPlayingSomething)
        {
            sendAllNotesOff();
        }
    }
}

void AudioFilePlayerProcessor::sendAllNotesOff()
{
    //Send to Lumi for 'stuck note' situations, doesn't seem to respond to MidiMessage::allNotesOff however
    for(auto msg : allNoteOffMessages)
        lumiMIDIEvent(msg.data, msg.numBytes);
    
    isPlayingSomething = false;
    pendingMIDIFlush = true;
}

void AudioFilePlayerProcessor::sendAllNotesOffToMIDIOut(MidiBuffer& midiMessages)
{
    for (auto i = 1; i <= 16; i++)
    {
        midiMessages.addEvent(MidiMessage::allNotesOff(i), 0);
    }

    pendingMIDIFlush = false;
}

bool AudioFilePlayerProcessor::hasEditor() const
{
    return true;
}

AudioProcessorEditor* AudioFilePlayerProcessor::createEditor()
{
    return new AudioFilePlayerEditor(*this);
}

void AudioFilePlayerProcessor::getStateInformation(MemoryBlock& destData)
{
    XmlElement xml("plugin-settings");

    xml.setAttribute("audiofile", currentlyLoadedFile.getFullPathName());

    copyXmlToBinary(xml, destData);
}

void AudioFilePlayerProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState != nullptr)
    {
        if (xmlState->hasTagName("plugin-settings"))
        {
            currentlyLoadedFile = File::createFileWithoutCheckingPath(xmlState->getStringAttribute("audiofile"));
            if (currentlyLoadedFile.existsAsFile())
            {
                loadAudioFileIntoTransport(currentlyLoadedFile);
            }
        }
    }
}

AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioFilePlayerProcessor();
}

void AudioFilePlayerProcessor::loadAudioFileIntoTransport(const File& audioFile)
{
    // unload the previous file source and delete it..
    transportSource.stop();
    transportSource.setSource(nullptr);
    currentAudioFileSource = nullptr;

    AudioFormatReader* reader = formatManager.createReaderFor(audioFile);
    currentlyLoadedFile = audioFile;

    if (reader != nullptr)
    {
        currentAudioFileSource = std::make_unique<AudioFormatReaderSource>(reader, true);

        // ..and plug it into our transport source
        transportSource.setSource(
            currentAudioFileSource.get(),
            32768,                   // tells it to buffer this many samples ahead
            &readAheadThread,        // this is the background thread to use for reading-ahead
            reader->sampleRate);     // allows for sample rate correction
    }
}
