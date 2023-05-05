#pragma once

#include <JuceHeader.h>

class AudioFilePlayerProcessor :
    public AudioProcessor
{
public:
    AudioFilePlayerProcessor();
    ~AudioFilePlayerProcessor();

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    void processBlock(AudioSampleBuffer&, MidiBuffer&) override;

    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const String getProgramName(int index) override;
    void changeProgramName(int index, const String& newName) override;

    void getStateInformation(MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    AudioTransportSource transportSource;
    AudioFormatManager formatManager;
    File currentlyLoadedFile;
    AudioThumbnailCache thumbnailCache;

    void loadAudioFileIntoTransport(const File& audioFile);
    void loadMIDIFile(const File& midiFile);
    
    juce::CriticalSection processLock;
    
    juce::MidiFile MIDIFile;
    bool isPlayingSomething;
    bool trackHasChanged = false;

    roli::Block::Ptr lumi;
    void sendAllNotesOff();
    juce::MidiBuffer latestMIDIBuffer;

    std::atomic<int> currentTrack;
    std::atomic<int> numTracks;
    std::atomic<int> numSamplesPerBuffer;

    std::atomic<bool> hearExtractedVocal = false;
    std::atomic<bool> newMIDIBufferAvailable = false;
    std::atomic<bool> pendingMIDIFlush = false;
private:
    juce::MidiBuffer allNoteOffMessages;
    std::unique_ptr<AudioFormatReaderSource> currentAudioFileSource;
    TimeSliceThread readAheadThread;

    void lumiMIDIEvent(const void* message, size_t size);
    void sendAllNotesOffToMIDIOut(MidiBuffer& buffer);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioFilePlayerProcessor)
};
