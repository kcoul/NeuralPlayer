#pragma once

#include <JuceHeader.h>

extern std::unique_ptr<juce::AudioDeviceManager> sharedAudioDeviceManager;
juce::AudioDeviceManager& getSharedAudioDeviceManager(int numInputChannels = -1, int numOutputChannels = -1);
