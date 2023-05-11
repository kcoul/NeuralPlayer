#include "PlayerComponent.h"

#include <memory>

PlayerComponent::PlayerComponent() :
        neuralPlayer(latestMIDIBufferFn, latestPlaybackLocationFn),
        progressBar(renderingProgress),
        sidePanelHeader("Software Debug Console"),
        softwareConsoleComponentPanel("Software Debug Console Panel", 700, false, nullptr, false),
        playlistComponent(trackSelected),
        thumbnailCache(1),
        readAheadThread("transport read ahead"),
        keyboardComponent (keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    renderingThread = std::make_unique<SourceSepMIDIRenderingThread>(textToPost, progressUpdate);
    textToPost = [this] (String text) { softwareConsoleComponent.insertText(text, true); };
    progressUpdate = [this] (double p) { renderingProgress = p; };
    renderingThread->addListener(this);
    formatManager.registerBasicFormats();
    readAheadThread.startThread();

    debugResourcesDirectory = walkDebugDirectoryToResourcesFolder();

    trackSelected = [this] (String trackName)
    {
        auto trackPath = currentPlaylistDirectory.getChildFile(trackName + ".wav");

        if(currentlyLoadedFile != trackPath && trackPath.existsAsFile())
        {
            threadPool.addJob(midiFlushJob);
            threadPool.addJob(transportStopJob);
            startStopButton.setButtonText("Play");
            keyboardState.allNotesOff(1);
            playbackProgressLabel.setText("00:00", juce::dontSendNotification);

            loadAudioFileIntoTransport(trackPath);
            thumbnail->setFile(trackPath);

            //Try loading a MIDI file of same path/name to MIDI player
            auto possibleMIDIFile = File(trackPath.getFullPathName().substring(0,
                                                                               trackPath.getFullPathName().length() - 4) +
                                                                                       ".mid");
            if (possibleMIDIFile.existsAsFile())
                loadMIDIFile(possibleMIDIFile);
        }
    };

    loadNewFolderButton.onClick = [this]
    {
        auto result = selectNewInputFolder();
        if (result.first == FolderSelectResult::ok)
        {
            auto inputFolder = result.second;
            renderingThread->setInputFolder(inputFolder);
            renderingThread->setDebugOutputFolder(File(inputFolder.getFullPathName() + "/debug"));
            renderingThread->startThread();
        };
    };
    addAndMakeVisible(loadNewFolderButton);

    haltButton.onClick = [this]
    {
        renderingThread->stopRenderingFlag = true;
    };
    addAndMakeVisible(haltButton);

    loadExistingPlaylistButton.onClick = [this]
    {
        auto result = selectExistingPlaylistFile();
        if (result.first == FolderSelectResult::ok)
        {
            auto playlistXML = result.second;
            currentPlaylistDirectory = playlistXML.getParentDirectory();
            playlistComponent.loadData(playlistXML);
        };
    };
    addAndMakeVisible(loadExistingPlaylistButton);

    addAndMakeVisible(progressBar);

    juce::Path p;
    p.clear();
    p.loadPathFromData(settingsIconPathData, sizeof(settingsIconPathData));
    openSidePanelButton.setShape(p, true, true, false);
    openSidePanelButton.onClick = [this] { softwareConsoleComponentPanel.showOrHide(true); };
    addAndMakeVisible(openSidePanelButton);

    playbackProgressLabel.setJustificationType(Justification::right);
    addAndMakeVisible(playbackProgressLabel);

    sidePanelHeader.setHomeButtonClicked([this] { softwareConsoleComponent.clear(); });
    softwareConsoleComponentPanel.setTitleBarComponent(&sidePanelHeader, true, false);
    softwareConsoleComponentPanel.setContent(&softwareConsoleComponent, false);
    addAndMakeVisible(softwareConsoleComponentPanel);

    addAndMakeVisible(playlistComponent);

    thumbnail = std::make_unique<AudioThumbnailComponent>(formatManager,
                                                          neuralPlayer.transportSource,
                                                          thumbnailCache,
                                                          thumbnailPlayheadPositionChanged,
                                                          currentlyLoadedFile);
    addAndMakeVisible(thumbnail.get());

    midiFlushJob = [this]()
            { neuralPlayer.sendAllNotesOff(); };
    transportStartJob = [this]()
            { neuralPlayer.transportSource.setPosition(0);
              neuralPlayer.transportSource.start(); };
    transportStopJob = [this]() { neuralPlayer.transportSource.stop(); };

    startStopButton.setButtonText("Play");
    startStopButton.onClick = [this]
    {
        if (neuralPlayer.transportSource.isPlaying())
        {
            threadPool.addJob(midiFlushJob);
            threadPool.addJob(transportStopJob);
            startStopButton.setButtonText("Play");
            keyboardState.allNotesOff(1);
            playbackProgressLabel.setText("00:00", juce::dontSendNotification);
        }
        else
        {
            threadPool.addJob(transportStartJob);
            startStopButton.setButtonText("Stop");
        }
    };
    addAndMakeVisible(startStopButton);

    neuralPlayer.streamFinishedCallback = [this]
    {
        threadPool.addJob(midiFlushJob);
        threadPool.addJob(transportStopJob);
        startStopButton.setButtonText("Play");
        keyboardState.allNotesOff(1);
    };

    latestMIDIBufferFn = [this] (juce::MidiBuffer latestBuffer)
    {
        keyboardState.processNextMidiBuffer(neuralPlayer.latestMIDIBuffer,
                                            0,
                                            neuralPlayer.numSamplesPerBuffer,
                                            false);
    };
    latestPlaybackLocationFn = [this] (double location)
    {
        juce::MessageManager::callAsync([this, location]
        {
            playbackProgressLabel.setText(tryParseDuration(location), juce::dontSendNotification);
        });

    };
    addAndMakeVisible(keyboardComponent);
}

PlayerComponent::~PlayerComponent()
{
    renderingThread->removeListener(this);
    renderingThread->stopRenderingFlag = true;
    renderingThread->stopThread(1000);
    softwareConsoleComponentPanel.setTitleBarComponent(nullptr, true);
    softwareConsoleComponentPanel.setContent(nullptr);
}

void PlayerComponent::paint(juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    auto normal = getLookAndFeel().findColour(juce::SidePanel::dismissButtonNormalColour);
    auto over = getLookAndFeel().findColour(juce::SidePanel::dismissButtonOverColour);
    auto down = getLookAndFeel().findColour(juce::SidePanel::dismissButtonDownColour);

    openSidePanelButton.setColours(normal, over, down);
}

void PlayerComponent::resized()
{
    auto area = getLocalBounds().reduced(4);
#if JUCE_IOS
    area.removeFromTop(30);
#endif

    auto vUnit = area.getHeight()/12;
    auto halfVUnit = vUnit / 2;

    auto halfVUnitSlot = area.removeFromTop(halfVUnit);

    openSidePanelButton.setBounds(halfVUnitSlot.removeFromLeft(halfVUnit));
    playbackProgressLabel.setBounds(halfVUnitSlot.removeFromRight(vUnit));

    area.removeFromTop(4);

    auto vUnitSlot = area.removeFromTop(vUnit);
    loadNewFolderButton.setBounds(vUnitSlot.removeFromLeft(vUnitSlot.getWidth()/3).reduced(2, 0));
    haltButton.setBounds(vUnitSlot.removeFromLeft(vUnitSlot.getWidth()/2).reduced(2, 0));
    loadExistingPlaylistButton.setBounds(vUnitSlot.reduced(2, 0));

    area.removeFromTop(4);

    progressBar.setBounds(area.removeFromTop(halfVUnit));

    playlistComponent.setBounds(area.removeFromTop(vUnit * 5));

    area.removeFromTop(4);

    thumbnail->setBounds(area.removeFromTop(vUnit * 2));

    area.removeFromTop(4);

    startStopButton.setBounds(area.removeFromTop(vUnit));

    area.removeFromTop(4);

    keyboardComponent.setBounds(area);
}

std::pair<FolderSelectResult, juce::File> PlayerComponent::selectNewInputFolder()
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

std::pair<FolderSelectResult, juce::File> PlayerComponent::selectExistingPlaylistFile()
{
    loadChooser =
            std::make_unique<juce::FileChooser>("Find Playlist.xml for previously rendered folder...",
                                                debugResourcesDirectory,
                                                "*.xml",
                                                true);
#if JUCE_MAC || JUCE_WINDOWS || JUCE_LINUX
    if (loadChooser->browseForFileToOpen())
    {
        return std::make_pair<FolderSelectResult, juce::File>(FolderSelectResult::ok, loadChooser->getResult());
    }
#endif

    return std::make_pair<FolderSelectResult, juce::File>(FolderSelectResult::cancelled, juce::File());
}

void PlayerComponent::loadAudioFileIntoTransport(const File& audioFile)
{
    // unload the previous file source and delete it..
    neuralPlayer.transportSource.stop();
    neuralPlayer.transportSource.setSource(nullptr);
    currentAudioFileSource = nullptr;

    AudioFormatReader* reader = formatManager.createReaderFor(audioFile);
    currentlyLoadedFile = audioFile;

    if (reader != nullptr)
    {
        currentAudioFileSource = std::make_unique<AudioFormatReaderSource>(reader, true);

        // ..and plug it into our transport source
        neuralPlayer.transportSource.setSource(
                currentAudioFileSource.get(),
                32768,                   // tells it to buffer this many samples ahead
                &readAheadThread,        // this is the background thread to use for reading-ahead
                reader->sampleRate);     // allows for sample rate correction
    }
}

void PlayerComponent::loadMIDIFile(const File& file)
{
    neuralPlayer.MIDIFile.clear();

    juce::FileInputStream stream(file);
    neuralPlayer.MIDIFile.readFrom(stream);

    //This function call means that the MIDI file is going to be played with the original tempo and signature.
    neuralPlayer.MIDIFile.convertTimestampTicksToSeconds();
}

juce::File PlayerComponent::walkDebugDirectoryToResourcesFolder()
{
    auto pwd = File::getCurrentWorkingDirectory();
    while (true) //Ascend to build inputFolder
    {
        pwd = pwd.getParentDirectory();
        if (pwd.getFileName().endsWith("cmake-build-debug") ||
            pwd.getFileName().endsWith("cmake-build-release") ||
            pwd.getFileName().endsWith("cmake-build-relwithdebinfo"))
            break;
    }

    pwd = pwd.getParentDirectory();
    pwd = pwd.getChildFile("Player/Resources/");

    return pwd;
}

void PlayerComponent::exitSignalSent()
{
    if (renderingThread->playlistSuccessfullyGenerated)
    {
        auto path = renderingThread->playlistPath;

        juce::MessageManager::callAsync( [this, path]
        {
            processingIndex = 0;
            auto textToPost = "Done!\n";
            softwareConsoleComponent.insertText(textToPost, true);

            playlistComponent.loadData(path);
            currentPlaylistDirectory = File(path).getParentDirectory();
            renderingProgress = 0.0;
            processingIndex = 0;
        });
    }

    renderingThread->playlistPath = "";
    renderingThread->playlistSuccessfullyGenerated = false;
}