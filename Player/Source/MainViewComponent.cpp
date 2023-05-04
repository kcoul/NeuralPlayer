#include "MainViewComponent.h"

MainViewComponent::MainViewComponent()
{
    renderingThread = std::make_unique<SpleeterRTBinRenderingThread>();

    formatManager.registerBasicFormats();
    setSize(700, 800);

    audioSettings = std::make_unique<juce::AudioDeviceSelectorComponent>(audioDeviceManager,
                                                                   0,
                                                                   0,
                                                                   2,
                                                                   2,
                                                                   false,
                                                                   true,
                                                                   true,
                                                                   false);




    addAndMakeVisible(*audioSettings);

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

    consoleViewComponent = std::make_unique<ConsoleViewComponent>(renderingThread->returnedText);
    addAndMakeVisible(*consoleViewComponent);



}

MainViewComponent::~MainViewComponent()
{

}

void MainViewComponent::resized()
{
    auto area = getLocalBounds().reduced(4);
#if JUCE_IOS
    area.removeFromTop(30);
#endif

    auto vUnit = area.getHeight()/12;

    if(audioSettings)
        audioSettings->setBounds(area.removeFromTop(vUnit * 3));

    loadButton.setBounds(area.removeFromTop(vUnit));

    area.removeFromTop(vUnit * 3);

    if(consoleViewComponent)
        consoleViewComponent->setBounds(area);
}

std::pair<FolderSelectResult, juce::File> MainViewComponent::selectInputFolder()
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


