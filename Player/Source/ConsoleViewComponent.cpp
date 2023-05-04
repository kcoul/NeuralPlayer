#include "ConsoleViewComponent.h"

//==============================================================================
ConsoleViewComponent::ConsoleViewComponent(WatchedVars& watchedVars) : watchedVariables(watchedVars)
{
    testResultsDocument = std::make_unique<juce::CodeDocument>();
    testResultsWindow = std::make_unique<juce::CodeEditorComponent>(*testResultsDocument, nullptr);
    addAndMakeVisible(*testResultsWindow);

    testResultsWindow->setFont(juce::Font("Courier", "Regular", 15.0f));
    testResultsWindow->setReadOnly(true);
    testResultsWindow->setEnabled(false);
}

ConsoleViewComponent::~ConsoleViewComponent()
{
    testResultsWindow.reset();
}

//==============================================================================
void ConsoleViewComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void ConsoleViewComponent::resized()
{
    auto bounds = getLocalBounds();
    if (testResultsWindow)
        testResultsWindow->setBounds(bounds);
}

void ConsoleViewComponent::clear()
{
    testResultsDocument->replaceAllContent("");
}


void ConsoleViewComponent::insertText(juce::String text, bool addCarriageReturn)
{
    if (addCarriageReturn)
    {
        testResultsDocument->insertText(documentPositionIndex, text + "\n");
        documentPositionIndex += (text.length() + 1);
    }
    else
    {
        testResultsDocument->insertText(documentPositionIndex, text);
        documentPositionIndex += (text.length());
    }

    testResultsWindow->moveCaretToEnd(false);
    testResultsWindow->scrollToKeepCaretOnScreen();
}

void ConsoleViewComponent::visibilityChanged()
{
    if (isVisible())
        startTimer (30);
    else
        stopTimer();
}

void ConsoleViewComponent::timerCallback()
{
    if (watchedVariables.currentFileIndex > 0 &&
        processingIndex != watchedVariables.currentFileIndex)
    {
        processingIndex = watchedVariables.currentFileIndex;
        auto textToPost = "Now processing file " + String(processingIndex) + " of " +
                                 String(watchedVariables.numFiles) + ", " +
                                 watchedVariables.currentFileName;
        insertText(textToPost, true);
    }
    else if (watchedVariables.currentFileIndex == 0 &&
             processingIndex != watchedVariables.currentFileIndex)
    {
        processingIndex = watchedVariables.currentFileIndex;
        auto textToPost = "Done!";
        insertText(textToPost, true);
    }

    if (lastDisplayedString != watchedVariables.returnedText)
    {
        lastDisplayedString = watchedVariables.returnedText;
        insertText(lastDisplayedString, true);
    }
}

void ConsoleViewComponent::exitSignalSent()
{

}
