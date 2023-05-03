#include "MainViewComponent.h"

MainViewComponent::MainViewComponent()
{
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
            auto folder = result.second;

            auto pwd = File::getCurrentWorkingDirectory();

            while (true) //Ascend to build folder
            {
                pwd = pwd.getParentDirectory();
                if (pwd.getFileName().endsWith("cmake-build-debug") ||
                    pwd.getFileName().endsWith("cmake-build-release"))
                    break;
            }

            //Descend to SpleeterRTBin location
            pwd = pwd.getChildFile("SpleeterRT/Executable/SpleeterRTBin");

            for (auto file : folder.findChildFiles(File::TypesOfFileToFind::findFiles,
                                                   false, "*.wav"))
            {
                StringArray arguments;
                arguments.add(pwd.getFullPathName());
                arguments.add("3"); //spawnNthreads
                arguments.add("512"); //timeStep
                arguments.add("1024"); //analyseBinLimit
                arguments.add("2"); //numStems (seems to ignore 4 and maxes at 3, SpleeterRTPlug does 4)
                arguments.add(file.getFullPathName());

                //String args;
                //for (auto a: arguments)
                //    args.append(a + " ", 255);

                ChildProcess p;
                p.start(arguments);
                //TODO: Get output as it arrives
                //TODO: Move processing to thread so it doesn't block GUI
                String returnedText = p.readAllProcessOutput();
                if (returnedText.isNotEmpty())
                    consoleViewComponent->insertText(returnedText, true);

                //Make dual-mono file from original and extracted vocal for ABing
                pwd = File::getCurrentWorkingDirectory();

                auto possibleVocalFile = File(pwd.getChildFile(file.getFileName().substring(0,
                                                                                            file.getFileName().length() - 4) + "_Vocal.wav"));
                if (possibleVocalFile.existsAsFile())
                {
                    int i = 1;
                }

                //Re-save vocal file in mono to get MIDI more quickly from NeuralNote
            }
        };
    };
    addAndMakeVisible(loadButton);

    consoleViewComponent = std::make_unique<ConsoleViewComponent>();
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


