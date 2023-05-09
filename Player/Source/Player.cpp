#include "Player.h"

Player::Player(std::function<void(juce::MidiBuffer)>& latestMIDIBufferFn,
               std::function<void(double)>& latestPlaybackLocation) :
latestMIDIBufferCallback(latestMIDIBufferFn), latestPlaybackLocationFn(latestPlaybackLocation)
{
    for (auto i = 0; i <= 127; i++)
    {
        allNoteOffMessages.addEvent(juce::MidiMessage::noteOff(1, i), 0);
    }

    audioDeviceManager.addAudioCallback(this);
}

Player::~Player() noexcept
{
    audioDeviceManager.removeAudioCallback(this);
}

void Player::audioDeviceAboutToStart(juce::AudioIODevice* device)
{
    sampleRate = device->getCurrentSampleRate();
    numSamplesPerBuffer = device->getCurrentBufferSizeSamples();
    transportSource.prepareToPlay(numSamplesPerBuffer, sampleRate);

    outDevice = MidiOutput::openDevice (audioDeviceManager.getDefaultMidiOutputIdentifier());
}

void Player::audioDeviceStopped()
{

}

void Player::audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                                       int numInputChannels,
                                                       float* const* outputChannelData,
                                                       int numOutputChannels,
                                                       int numSamples,
                                                       const AudioIODeviceCallbackContext& context)
{
    juce::ScopedNoDenormals noDenormals;
    latestMIDIBuffer.clear();

    if (transportSource.isPlaying())
    {
        AudioSampleBuffer buffer;
        buffer.setDataToReferTo(outputChannelData, numOutputChannels, numSamples);
        transportSource.getNextAudioBlock(AudioSourceChannelInfo(buffer));

        const juce::MidiMessageSequence *midiSequence = MIDIFile.getTrack(0);

        auto startTime = transportSource.getCurrentPosition();
        auto endTime = startTime + buffer.getNumSamples() / sampleRate;

        // If we've reached the end of the sequence
        if (startTime >= midiSequence->getEndTime())
        {
            sendAllNotesOff();
        }
        else
        {
            // Iterate through the MIDI file contents and trying to find an event that
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
                        auto samplePosition = juce::roundToInt((event->message.getTimeStamp() - startTime) * sampleRate);

                        latestMIDIBuffer.addEvent(event->message, samplePosition);
                        lumiMIDIEvent((void*)event->message.getRawData(), event->message.getRawDataSize());

                        if (outDevice)
                        {
                            outDevice->sendBlockOfMessagesNow(latestMIDIBuffer);
                        }
                    }
                }
            }
            latestMIDIBufferCallback(latestMIDIBuffer);
        }
        latestPlaybackLocationFn(transportSource.getCurrentPosition());
    }
    else if (transportSource.positionableSource != nullptr && transportSource.hasStreamFinished())
    {
        juce::MessageManager::callAsync([this]
        {
            streamFinishedCallback();
        });
        streamWasPlaying = false;
    }
    else
    {
        for(int i = 0; i < numOutputChannels; ++i)
            FloatVectorOperations::clear(outputChannelData[i], numSamples);
    }
}

void Player::lumiMIDIEvent(const void* message, size_t size)
{
    if (lumi)
        lumi->sendMessage(message, size);
}

void Player::sendAllNotesOff()
{
    //Send to Lumi for 'stuck note' situations, doesn't seem to respond to MidiMessage::allNotesOff however
    for(auto msg : allNoteOffMessages)
        lumiMIDIEvent(msg.data, msg.numBytes);

    latestMIDIBufferCallback(allNoteOffMessages);

    if (outDevice)
    {
        outDevice->sendBlockOfMessagesNow(allNoteOffMessages);
    }
}