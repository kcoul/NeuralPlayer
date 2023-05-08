#pragma once

#include <JuceHeader.h>
#include "DataStructures.h"

class ConsoleViewComponent : public juce::Component
{
public:
    //==============================================================================
    ConsoleViewComponent();
    ~ConsoleViewComponent() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

    void insertText(juce::String text, bool addCarriageReturn);
    void clear();
protected:
    void visibilityChanged() override;
private:
    //==============================================================================
    std::unique_ptr<juce::CodeEditorComponent> testResultsWindow;
    std::unique_ptr<juce::CodeDocument> testResultsDocument;
    int documentPositionIndex = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConsoleViewComponent)
};
