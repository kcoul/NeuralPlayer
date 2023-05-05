#include "SettingsComponent.h"

SettingsComponent::SettingsComponent(WatchedVars& consoleVars, std::function<void(roli::Block::Ptr)>& lumiCallbackFn) :
lumiCallback(lumiCallbackFn)
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

    // Register to receive topologyChanged() callbacks from pts.
    pts.addListener (this);
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

void SettingsComponent::topologyChanged()
{
    // We have a new topology, so find out what it is and store it in a local
    // variable.
    auto currentTopology = pts.getCurrentTopology();
    consoleViewComponent->insertText("New BLOCKS topology.", true);

    // The blocks member of a BlockTopology contains an array of blocks. Here we
    // loop over them and print some information.
    consoleViewComponent->insertText("Detected " +
                                     String (currentTopology.blocks.size()) +
                                     " blocks:", true);

    for (auto& block : currentTopology.blocks)
    {
        consoleViewComponent->insertText("    Description:   " + block->getDeviceDescription(), true);
        consoleViewComponent->insertText("    Battery level: " + String (block->getBatteryLevel()), true);
        consoleViewComponent->insertText("    UID:           " + String (block->uid), true);
        consoleViewComponent->insertText("    Serial number: " + block->serialNumber, true);
        consoleViewComponent->insertText("    TypeID:        " + String (block->getType()), true);
        switch (block->getType())
        {
            case roli::Block::unknown:
                consoleViewComponent->insertText("    Type:          unknown", true);
                break;
            case roli::Block::lightPadBlock:
                consoleViewComponent->insertText("    Type:          lightPadBlock", true);
                break;
            case roli::Block::liveBlock:
                consoleViewComponent->insertText("    Type:          liveBlock", true);
                break;
            case roli::Block::loopBlock:
                consoleViewComponent->insertText("    Type:          loopBlock", true);
                break;
            case roli::Block::developerControlBlock:
                consoleViewComponent->insertText("    Type:          developerControlBlock", true);
                break;
            case roli::Block::touchBlock:
                consoleViewComponent->insertText("    Type:          touchBlock", true);
                break;
            case roli::Block::seaboardBlock:
                consoleViewComponent->insertText("    Type:          seaboardBlock", true);
                break;
            case roli::Block::lumiKeysBlock:
                consoleViewComponent->insertText("    Type:          lumiKeysBlock", true);
                lumiCallback(block);
                //lumiDetectedButton.setToggleState(true, juce::dontSendNotification);
                break;
        };
    }
}