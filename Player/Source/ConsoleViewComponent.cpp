#include "ConsoleViewComponent.h"

//==============================================================================
ConsoleViewComponent::ConsoleViewComponent()
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

}


