#include <juce_gui_basics/juce_gui_basics.h>

#include "app/MainComponent.h"
#include "ui/look/Tokens.h"

namespace
{
    class MainWindow : public juce::DocumentWindow
    {
    public:
        MainWindow()
            : DocumentWindow("Turbito DJ",
                             juce::Colour(tokens::bg),
                             juce::DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar(true);
            setContentOwned(new MainComponent(), true);
            setResizable(true, true);
            setResizeLimits(tokens::windowMinimum.width, tokens::windowMinimum.height,
                            7680, 4320);
            centreWithSize(tokens::windowReference.width, tokens::windowReference.height);
            setVisible(true);
        }

        void closeButtonPressed() override
        {
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
    };

    class TurbitoApplication : public juce::JUCEApplication
    {
    public:
        const juce::String getApplicationName() override { return JUCE_APPLICATION_NAME_STRING; }
        const juce::String getApplicationVersion() override { return JUCE_APPLICATION_VERSION_STRING; }
        bool moreThanOneInstanceAllowed() override { return true; }

        void initialise(const juce::String&) override
        {
            mainWindow = std::make_unique<MainWindow>();
        }

        void shutdown() override
        {
            mainWindow.reset();
        }

        void systemRequestedQuit() override
        {
            quit();
        }

        void anotherInstanceStarted(const juce::String&) override {}

    private:
        std::unique_ptr<MainWindow> mainWindow;
    };
}

START_JUCE_APPLICATION(TurbitoApplication)
