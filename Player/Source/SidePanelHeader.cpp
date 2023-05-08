#include "SidePanelHeader.h"

SidePanelHeader::SidePanelHeader(const juce::String& titleText)
{
    setOpaque(true);

    juce::Path p;
    p.loadPathFromData(homeIconPathData, sizeof(homeIconPathData));
    homeButton.setShape(p, true, true, false);

    titleLabel.setFont(juce::Font(16.0f));
    titleLabel.setText(titleText, juce::NotificationType::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(titleLabel);
    addAndMakeVisible(homeButton);
}

SidePanelHeader::~SidePanelHeader() {}

void SidePanelHeader::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::SidePanel::backgroundColour)); // clear the background

    auto normal = getLookAndFeel().findColour(juce::SidePanel::dismissButtonNormalColour);
    auto over = getLookAndFeel().findColour(juce::SidePanel::dismissButtonOverColour);
    auto down = getLookAndFeel().findColour(juce::SidePanel::dismissButtonDownColour);

    homeButton.setColours(normal, over, down);
}

void SidePanelHeader::resized()
{
    auto bounds = getLocalBounds();

    auto cid1 = homeButton.getComponentID();

    bounds.removeFromLeft(10);
    homeButton.setBounds(bounds.removeFromLeft(homeButton.getWidth() + 20).reduced(7));

    bounds.removeFromRight(50); //Right pad to center title

    titleLabel.setBounds(bounds);
}

void SidePanelHeader::setHomeButtonClicked(std::function<void()> callback) { homeButton.onClick = callback; }
