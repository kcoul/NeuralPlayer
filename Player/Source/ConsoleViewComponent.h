#pragma once

#include <JuceHeader.h>

class ConsoleViewComponent : public juce::Component,
                             public juce::Thread::Listener,
                             juce::Timer
{
public:
    //==============================================================================
    ConsoleViewComponent(String& consoleInput);
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
    String& newConsoleInputToDisplay;
    String lastDisplayedString = "";
    std::unique_ptr<juce::CodeEditorComponent> testResultsWindow;
    std::unique_ptr<juce::CodeDocument> testResultsDocument;
    int positionIndex = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConsoleViewComponent)
};
