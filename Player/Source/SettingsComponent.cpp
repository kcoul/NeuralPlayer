#include "SettingsComponent.h"

SettingsComponent::SettingsComponent(std::function<void(roli::Block::Ptr)>& lumiCallbackFn) :
lumiCallback(lumiCallbackFn)
{
    audioMIDISettings = std::make_unique<juce::AudioDeviceSelectorComponent>(audioDeviceManager,
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
    //audioMIDISettings->setLookAndFeel (&settingsLaf);

    addAndMakeVisible(*audioMIDISettings);

    lumiDetectedButton.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(lumiDetectedButton);

    xmlWriteTestButton.setButtonText("Write debug XML");
    xmlWriteTestButton.onClick = [this]{ xmlWriteTest(); };
    addAndMakeVisible(xmlWriteTestButton);

    consoleViewLabel.setJustificationType(Justification::centred);
    addAndMakeVisible(consoleViewLabel);

    consoleViewComponent = std::make_unique<ConsoleViewComponent>();
    addAndMakeVisible(*consoleViewComponent);

    // Register to receive topologyChanged() callbacks from pts.
    pts.addListener (this);
}

SettingsComponent::~SettingsComponent()
{
    //audioMIDISettings->setLookAndFeel (nullptr);
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

    auto vUnit = area.getHeight()/12;
    auto halfVUnit = vUnit/2;

    consoleViewLabel.setBounds(area.removeFromTop(halfVUnit));

    if(consoleViewComponent)
        consoleViewComponent->setBounds(area.removeFromTop(vUnit * 7));

    lumiDetectedButton.setBounds(area.removeFromTop(halfVUnit * 3));
    //xmlWriteTestButton.setBounds(area.removeFromTop(halfVUnit * 3));

    if(audioMIDISettings)
        audioMIDISettings->setBounds(area.removeFromTop(vUnit * 3));


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
                lumiDetectedButton.setToggleState(true, juce::dontSendNotification);
                break;
        };
        consoleViewComponent->insertText("", true);
    }
}

void SettingsComponent::xmlWriteTest()
{
    XmlElement playlist ("TABLEDATA");
    XmlElement* headers = new XmlElement("HEADERS");

    for (int i = 1; i <= 3; ++i)
    {
        XmlElement* column = new XmlElement("COLUMN");

        column->setAttribute("columnId", i);
        if (i == 1)
        {
            column->setAttribute("name", "Title");
            column->setAttribute("width", 320);
        }
        else if (i == 2)
        {
            column->setAttribute("name", "Author");
            column->setAttribute("width", 100);
        }
        else if (i == 3)
        {
            column->setAttribute("name", "Duration");
            column->setAttribute("width", 100);
        }

        headers->addChildElement(column);
    }
    playlist.addChildElement(headers);

    XmlElement* data = new XmlElement("DATA");

    for (int i = 1; i <= 2; ++i)
    {
        XmlElement* item = new XmlElement("ITEM");
        if (i == 1)
        {
            item->setAttribute("Title", "TrueSurvivor");
            item->setAttribute("Author", "Super MadNES");
            item->setAttribute("Duration", "02:57");
        }
        else if (i == 2)
        {
            item->setAttribute("Title", "Yee");
            item->setAttribute("Author", "");
            item->setAttribute("Duration", "00:09");
        }

        data->addChildElement(item);
    }
    playlist.addChildElement(data);

    playlist.writeTo(
            juce::File::getSpecialLocation(juce::File::userDesktopDirectory)
                    .getChildFile("Playlist.xml"), XmlElement::TextFormat());
}