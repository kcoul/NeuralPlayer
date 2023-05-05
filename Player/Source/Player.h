#include <JuceHeader.h>

class Player : public juce::AudioIODeviceCallback
{
public:
    Player(std::function<void(juce::MidiBuffer)>& latestMIDIBuffer);

    ~Player();

    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;

    void audioDeviceStopped() override;

    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                          int numInputChannels,
                                          float* const* outputChannelData,
                                          int numOutputChannels,
                                          int numSamples,
                                          const AudioIODeviceCallbackContext& context) override;

    roli::Block::Ptr lumi;
    AudioTransportSource transportSource;
    juce::MidiFile MIDIFile;

    void sendAllNotesOff();

    std::function<void(juce::MidiBuffer)>& latestMIDIBufferCallback;
    juce::MidiBuffer latestMIDIBuffer;

    int numSamplesPerBuffer;
    double sampleRate;
private:
    void lumiMIDIEvent(const void* message, size_t size);
    juce::MidiBuffer allNoteOffMessages;
};


