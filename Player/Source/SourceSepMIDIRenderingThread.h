#pragma once

#include <JuceHeader.h>
#include "Converters.h"

#include "BasicPitch.h"
#include "DownSampler.h"
#include "NoteOptions.h"
#include "MidiFileWriter.h"
#include "Paths.h"

class SourceSepMIDIRenderingThread : public juce::Thread
{
public:
    explicit SourceSepMIDIRenderingThread(std::function<void(String)>& textToPostFn,
                                          std::function<void(double)>& progressUpdateFn) :
    juce::Thread("SourceSepMIDI Rendering Thread"),
    textToPost(textToPostFn), progressUpdate(progressUpdateFn)
    {
        formatManager.registerBasicFormats();
        SpleeterRTBinFile = walkDebugDirectoryToSpleeterRTBin();
    }
    void run() override
    {
        stopRenderingFlag = false;

        //Capture GUI state once at beginning of render only
        useStockSpleeter = stockSpleeterFlag;
        useStockBasicPitch = stockBasicPitchFlag;

        if (useStockSpleeter)
        {
            if (renderFiles(juce::File("/usr/local/bin/spleeter")))
                playlistSuccessfullyGenerated = true;
        }
        else
        {
            if (renderFiles(SpleeterRTBinFile))
                playlistSuccessfullyGenerated = true;
        }
        signalThreadShouldExit();
    }

    void setInputFolder(juce::File inFolder) { inputFolder = std::move(inFolder); }
    void setDebugOutputFolder(juce::File outFolder) { debugOutputFolder = std::move(outFolder); }

    bool stockSpleeterFlag = false;
    bool stockBasicPitchFlag = false;
    bool stopRenderingFlag = false;
    bool playlistSuccessfullyGenerated = false;
    String playlistPath = "";
    std::function<void(String)>& textToPost; //Don't call this directly, use the postText method TODO: fix design
    std::function<void(double)>& progressUpdate;
    String currentFileName;
    int currentFileIndex = 0;
    int numFiles = 0;
private:
    void postText(String text)
    {
        juce::MessageManager::callAsync([this, text]
        {
            textToPost(text);
        });
    }
    juce::File SpleeterRTBinFile;
    XmlElement* playlist = nullptr;
    bool useStockSpleeter = false;
    bool useStockBasicPitch = false;

    AudioFormatManager formatManager;

    juce::File currentInputFile;
    juce::File debugOutputFolder;
    juce::File inputFolder;

    DownSampler downSampler;
    BasicPitch basicPitch;
    NoteOptions noteOptions;
    std::vector<Notes::Event> postProcessedNotes;
    MidiFileWriter midiFileWriter;

    bool renderFiles(juce::File bin)
    {
        auto files = inputFolder.findChildFiles(File::TypesOfFileToFind::findFiles,
                                                false, "*.wav");
        numFiles = files.size();

        playlist = new XmlElement("TABLEDATA");
        writePlaylistHeader(*playlist);
        XmlElement* data = new XmlElement("DATA");

        bool playlistContainsAtLeastOneEntry = false;

        for (auto inputFile : files)
        {
            currentFileName = inputFile.getFileName();
            if (renderSingleFile(bin, inputFile, *data))
            {
                playlistContainsAtLeastOneEntry = true;
                if (stopRenderingFlag) break;
            }
            else
            {
                if (stopRenderingFlag)
                {
                    postText("Render aborted for " + inputFile.getFileName());
                    break;
                }
                else
                    postText("Render failed for " + inputFile.getFileName());
            }
            progressUpdate((double)currentFileIndex / numFiles);
        }
        currentFileIndex = 0; //Reset counter in case we decide to keep thread alive indefinitely

        if (playlistContainsAtLeastOneEntry)
        {
            playlist->addChildElement(data);
            if (playlist->writeTo(inputFolder
                                         .getChildFile("Playlist.xml"), XmlElement::TextFormat()))
            {
                playlistPath = inputFolder.getChildFile("Playlist.xml").getFullPathName();
                delete (data);
                return true;
            }
        }
        delete (data);
        return false;
    }

    bool renderSingleFile(juce::File bin, juce::File inputFile, XmlElement& data)
    {
        //Source Separation
        auto binPath = bin.getFullPathName();
        auto inputPath = inputFile.getFullPathName();
        auto inputFilenameWithoutExtension = inputFile.getFileName().substring(0,
                                                                                  inputFile.getFileName().length() - 4);

        if (!applySourceSeparation(binPath, inputPath, inputFilenameWithoutExtension))
            return false;
        if (stopRenderingFlag) return false;

        if (!debugOutputFolder.exists())
            debugOutputFolder.createDirectory();

        //Make dual-mono inputFile from original and extracted vocal for ABing
        auto pwd = File::getCurrentWorkingDirectory(); //SpleeterRTBin saves files next to ParentProcess, aka this
        File vocalFile;
        if (useStockSpleeter)
        {
            vocalFile = File(pwd.getChildFile(inputFilenameWithoutExtension).getChildFile("vocals.wav"));
        }
        else
        {
            vocalFile = File(pwd.getChildFile(inputFilenameWithoutExtension + "_Vocal.wav"));
        }
        AudioSampleBuffer extractedVocalMonoBuffer; //TODO: Keep this around for streaming into visualizer
        double duration;

        if (!writeWavOutputs(inputFile, vocalFile, inputFilenameWithoutExtension, extractedVocalMonoBuffer, duration))
            return false;
        if (stopRenderingFlag) return false;
        if (!generateAndWriteMIDIFiles(extractedVocalMonoBuffer, vocalFile, inputFilenameWithoutExtension))
            return false;
        if (stopRenderingFlag) return false;

        //Add entry to Playlist.xml
        addPlaylistEntry(data, inputFilenameWithoutExtension, duration);

        return true;
    }

    bool applySourceSeparation(juce::String binPath, juce::String inputPath, juce::String inputFilenameWithoutExtension)
    {
        StringArray arguments;
        arguments.add(binPath);
        if (useStockSpleeter)
        {
            arguments.add("separate");
            arguments.add("-o");
            arguments.add(File::getCurrentWorkingDirectory().getFullPathName());
            //arguments.add(" -p");
            //arguments.add("spleeter:2stems-16kHz"); //':' character seems to be breaking p.start()... luckily -p is optional
            arguments.add(inputPath);
        }
        else
        {
            arguments.add("3"); //spawnNthreads
            arguments.add("512"); //timeStep
            arguments.add("1024"); //analyseBinLimit
            arguments.add("2"); //numStems (seems to ignore 4 and maxes at 3, SpleeterRTPlug does 4)
            arguments.add(inputPath);
        }
        currentFileIndex++;
#if DEBUG
        String argsConcat;
        for (auto arg : arguments)
            argsConcat += arg + " ";

        postText(argsConcat);
#else
        postText("Processing file " + String(currentFileIndex) + " of " +
                                                            String(numFiles) + ", " +
                                                            currentFileName);
#endif

        auto exe = arguments[0].unquoted();
        if (!File::getCurrentWorkingDirectory().getChildFile (exe).existsAsFile())
        {
            postText(exe + " does not exist. Did you forget to build (or install) it?");
            return false;
        }

        ChildProcess p;
        p.start(arguments);
        auto start = std::chrono::high_resolution_clock::now();
        auto returnedText = p.readAllProcessOutput();
        auto stop = std::chrono::high_resolution_clock::now();
        if (returnedText.isNotEmpty())
            postText(returnedText);
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
        auto seconds = (int)duration.count() / 1000;
        auto milliseconds = duration.count() % 1000;
        postText("Source Separation took " + (String)seconds + "." + (String)milliseconds + " seconds");

        //Make dual-mono inputFile from original and extracted vocal for ABing
        auto pwd = File::getCurrentWorkingDirectory();
        if (useStockSpleeter)
        {
            auto possibleVocalFile = File(pwd.getChildFile(inputFilenameWithoutExtension).getChildFile("vocals.wav"));
            if (!possibleVocalFile.existsAsFile())
            {
                postText("Vocal extraction operation failed.");
                return false;
            }
        }
        else
        {
            auto possibleVocalFile = File(pwd.getChildFile(inputFilenameWithoutExtension + "_Vocal.wav"));
            if (!possibleVocalFile.existsAsFile())
            {
                postText("Vocal extraction operation failed.");
                return false;
            }
        }
        return true;
    }

    bool writeWavOutputs(const juce::File& inputFile, const juce::File& vocalFile, juce::String inputFilenameWithoutExtension,
                         AudioSampleBuffer& extractedVocalMonoBuffer, double& duration)
    {
        //Combine ch.1 of inputFile with ch.1 of vocalFile and save as a dual-mono inputFile
        AudioFormatReader *reader = formatManager.createReaderFor(inputFile);
        auto origNumSamples = reader->lengthInSamples;
        AudioSampleBuffer originalMonoBuffer (1, origNumSamples);
        reader->read(originalMonoBuffer.getArrayOfWritePointers(), 1, 0, origNumSamples);

        reader = formatManager.createReaderFor(vocalFile);
        auto vocalNumSamples = reader->lengthInSamples;
        duration = reader->lengthInSamples / reader->sampleRate;
        jassert(vocalNumSamples == origNumSamples);
        extractedVocalMonoBuffer.setSize(1, vocalNumSamples);
        reader->read(extractedVocalMonoBuffer.getArrayOfWritePointers(), 1, 0, vocalNumSamples);

        delete reader;

        //Delete temp files
        if (!useStockBasicPitch) //We need vocalFile on disk a little longer if using stock basic-pitch
        {
            if (vocalFile.existsAsFile())
                vocalFile.deleteFile();
        }

        File possibleAccompanimentFile;
        if (useStockSpleeter)
        {
            possibleAccompanimentFile = vocalFile.getParentDirectory().getChildFile("accompaniment.wav");
        }
        else
        {
            possibleAccompanimentFile = vocalFile.getParentDirectory().getChildFile(inputFilenameWithoutExtension + "_Accompaniment.wav");
        }

        if (possibleAccompanimentFile.existsAsFile())
            possibleAccompanimentFile.deleteFile();

        if (useStockSpleeter)
        {
            auto dir = vocalFile.getParentDirectory();
            if (dir.exists())
                dir.deleteFile();
        }

        AudioSampleBuffer dualMonoBuffer;
        dualMonoBuffer.setSize(2, vocalNumSamples);
        dualMonoBuffer.copyFrom(0, 0, originalMonoBuffer.getReadPointer(0), vocalNumSamples);
        dualMonoBuffer.copyFrom(1, 0, extractedVocalMonoBuffer.getReadPointer(0), vocalNumSamples);

        std::unique_ptr<AudioFormatWriter> writer;
        WavAudioFormat format;
        writer.reset (format.createWriterFor (new FileOutputStream (
                                                      debugOutputFolder.getChildFile(inputFilenameWithoutExtension +
                                                                                     "_DualMono.wav")),
                                              44100.0,
                                              dualMonoBuffer.getNumChannels(),
                                              16,
                                              {},
                                              0));
        if (writer == nullptr)
            return false;
        writer->writeFromAudioSampleBuffer (dualMonoBuffer, 0, dualMonoBuffer.getNumSamples());

        //TODO: save vocal inputFile (if necessary) to stream directly into visualizer
        //writer.reset (format.createWriterFor (new FileOutputStream (debugOutputFolder.getChildFile(inputFilenameWithoutExtension + "_VocalMono.wav")),
        //                                      44100.0,
        //                                      extractedVocalMonoBuffer.getNumChannels(),
        //                                      16,
        //                                      {},
        //                                      0));
        //if (writer == nullptr)
        //    return false;
        //writer->writeFromAudioSampleBuffer (extractedVocalMonoBuffer, 0, extractedVocalMonoBuffer.getNumSamples());

        return true;
    }

    bool generateAndWriteMIDIFiles(const AudioSampleBuffer& extractedVocalMonoBuffer,
                                   const juce::File& vocalFile,
                                   const juce::String& inputFilenameWithoutExtension)
    {
        if (useStockBasicPitch) //Write to debug folder only, stock basic-pitch MIDI is not used in this app
        {
            StringArray arguments;
            arguments.add("/usr/local/bin/basic-pitch");
            auto debugFolder = inputFolder.getFullPathName() + "/debug";
            arguments.add(debugFolder);
            arguments.add(vocalFile.getFullPathName());

            auto exe = arguments[0].unquoted();
            if (!File::getCurrentWorkingDirectory().getChildFile (exe).existsAsFile())
            {
                postText(exe + " does not exist. Did you forget to build (or install) it?");
                return false;
            }

            ChildProcess p;
            p.start(arguments);
            auto start = std::chrono::high_resolution_clock::now();
            auto returnedText = p.readAllProcessOutput();
            auto stop = std::chrono::high_resolution_clock::now();
            if (returnedText.isNotEmpty())
                postText(returnedText);
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
            auto seconds = (int)duration.count() / 1000;
            auto milliseconds = duration.count() % 1000;
            postText("Vocal MIDI transcription took " + (String)seconds + "." + (String)milliseconds + " seconds\n");

            auto midiOutput = File(debugFolder).getChildFile("vocals_basic_pitch.mid");
            if (midiOutput.existsAsFile())
            {
                auto midiDebugOutput = inputFolder.getFullPathName() + "/debug/" + inputFilenameWithoutExtension + "_Vocal_basic_pitch.mid";
                midiOutput.copyFileTo(File(midiDebugOutput));
                midiOutput.deleteFile();
                vocalFile.deleteFile();
                return true;
            }
            else
            {
                vocalFile.deleteFile();
                return false;
            }
        }
        else
        {
            //MIDI Processing
            //Resample from 44100 to 22050 all in one shot per file for now
            AudioSampleBuffer mAudioBufferForMIDITranscription;
            mAudioBufferForMIDITranscription.setSize(1, extractedVocalMonoBuffer.getNumSamples()/2);
            downSampler.prepareToPlay(44100, extractedVocalMonoBuffer.getNumSamples());
            downSampler.processBlock(
                    extractedVocalMonoBuffer,
                    mAudioBufferForMIDITranscription.getWritePointer(0),
                    extractedVocalMonoBuffer.getNumSamples());

            basicPitch.setParameters(0.7,
                                     0.5,
                                     125);
            auto start = std::chrono::high_resolution_clock::now();
            basicPitch.transcribeToMIDI(mAudioBufferForMIDITranscription.getWritePointer(0),
                                        mAudioBufferForMIDITranscription.getNumSamples());
            auto stop = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
            auto seconds = (int)duration.count() / 1000;
            auto milliseconds = duration.count() % 1000;
            postText("Vocal MIDI transcription took " + (String)seconds + "." + (String)milliseconds + " seconds\n");

            auto noteEvents = basicPitch.getNoteEvents();

            //Write MIDI to debug output folder to be able to load it alongside DualMono file in AudioFilePlayerPlugin
            auto debugMidiOutputFilePath = debugOutputFolder.getChildFile(inputFilenameWithoutExtension + "_DualMono.mid");
            //Also write MIDI to input folder to be able to load it as part of Playlist.xml
            auto releaseMidiOutputFilePath = inputFolder.getChildFile(inputFilenameWithoutExtension + ".mid");
            if (!midiFileWriter.writeMidiFile(noteEvents, debugMidiOutputFilePath, 120) ||
                !midiFileWriter.writeMidiFile(noteEvents, releaseMidiOutputFilePath, 120))
            {
                postText("MIDI write operation failed.");
                return false;
            }
        }
        return true;
    }

    void writePlaylistHeader(XmlElement& playlist)
    {

        XmlElement* headers = new XmlElement("HEADERS");

        for (int i = 1; i <= 3; ++i)
        {
            XmlElement* column = new XmlElement("COLUMN");

            column->setAttribute("columnId", i);
            if (i == 1)
            {
                column->setAttribute("name", "Title");
                column->setAttribute("width", 320);
            }
            else if (i == 2)
            {
                column->setAttribute("name", "Author");
                column->setAttribute("width", 100);
            }
            else if (i == 3)
            {
                column->setAttribute("name", "Duration");
                column->setAttribute("width", 100);
            }

            headers->addChildElement(column);
        }
        playlist.addChildElement(headers);
    }

    static void addPlaylistEntry(XmlElement& data, const juce::String& inputFilenameWithoutExtension, double duration)
    {
        XmlElement* item = new XmlElement("ITEM");

        item->setAttribute("Title", inputFilenameWithoutExtension);
        item->setAttribute("Author", tryParseAuthor(inputFilenameWithoutExtension));
        item->setAttribute("Duration", tryParseDuration(duration));
        data.addChildElement(item);
    }

    static String tryParseAuthor(const juce::String& inputFilenameWithoutExtension)
    {
        if (inputFilenameWithoutExtension.contains(" - "))
        {
            return inputFilenameWithoutExtension.substring(0, inputFilenameWithoutExtension.indexOf(" - "));
        }
        else if (inputFilenameWithoutExtension.contains(" ~ "))
        {
            return inputFilenameWithoutExtension.substring(0, inputFilenameWithoutExtension.indexOf(" ~ "));
        }
        else if (inputFilenameWithoutExtension.contains(" by "))
        {
            return inputFilenameWithoutExtension.substring(inputFilenameWithoutExtension.indexOf(" by ") + 4,
                                                           inputFilenameWithoutExtension.length());
        }
        else return "";
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SourceSepMIDIRenderingThread)
};
