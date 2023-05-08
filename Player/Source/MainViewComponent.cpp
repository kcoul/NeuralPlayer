#include "MainViewComponent.h"

MainViewComponent::MainViewComponent() : tabComponent(juce::TabbedButtonBar::TabsAtTop)
{
    playerComponent = std::make_unique<PlayerComponent>();

    lumiCallback = [this] (roli::Block::Ptr p){ playerComponent->setLumi(p); };

    settingsComponent = std::make_unique<SettingsComponent>(lumiCallback);

    setSize(700, 800);

    tabComponent.addTab("Player", getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId),
                        playerComponent.get(), false);
    tabComponent.addTab("Settings", getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId),
                        settingsComponent.get(), false);
    addAndMakeVisible(tabComponent);
}

MainViewComponent::~MainViewComponent()
{
}

void MainViewComponent::paint(juce::Graphics& g)
{

}

void MainViewComponent::resized()
{
    auto area = getLocalBounds().reduced(4);
#if JUCE_IOS
    area.removeFromTop(30);
#endif

    tabComponent.setBounds(area);
}


