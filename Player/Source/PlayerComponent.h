#pragma once

#include <JuceHeader.h>

#include "AudioThumbnailComp.h"
#include "PlaylistComponent.h"
#include "SharedAudioDeviceManager.h"
#include "SourceSepMIDIRenderingThread.h"

enum class FolderSelectResult
{
    ok,
    cancelled
};

class PlayerComponent : public juce::Component,
                        public juce::AudioIODeviceCallback
{
public:
    PlayerComponent(std::unique_ptr<SourceSepMIDIRenderingThread>& renderingThread);
    ~PlayerComponent() override;
    void paint(juce::Graphics&) override;
    void resized() override;

    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;

    void audioDeviceStopped() override;

    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                               int numInputChannels,
                               float* const* outputChannelData,
                               int numOutputChannels,
                               int numSamples,
                               const AudioIODeviceCallbackContext& context) override;
private:
    juce::AudioDeviceManager& audioDeviceManager { getSharedAudioDeviceManager(0, 2) };

    juce::TextButton loadButton {"Load Audio"};

    std::unique_ptr<juce::FileChooser> loadChooser;
    std::pair<FolderSelectResult, juce::File> selectInputFolder();
    juce::File selectedOutputFolder;

    std::unique_ptr<SourceSepMIDIRenderingThread>& renderingThread;

    PlaylistComponent playlistComponent;
    std::function<void(String)> trackSelected;
    std::unique_ptr<AudioFormatReaderSource> currentAudioFileSource;
    void loadAudioFileIntoTransport(const File& audioFile);

    std::unique_ptr<AudioThumbnailComp> thumbnail;
    AudioTransportSource transportSource;
    AudioFormatManager formatManager;
    AudioThumbnailCache thumbnailCache;
    std::function<void()> thumbnailPlayheadPositionChanged = [this]()
    {
        //processor.sendAllNotesOff();
    };
    File currentlyLoadedFile;
    TimeSliceThread readAheadThread;

    juce::MidiFile MIDIFile;

    juce::TextButton startStopButton;

    juce::MidiKeyboardState keyboardState;
    juce::MidiKeyboardComponent keyboardComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlayerComponent)
};

