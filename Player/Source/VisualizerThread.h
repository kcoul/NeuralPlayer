#pragma once

#include <JuceHeader.h>

#include "Paths.h"

class VisualizerThread : public juce::Thread
{
public:
    explicit VisualizerThread(std::function<void(String)>& textToPostFn) :
            juce::Thread("Visualizer Thread"),
            textToPost(textToPostFn)
    {

    }

    void run() override
    {
        auto vizPath = walkDebugDirectoryToProjectMSDL();
        if (vizPath.existsAsFile())
        {
            StringArray arguments;
            arguments.add(vizPath.getFullPathName());
            arguments.add("&"); //Don't block this app when opening project M
            ChildProcess p;
            p.start(arguments);

            //readAllProcessOutput cannot work in this kind of situation since childProcess is perpetually running
            //TODO: Try reading from projectM-SDL console output periodically using readProcessOutput()
            auto returnedText = p.readAllProcessOutput();
            if (returnedText.isNotEmpty())
                postText(returnedText);
        }
    }

    std::function<void(String)>& textToPost; //Don't call this directly, use the postText method TODO: fix design
private:
    void postText(String text)
    {
        juce::MessageManager::callAsync([this, text]
                                        {
                                            textToPost(text);
                                        });
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VisualizerThread)
};
