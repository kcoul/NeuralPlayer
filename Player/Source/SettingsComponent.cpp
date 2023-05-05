#include "SettingsComponent.h"

SettingsComponent::SettingsComponent(WatchedVars& consoleVars)
{
    audioSettings = std::make_unique<juce::AudioDeviceSelectorComponent>(audioDeviceManager,
                                                                         0,
                                                                         0,
                                                                         2,
                                                                         2,
                                                                         false,
                                                                         true,
                                                                         true,
                                                                         false);

    //settingsLaf.setColour (Label::textColourId, Colours::white);
    //settingsLaf.setColour (TextButton::buttonColourId, Colours::white);
    //settingsLaf.setColour (TextButton::textColourOffId, Colours::black);
    //audioSettings->setLookAndFeel (&settingsLaf);
    addAndMakeVisible(*audioSettings);

    consoleViewComponent = std::make_unique<ConsoleViewComponent>(consoleVars);
    addAndMakeVisible(*consoleViewComponent);
}

SettingsComponent::~SettingsComponent()
{
    //audioSettings->setLookAndFeel (nullptr);
}

void SettingsComponent::paint(juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void SettingsComponent::resized()
{
    auto area = getLocalBounds().reduced(4);
#if JUCE_IOS
    area.removeFromTop(30);
#endif

    auto vUnit = area.getHeight()/4;

    if(audioSettings)
        audioSettings->setBounds(area.removeFromTop(vUnit));

    if(consoleViewComponent)
        consoleViewComponent->setBounds(area);
}