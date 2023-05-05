#include "PlayerComponent.h"

PlayerComponent::PlayerComponent(std::unique_ptr<SourceSepMIDIRenderingThread>& thread) :
renderingThread(thread),
keyboardComponent (keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard),
thumbnailCache(1),
readAheadThread("transport read ahead")
{
    formatManager.registerBasicFormats();
    readAheadThread.startThread();
    
    loadButton.onClick = [this]
    {
        auto result = selectInputFolder();
        if (result.first == FolderSelectResult::ok)
        {
            auto inputFolder = result.second;
            renderingThread->setInputFolder(inputFolder);
            renderingThread->setOutputFolder(File(inputFolder.getFullPathName() + "/out"));
            renderingThread->startThread();
        };
    };
    addAndMakeVisible(loadButton);

    addAndMakeVisible(playlistComponent);

    thumbnail = std::make_unique<AudioThumbnailComp>(formatManager,
                                                     transportSource,
                                                     thumbnailCache,
                                                     thumbnailPlayheadPositionChanged,
                                                     currentlyLoadedFile);
    addAndMakeVisible(thumbnail.get());

    addAndMakeVisible(startStopButton);
    startStopButton.setButtonText("Play");
    startStopButton.onClick = [this]
    {
        if (startStopButton.getToggleState())
            startStopButton.setButtonText("Stop");
        else
            startStopButton.setButtonText("Play");
    };
    addAndMakeVisible(startStopButton);

    addAndMakeVisible(keyboardComponent);
}

PlayerComponent::~PlayerComponent()
{

}

void PlayerComponent::paint(juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void PlayerComponent::resized()
{
    auto area = getLocalBounds().reduced(4);
#if JUCE_IOS
    area.removeFromTop(30);
#endif
    auto vUnit = area.getHeight()/12;
    loadButton.setBounds(area.removeFromTop(vUnit));

    area.removeFromTop(4);

    playlistComponent.setBounds(area.removeFromTop(vUnit * 6));

    area.removeFromTop(4);

    thumbnail->setBounds(area.removeFromTop(vUnit * 2));

    area.removeFromTop(4);

    startStopButton.setBounds(area.removeFromTop(vUnit));

    area.removeFromTop(4);

    keyboardComponent.setBounds(area);
}

void PlayerComponent::audioDeviceAboutToStart(juce::AudioIODevice* device)
{

}

void PlayerComponent::audioDeviceStopped()
{

}

void PlayerComponent::audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                           int numInputChannels,
                           float* const* outputChannelData,
                           int numOutputChannels,
                           int numSamples,
                           const AudioIODeviceCallbackContext& context)
{

}

std::pair<FolderSelectResult, juce::File> PlayerComponent::selectInputFolder()
{
    loadChooser =
            std::make_unique<juce::FileChooser>("Choose folder of audio files to load...",
                                                juce::File::getSpecialLocation(juce::File::userDesktopDirectory),
                                                "",
                                                true);
#if JUCE_MAC || JUCE_WINDOWS || JUCE_LINUX
    if (loadChooser->browseForDirectory())
    {
        return std::make_pair<FolderSelectResult, juce::File>(FolderSelectResult::ok, loadChooser->getResult());
    }
#endif

    return std::make_pair<FolderSelectResult, juce::File>(FolderSelectResult::cancelled, juce::File());
}