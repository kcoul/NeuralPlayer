#pragma once

#include <JuceHeader.h>
#include <roli_blocks_basics/roli_blocks_basics.h>

#include "AudioThumbnailComponent.h"
#include "Player.h"
#include "PlaylistComponent.h"
#include "SourceSepMIDIRenderingThread.h"

enum class FolderSelectResult
{
    ok,
    cancelled
};

class PlayerComponent : public juce::Component
{
public:
    PlayerComponent(std::unique_ptr<SourceSepMIDIRenderingThread>& renderingThread);
    ~PlayerComponent() override;
    void paint(juce::Graphics&) override;
    void resized() override;

    void setLumi(roli::Block::Ptr lumi) { audioMIDIPlayer.lumi = lumi; };
private:
    Player audioMIDIPlayer;

    juce::TextButton loadButton {"Load Audio"};

    std::unique_ptr<juce::FileChooser> loadChooser;
    std::pair<FolderSelectResult, juce::File> selectInputFolder();
    juce::File selectedOutputFolder;

    std::unique_ptr<SourceSepMIDIRenderingThread>& renderingThread;

    void loadAudioFileIntoTransport(const File& audioFile);
    void loadMIDIFile(const File& midiFile);

    PlaylistComponent playlistComponent;
    std::function<void(String)> trackSelected;

    std::function<void(juce::MidiBuffer)> latestMIDIBufferFn;

    std::unique_ptr<AudioThumbnailComponent> thumbnail;

    std::unique_ptr<AudioFormatReaderSource> currentAudioFileSource;
    AudioFormatManager formatManager;
    AudioThumbnailCache thumbnailCache;

    std::function<void()> thumbnailPlayheadPositionChanged = [this]()
    {
        threadPool.addJob(midiFlushJob);
        startStopButton.setButtonText("Stop");
    };

    juce::ThreadPool threadPool;
    std::function<void()> midiFlushJob;
    std::function<void()> transportStartJob;
    std::function<void()> transportStopJob;

    File currentlyLoadedFile;
    TimeSliceThread readAheadThread;

    juce::TextButton startStopButton;

    juce::MidiKeyboardState keyboardState;
    juce::MidiKeyboardComponent keyboardComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlayerComponent)
};

