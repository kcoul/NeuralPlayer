#pragma once

#include <JuceHeader.h>
#include "DataStructures.h"

#include "BasicPitch.h"
#include "DownSampler.h"
#include "NoteOptions.h"
#include "MidiFileWriter.h"
#include "RhythmOptions.h"

class SourceSepMIDIRenderingThread : public juce::Thread
{
public:
    SourceSepMIDIRenderingThread() : juce::Thread("SourceSepMIDI Rendering Thread")
    {
        formatManager.registerBasicFormats();
    }
    void run() override
    {
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

        //renderSingleFile(pwd, currentInputFile);
        renderFiles(pwd);

        signalThreadShouldExit();
    }

    void setInputFolder(juce::File inFolder) { inputFolder = std::move(inFolder); }
    void setOutputFolder(juce::File outFolder) { outputFolder = std::move(outFolder); }
    bool stopRenderingFlag = false;

    WatchedVars threadVars;
private:
    AudioFormatManager formatManager;

    juce::File currentInputFile;
    juce::File outputFolder;
    juce::File inputFolder;

    DownSampler mDownSampler;
    BasicPitch mBasicPitch;
    NoteOptions mNoteOptions;
    RhythmOptions mRhythmOptions;
    std::vector<Notes::Event> mPostProcessedNotes;
    MidiFileWriter mMidiFileWriter;

    void renderFiles(juce::File bin)
    {
        auto files = inputFolder.findChildFiles(File::TypesOfFileToFind::findFiles,
                                                false, "*.wav");
        threadVars.numFiles = files.size();
        for (auto inputFile : files)
        {
            threadVars.currentFileName = inputFile.getFileName();
            threadVars.currentFileIndex++;
            renderSingleFile(bin, inputFile);
        }
        threadVars.currentFileIndex = 0;
    }

    void renderSingleFile(juce::File bin, juce::File inputFile)
    {
        //Source Separation Workbench

        const String fullPathName = inputFile.getFullPathName();
        StringArray arguments;
        arguments.add(bin.getFullPathName());
        arguments.add("3"); //spawnNthreads
        arguments.add("512"); //timeStep
        arguments.add("1024"); //analyseBinLimit
        arguments.add("2"); //numStems (seems to ignore 4 and maxes at 3, SpleeterRTPlug does 4)
        arguments.add(fullPathName);

        ChildProcess p;
        p.start(arguments);
        threadVars.returnedText = p.readAllProcessOutput();

        //Make dual-mono inputFile from original and extracted vocal for ABing
        auto pwd = File::getCurrentWorkingDirectory();

        auto originalFilenameWithoutExtension = inputFile.getFileName().substring(0,
                                                                                  inputFile.getFileName().length() - 4);
        auto possibleVocalFile = File(pwd.getChildFile(originalFilenameWithoutExtension + "_Vocal.wav"));
        if (possibleVocalFile.existsAsFile())
        {
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


            //MIDI Processing Workbench
            //Resample from 44100 to 22050 all in one shot per file for now
            AudioSampleBuffer mAudioBufferForMIDITranscription;
            mAudioBufferForMIDITranscription.setSize(1, extractedVocalMonoBuffer.getNumSamples()/2);
            mDownSampler.prepareToPlay(44100, extractedVocalMonoBuffer.getNumSamples());
            mDownSampler.processBlock(
                    extractedVocalMonoBuffer,
                    mAudioBufferForMIDITranscription.getWritePointer(0),
                    extractedVocalMonoBuffer.getNumSamples());

            mBasicPitch.setParameters(0.7,
                                      0.5,
                                      125);

            mBasicPitch.transcribeToMIDI(mAudioBufferForMIDITranscription.getWritePointer(0),
                                         mAudioBufferForMIDITranscription.getNumSamples());

            auto noteEvents = mBasicPitch.getNoteEvents();
            auto midiOutputFilePath = outputFolder.getChildFile(originalFilenameWithoutExtension + ".mid");
            if (!mMidiFileWriter.writeMidiFile(
                    noteEvents,
                    midiOutputFilePath,
                    120))
            {
                threadVars.returnedText = "MIDI write operation failed.";
            }
            else
            {
                threadVars.returnedText = "MIDI file generated.";
            }
        }

    }
};
