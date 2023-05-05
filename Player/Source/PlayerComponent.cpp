#include "PlayerComponent.h"

PlayerComponent::PlayerComponent(std::unique_ptr<SourceSepMIDIRenderingThread>& thread) :
renderingThread(thread),
playlistComponent(trackSelected),
thumbnailCache(1),
readAheadThread("transport read ahead"),
keyboardComponent (keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    formatManager.registerBasicFormats();
    readAheadThread.startThread();

    trackSelected = [this] (String trackName)
    {
        auto pwd = File::getCurrentWorkingDirectory();
        while (true) //Ascend to build inputFolder
        {
            pwd = pwd.getParentDirectory();
            if (pwd.getFileName().endsWith("cmake-build-debug") ||
                pwd.getFileName().endsWith("cmake-build-release"))
                break;
        }

        pwd = pwd.getParentDirectory();
        pwd = pwd.getChildFile("Player/Resources/" + trackName + ".wav");

        if(currentlyLoadedFile != pwd && pwd.existsAsFile())
        {
            loadAudioFileIntoTransport(pwd);
            thumbnail->setFile(pwd);

            //Try loading a MIDI file of same path/name to MIDI player
            auto possibleMIDIFile = File(pwd.getFullPathName().substring(0,
                                                        pwd.getFullPathName().length() - 4) + ".mid");
            if (possibleMIDIFile.existsAsFile())
                loadMIDIFile(possibleMIDIFile);
        }
    };

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

    thumbnail = std::make_unique<AudioThumbnailComponent>(formatManager,
                                                     audioMIDIPlayer.transportSource,
                                                     thumbnailCache,
                                                     thumbnailPlayheadPositionChanged,
                                                     currentlyLoadedFile);
    addAndMakeVisible(thumbnail.get());

    addAndMakeVisible(startStopButton);
    startStopButton.setButtonText("Play");

    midiFlushJob = [this]() { audioMIDIPlayer.sendAllNotesOff(); };
    transportStartJob = [this]()
            { audioMIDIPlayer.transportSource.setPosition(0);
              audioMIDIPlayer.transportSource.start(); };
    transportStopJob = [this]() { audioMIDIPlayer.transportSource.stop(); };

    startStopButton.onClick = [this]
    {
        if (audioMIDIPlayer.transportSource.isPlaying())
        {
            threadPool.addJob(midiFlushJob);
            threadPool.addJob(transportStopJob);
            startStopButton.setButtonText("Play");
        }
        else
        {
            threadPool.addJob(transportStartJob);
            startStopButton.setButtonText("Stop");
        }
    };
    addAndMakeVisible(startStopButton);

    addAndMakeVisible(keyboardComponent);

    audioDeviceManager.addAudioCallback(&audioMIDIPlayer);
}

PlayerComponent::~PlayerComponent()
{
    audioDeviceManager.removeAudioCallback(&audioMIDIPlayer);
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

void PlayerComponent::loadAudioFileIntoTransport(const File& audioFile)
{
    // unload the previous file source and delete it..
    audioMIDIPlayer.transportSource.stop();
    audioMIDIPlayer.transportSource.setSource(nullptr);
    currentAudioFileSource = nullptr;

    AudioFormatReader* reader = formatManager.createReaderFor(audioFile);
    currentlyLoadedFile = audioFile;

    if (reader != nullptr)
    {
        currentAudioFileSource = std::make_unique<AudioFormatReaderSource>(reader, true);

        // ..and plug it into our transport source
        audioMIDIPlayer.transportSource.setSource(
                currentAudioFileSource.get(),
                32768,                   // tells it to buffer this many samples ahead
                &readAheadThread,        // this is the background thread to use for reading-ahead
                reader->sampleRate);     // allows for sample rate correction
    }
}

void PlayerComponent::loadMIDIFile(const File& file)
{
    audioMIDIPlayer.MIDIFile.clear();

    juce::FileInputStream stream(file);
    audioMIDIPlayer.MIDIFile.readFrom(stream);

    //This function call means that the MIDI file is going to be played with the original tempo and signature.
    audioMIDIPlayer.MIDIFile.convertTimestampTicksToSeconds();
}