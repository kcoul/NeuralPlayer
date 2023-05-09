#pragma once

#include <JuceHeader.h>
#include <roli_blocks_basics/roli_blocks_basics.h>

#include "AudioThumbnailComponent.h"
#include "ConsoleViewComponent.h"
#include "Converters.h"
#include "Player.h"
#include "PlaylistComponent.h"
#include "SidePanelHeader.h"
#include "SourceSepMIDIRenderingThread.h"
#include "VectorPathData.h"

enum class FolderSelectResult
{
    ok,
    cancelled
};

class PlayerComponent : public juce::Component,
                        public juce::Thread::Listener,
                        public juce::Timer
{
public:
    PlayerComponent();
    ~PlayerComponent() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    void exitSignalSent() override;
    void timerCallback() override;
    void setLumi(roli::Block::Ptr lumi) { neuralPlayer.lumi = lumi; };

    std::unique_ptr<SourceSepMIDIRenderingThread> renderingThread;
    double renderingProgress = 0.0;
private:
    Player neuralPlayer;

    juce::TextButton loadNewFolderButton {"Load New"};
    juce::TextButton haltButton { "Halt" };
    juce::TextButton loadExistingPlaylistButton {"Load Existing"};

    std::unique_ptr<juce::FileChooser> loadChooser;
    std::pair<FolderSelectResult, juce::File> selectNewInputFolder();
    std::pair<FolderSelectResult, juce::File> selectExistingPlaylistFile();
    juce::File debugResourcesDirectory;
    static juce::File walkDebugDirectoryToResourcesFolder();
    juce::File currentPlaylistDirectory;

    void loadAudioFileIntoTransport(const File& audioFile);
    void loadMIDIFile(const File& midiFile);

    ProgressBar progressBar;
    juce::ShapeButton openSidePanelButton { "Settings",
                                            juce::Colours::transparentBlack,
                                            juce::Colours::transparentBlack,
                                            juce::Colours::transparentBlack };
    juce::Label playbackProgressLabel {"playBackProgress", "00:00"};
    SidePanelHeader sidePanelHeader;
    ConsoleViewComponent softwareConsoleComponent;
    String lastDisplayedString = "";
    juce::SidePanel softwareConsoleComponentPanel;

    PlaylistComponent playlistComponent;
    std::function<void(String)> trackSelected;

    std::function<void(juce::MidiBuffer)> latestMIDIBufferFn;
    std::function<void(double)> latestPlaybackLocationFn;

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

    double processingIndex = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlayerComponent)
};

