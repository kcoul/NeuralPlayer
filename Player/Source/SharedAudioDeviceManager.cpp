#include "SharedAudioDeviceManager.h"

static juce::String getCurrentDefaultAudioDeviceName(juce::AudioDeviceManager& deviceManager, bool isInput)
{
    auto* deviceType = deviceManager.getCurrentDeviceTypeObject();
    jassert(deviceType != nullptr);

    if (deviceType != nullptr)
    {
        auto deviceNames = deviceType->getDeviceNames();
        return deviceNames[deviceType->getDefaultDeviceIndex(isInput)];
    }

    return {};
}

//Returns a shared AudioDeviceManager reference that entire test infrastructure can use
juce::AudioDeviceManager& getSharedAudioDeviceManager(int numInputChannels, int numOutputChannels)
{
    if (sharedAudioDeviceManager == nullptr)
        sharedAudioDeviceManager.reset(new juce::AudioDeviceManager());

    auto* currentDevice = sharedAudioDeviceManager->getCurrentAudioDevice();

    if (numInputChannels < 0)
        numInputChannels =
            (currentDevice != nullptr ? currentDevice->getActiveInputChannels().countNumberOfSetBits() : 1);

    if (numOutputChannels < 0)
        numOutputChannels =
            (currentDevice != nullptr ? currentDevice->getActiveOutputChannels().countNumberOfSetBits() : 2);

    if (numInputChannels > 0 && !juce::RuntimePermissions::isGranted(juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request(juce::RuntimePermissions::recordAudio,
                                          [numInputChannels, numOutputChannels](bool granted)
                                          {
                                              if (granted)
                                                  getSharedAudioDeviceManager(numInputChannels, numOutputChannels);
                                          });

        numInputChannels = 0;
    }

    if (sharedAudioDeviceManager->getCurrentAudioDevice() != nullptr)
    {
        auto setup = sharedAudioDeviceManager->getAudioDeviceSetup();

        auto numInputs = juce::jmax(numInputChannels, setup.inputChannels.countNumberOfSetBits());
        auto numOutputs = juce::jmax(numOutputChannels, setup.outputChannels.countNumberOfSetBits());

        auto oldInputs = setup.inputChannels.countNumberOfSetBits();
        auto oldOutputs = setup.outputChannels.countNumberOfSetBits();

        if (oldInputs != numInputs || oldOutputs != numOutputs)
        {
            if (oldInputs == 0 && oldOutputs == 0)
            {
                sharedAudioDeviceManager->initialise(numInputChannels, numOutputChannels, nullptr, true, {}, nullptr);
            }
            else
            {
                setup.useDefaultInputChannels = setup.useDefaultOutputChannels = false;

                setup.inputChannels.clear();
                setup.outputChannels.clear();

                setup.inputChannels.setRange(0, numInputs, true);
                setup.outputChannels.setRange(0, numOutputs, true);

                if (oldInputs == 0 && numInputs > 0 && setup.inputDeviceName.isEmpty())
                    setup.inputDeviceName = getCurrentDefaultAudioDeviceName(*sharedAudioDeviceManager, true);

                if (oldOutputs == 0 && numOutputs > 0 && setup.outputDeviceName.isEmpty())
                    setup.outputDeviceName = getCurrentDefaultAudioDeviceName(*sharedAudioDeviceManager, false);

                sharedAudioDeviceManager->setAudioDeviceSetup(setup, false);
            }
        }
    }
    else
    {
        sharedAudioDeviceManager->initialise(numInputChannels, numOutputChannels, nullptr, true, {}, nullptr);

        auto setup = sharedAudioDeviceManager->getAudioDeviceSetup();
        setup.bufferSize = 256;

#if JUCE_LINUX
            setup.inputDeviceName = "JACK Audio Connection Kit";
            setup.outputDeviceName = "JACK Audio Connection Kit";
#endif

        sharedAudioDeviceManager->setAudioDeviceSetup(setup, true);

        /*
        currentDevice = sharedAudioDeviceManager->getCurrentAudioDevice();
        auto bufferSizes = currentDevice->getAvailableBufferSizes();
        for(int bufferSize : bufferSizes)
        {
            std::cout << "Available buffer size: " << bufferSize << std::endl;
        }
         */
    }

    return *sharedAudioDeviceManager;
}
