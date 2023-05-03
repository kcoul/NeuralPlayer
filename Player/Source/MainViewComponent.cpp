#include "MainViewComponent.h"

MainViewComponent::MainViewComponent()
{
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

            auto pwd = File::getCurrentWorkingDirectory();

            while (true) //Ascend to build inputFolder
            {
                pwd = pwd.getParentDirectory();
                if (pwd.getFileName().endsWith("cmake-build-debug") ||
                    pwd.getFileName().endsWith("cmake-build-release"))
                    break;
            }

            //Descend to SpleeterRTBin location
            pwd = pwd.getChildFile("SpleeterRT/Executable/SpleeterRTBin");

            for (auto inputFile : inputFolder.findChildFiles(File::TypesOfFileToFind::findFiles,
                                                             false, "*.wav"))
            {
                StringArray arguments;
                arguments.add(pwd.getFullPathName());
                arguments.add("3"); //spawnNthreads
                arguments.add("512"); //timeStep
                arguments.add("1024"); //analyseBinLimit
                arguments.add("2"); //numStems (seems to ignore 4 and maxes at 3, SpleeterRTPlug does 4)
                arguments.add(inputFile.getFullPathName());

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

                //Make dual-mono inputFile from original and extracted vocal for ABing
                pwd = File::getCurrentWorkingDirectory();

                auto originalFilenameWithoutExtension = inputFile.getFileName().substring(0,
                                                                                          inputFile.getFileName().length() - 4);

                auto possibleVocalFile = File(pwd.getChildFile(originalFilenameWithoutExtension + "_Vocal.wav"));
                if (possibleVocalFile.existsAsFile())
                {
                    auto outputFolder = File(inputFolder.getFullPathName() + "/out");
                    if (!outputFolder.exists())
                        outputFolder.createDirectory();

                    //Combine ch.1 of inputFile with ch.1 of possibleVocalFile and save as a dual-mono inputFile
                    AudioFormatReader *reader = formatManager.createReaderFor(inputFile);
                    auto origNumSamples = reader->lengthInSamples;
                    AudioSampleBuffer originalMonoBuffer (1, origNumSamples);
                    reader->read(originalMonoBuffer.getArrayOfWritePointers(), 1, 0, origNumSamples);

                    reader = formatManager.createReaderFor(possibleVocalFile);
                    auto vocalNumSamples = reader->lengthInSamples;
                    jassert(vocalNumSamples == origNumSamples);
                    AudioSampleBuffer extractedVocalMonoBuffer(1, vocalNumSamples);
                    reader->read(extractedVocalMonoBuffer.getArrayOfWritePointers(), 1, 0, vocalNumSamples);

                    delete reader;

                    AudioSampleBuffer dualMonoBuffer;
                    dualMonoBuffer.setSize(2, vocalNumSamples);
                    dualMonoBuffer.copyFrom(0, 0, originalMonoBuffer.getReadPointer(0), vocalNumSamples);
                    dualMonoBuffer.copyFrom(1, 0, extractedVocalMonoBuffer.getReadPointer(0), vocalNumSamples);

                    std::unique_ptr<AudioFormatWriter> writer;
                    WavAudioFormat format;
                    writer.reset (format.createWriterFor (new FileOutputStream (outputFolder.getChildFile(originalFilenameWithoutExtension + "_DualMono.wav")),
                                                          44100.0,
                                                          dualMonoBuffer.getNumChannels(),
                                                          16,
                                                          {},
                                                          0));
                    if (writer != nullptr)
                        writer->writeFromAudioSampleBuffer (dualMonoBuffer, 0, dualMonoBuffer.getNumSamples());

                    //Also re-save vocal inputFile in mono to get MIDI more quickly from NeuralNote
                    writer.reset (format.createWriterFor (new FileOutputStream (outputFolder.getChildFile(originalFilenameWithoutExtension + "_VocalMono.wav")),
                                                          44100.0,
                                                          extractedVocalMonoBuffer.getNumChannels(),
                                                          16,
                                                          {},
                                                          0));
                    if (writer != nullptr)
                        writer->writeFromAudioSampleBuffer (extractedVocalMonoBuffer, 0, extractedVocalMonoBuffer.getNumSamples());
                }
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


