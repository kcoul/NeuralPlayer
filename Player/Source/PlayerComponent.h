#pragma once

#include <JuceHeader.h>

#include "PlaylistComponent.h"
#include "SharedAudioDeviceManager.h"
#include "SourceSepMIDIRenderingThread.h"

enum class FolderSelectResult
{
    ok,
    cancelled
};

class PlayerComponent : public juce::Component {
public:
    PlayerComponent(std::unique_ptr<SourceSepMIDIRenderingThread>& renderingThread);
    ~PlayerComponent() override;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    juce::AudioDeviceManager& audioDeviceManager { getSharedAudioDeviceManager(0, 2) };

    juce::TextButton loadButton {"Load Audio"};

    std::unique_ptr<juce::FileChooser> loadChooser;
    std::pair<FolderSelectResult, juce::File> selectInputFolder();
    juce::File selectedOutputFolder;

    std::unique_ptr<SourceSepMIDIRenderingThread>& renderingThread;

    PlaylistComponent playlistComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlayerComponent)
};

