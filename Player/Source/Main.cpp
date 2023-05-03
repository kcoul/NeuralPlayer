#include "MainViewComponent.h"

#include <JuceHeader.h>

std::unique_ptr<juce::AudioDeviceManager> sharedAudioDeviceManager;

//==============================================================================
class PinPointTestGuiApplication : public juce::JUCEApplication
{
public:
    //==============================================================================
    PinPointTestGuiApplication() {}

    ~PinPointTestGuiApplication() override { sharedAudioDeviceManager.reset(); }

    const juce::String getApplicationName() override { return ProjectInfo::projectName; }
    const juce::String getApplicationVersion() override { return ProjectInfo::versionString; }

    bool moreThanOneInstanceAllowed() override { return true; }

    //==============================================================================
    void initialise(const juce::String&) override
    {
        mainWindow.reset(new MainWindow(getApplicationName()));
    }

    void shutdown() override { mainWindow = nullptr; }

    //==============================================================================
    void systemRequestedQuit() override { quit(); }

    void anotherInstanceStarted(const juce::String&) override {}

    //==============================================================================
    class MainWindow : public juce::DocumentWindow
    {
    public:
        MainWindow(juce::String name)
            : DocumentWindow(name,
                             juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(
                                 juce::ResizableWindow::backgroundColourId),
                             DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar(true);
            setContentOwned(new MainViewComponent(), true);

#if JUCE_IOS || JUCE_ANDROID
            setFullScreen(true);
#else
            setResizable(true, true);
            setResizeLimits(540, 800, 540, 800);
            centreWithSize(getWidth(), getHeight());
#endif

            setVisible(true);
        }

        void closeButtonPressed() override { JUCEApplication::getInstance()->systemRequestedQuit(); }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

//==============================================================================
START_JUCE_APPLICATION(PinPointTestGuiApplication)
