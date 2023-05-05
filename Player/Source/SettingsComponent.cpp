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




    addAndMakeVisible(*audioSettings);

    consoleViewComponent = std::make_unique<ConsoleViewComponent>(consoleVars);
    addAndMakeVisible(*consoleViewComponent);
}

SettingsComponent::~SettingsComponent()
{

}

void SettingsComponent::paint(juce::Graphics& g)
{

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

    area.removeFromTop(vUnit);

    if(consoleViewComponent)
        consoleViewComponent->setBounds(area);
}