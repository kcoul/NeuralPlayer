#pragma once

#include <JuceHeader.h>

class SidePanelHeader : public juce::Component
{
public:
    SidePanelHeader(const juce::String& titleText);
    ~SidePanelHeader();
    void paint(juce::Graphics&) override;
    void resized() override;
    void setHomeButtonClicked(std::function<void()> callback);
    void setSettingButtonClicked(std::function<void()> callback);

private:
    juce::Label titleLabel;
    juce::ShapeButton homeButton { "Home",
                                   juce::Colours::transparentBlack,
                                   juce::Colours::transparentBlack,
                                   juce::Colours::transparentBlack },
        settingsButton { "Settings",
                         juce::Colours::transparentBlack,
                         juce::Colours::transparentBlack,
                         juce::Colours::transparentBlack };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SidePanelHeader)
};
