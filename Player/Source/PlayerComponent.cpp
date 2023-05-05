#include "PlayerComponent.h"

PlayerComponent::PlayerComponent(std::unique_ptr<SourceSepMIDIRenderingThread>& thread) : renderingThread(thread)
{
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
}

PlayerComponent::~PlayerComponent()
{

}

void PlayerComponent::paint(juce::Graphics& g)
{

}

void PlayerComponent::resized()
{
    auto area = getLocalBounds().reduced(4);
#if JUCE_IOS
    area.removeFromTop(30);
#endif
    auto vUnit = area.getHeight()/12;
    loadButton.setBounds(area.removeFromTop(vUnit));
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