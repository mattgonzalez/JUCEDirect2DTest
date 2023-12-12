#include <JuceHeader.h>
#include "RenderTestComponent.h"
#include "ComparisonComponent.h"

class RendererComparisonApplication  : public juce::JUCEApplication
{
public:
    RendererComparisonApplication() {}

    const juce::String getApplicationName() override       { return ProjectInfo::projectName; }
    const juce::String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override             { return true; }

    void initialise (const juce::String&) override
    {
        int64_t seed = 1234567890;

        softwareRendererWindow = std::make_unique<TestWindow>("Software Renderer", 0, seed);
        direct2DRendererWindow = std::make_unique<TestWindow>("Direct2D Renderer", 1, seed);
         comparisonWindow = std::make_unique<ComparisonWindow>();

        auto userArea = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay()->userArea;

        auto windowArea = userArea.removeFromLeft(userArea.getWidth() / 3);
        auto border = softwareRendererWindow->getPeer()->getFrameSize();
        windowArea = border.subtractedFrom(windowArea);
        softwareRendererWindow->setBounds(windowArea);
        direct2DRendererWindow->setBounds(windowArea.withX(windowArea.getRight()));
        comparisonWindow->setBounds(windowArea.withX(direct2DRendererWindow->getRight()));

        juce::Timer::callAfterDelay(500, [this]
            {
                //comparisonWindow->softwareRendererSnapshot = softwareRendererWindow->createComponentSnapshot(softwareRendererWindow->getLocalBounds());
                //comparisonWindow->direct2DRendererSnapshot = direct2DRendererWindow->createComponentSnapshot(direct2DRendererWindow->getLocalBounds());
                comparisonWindow->softwareRendererSnapshot = juce::createSnapshotOfNativeWindow(softwareRendererWindow->getPeer()->getNativeHandle());
                comparisonWindow->direct2DRendererSnapshot = juce::createSnapshotOfNativeWindow(direct2DRendererWindow->getPeer()->getNativeHandle());

                comparisonWindow->repaint();
            });
    }

    void shutdown() override
    {
        softwareRendererWindow = nullptr;
        direct2DRendererWindow = nullptr;
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted (const juce::String&) override
    {
    }

    class TestWindow : public juce::DocumentWindow
    {
    public:
        TestWindow (juce::String name, int renderer, int64_t seed)
            : DocumentWindow (name,
                              juce::Desktop::getInstance().getDefaultLookAndFeel()
                                                          .findColour (juce::ResizableWindow::backgroundColourId),
                              DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new RenderTestComponent(seed), true);

            setResizable (true, true);

            getPeer()->setCurrentRenderingEngine(renderer);

            setVisible (true);
        }

        void closeButtonPressed() override
        {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TestWindow)
    };

    class ComparisonWindow : public juce::DocumentWindow
    {
    public:
        ComparisonWindow()
            : DocumentWindow("Comparison",
                juce::Desktop::getInstance().getDefaultLookAndFeel()
                .findColour(juce::ResizableWindow::backgroundColourId),
                DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar(true);
            setContentOwned(new ComparisonComponent{ softwareRendererSnapshot, direct2DRendererSnapshot }, true);

            setResizable(true, true);

            getPeer()->setCurrentRenderingEngine(0);

            setVisible(true);
        }

        void closeButtonPressed() override
        {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

        juce::Image softwareRendererSnapshot;
        juce::Image direct2DRendererSnapshot;

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ComparisonWindow)
    };

private:
    std::unique_ptr<TestWindow> softwareRendererWindow;
    std::unique_ptr<TestWindow> direct2DRendererWindow;
    std::unique_ptr<ComparisonWindow> comparisonWindow;
};

START_JUCE_APPLICATION (RendererComparisonApplication)
