#pragma once

#include <JuceHeader.h>
#include "DataStructures.h"

class ConsoleViewComponent : public juce::Component,
                             public juce::Thread::Listener,
                             juce::Timer
{
public:
    //==============================================================================
    ConsoleViewComponent(WatchedVars& threadVars);
    ~ConsoleViewComponent() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

    void exitSignalSent() override;

    void insertText(juce::String text, bool addCarriageReturn);
    void clear();
protected:
    void visibilityChanged() override;
private:
    //==============================================================================
    WatchedVars& watchedVariables;
    String lastDisplayedString = "";
    int processingIndex = 0;

    std::unique_ptr<juce::CodeEditorComponent> testResultsWindow;
    std::unique_ptr<juce::CodeDocument> testResultsDocument;
    int documentPositionIndex = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConsoleViewComponent)
};
