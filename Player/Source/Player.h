#include <JuceHeader.h>

#include "SharedAudioDeviceManager.h"

class Player : public juce::AudioIODeviceCallback
{
public:
    Player(std::function<void(juce::MidiBuffer)>& latestMIDIBuffer,
           std::function<void(double)>& latestPlaybackLocation);

    ~Player();

    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;

    void audioDeviceStopped() override;

    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                          int numInputChannels,
                                          float* const* outputChannelData,
                                          int numOutputChannels,
                                          int numSamples,
                                          const AudioIODeviceCallbackContext& context) override;

    std::vector<roli::Block::Ptr> lumi;
    AudioTransportSource transportSource;
    juce::MidiFile MIDIFile;
    std::function<void()> streamFinishedCallback;
    void sendAllNotesOff();

    std::function<void(juce::MidiBuffer)>& latestMIDIBufferCallback;
    juce::MidiBuffer latestMIDIBuffer;
    std::function<void(double)>& latestPlaybackLocationFn;

    int numSamplesPerBuffer;
    double sampleRate;
private:
    juce::AudioDeviceManager& audioDeviceManager { getSharedAudioDeviceManager(0, 2) };

    void lumiMIDIEvent(const void* message, size_t size);
    juce::MidiBuffer allNoteOffMessages;

    MidiDeviceInfo deviceInfo;
    std::unique_ptr<MidiOutput> outDevice;

    bool wasJustPlaying = false;
};


