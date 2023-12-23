#include <JuceHeader.h>
#include "ImageComparator.h"

class TestWindow : public juce::DocumentWindow
{
public:
    TestWindow(juce::String name, int renderer, int64_t seed)
        : DocumentWindow(name,
            juce::Colours::black,
            juce::DocumentWindow::allButtons)
    {
        setUsingNativeTitleBar(true);
        setContentOwned(new RenderTestComponent(seed), true);

        setResizable(true, true);

        getPeer()->setCurrentRenderingEngine(renderer);

        setVisible(true);
    }

    void closeButtonPressed() override
    {
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TestWindow)
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
        setContentNonOwned(&comparisonComponent, false);

        setResizable(true, true);

        getPeer()->setCurrentRenderingEngine(1);

        setVisible(true);

    }

    void closeButtonPressed() override
    {
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }

    ImageComparator comparisonComponent;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ComparisonWindow)
};

class RendererComparisonApplication  : public juce::JUCEApplication
{
public:
    RendererComparisonApplication() {}

    const juce::String getApplicationName() override       { return ProjectInfo::projectName; }
    const juce::String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override             { return true; }

    void initialise (const juce::String&) override
    {
//         softwareRendererWindow = std::make_unique<TestWindow>("Software Renderer", 0, seed);
//         direct2DRendererWindow = std::make_unique<TestWindow>("Direct2D Renderer", 1, seed);
         comparisonWindow = std::make_unique<ComparisonWindow>();

        auto userArea = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay()->userArea;

        auto border = comparisonWindow->getPeer()->getFrameSize();
        auto windowArea = border.subtractedFrom(userArea);
//         softwareRendererWindow->setBounds(windowArea);
//         direct2DRendererWindow->setBounds(windowArea.withX(windowArea.getRight()));
//         comparisonWindow->setBounds(windowArea.withX(direct2DRendererWindow->getRight()));
        comparisonWindow->setBounds(windowArea);

#if 1
        //auto image1 = juce::ImageCache::getFromFile(juce::File{ "C:/temp/demorunner d2d.PNG" });
        auto image1 = juce::ImageCache::getFromFile(juce::File{ "C:/temp/edited.PNG" });
        auto image2 = juce::ImageCache::getFromFile(juce::File{ "C:/temp/demorunner gdi.PNG" });
        comparisonWindow->comparisonComponent.compare(image1, image2);
#endif


#if 0
        //juce::Timer::callAfterDelay(500, [=]
            {
                auto imageBounds = juce::Rectangle<int>{ 100, 100 };
                juce::SoftwareImageType softwareImageType;
                juce::Image softwareImage{ softwareImageType.create(juce::Image::ARGB, imageBounds.getWidth(), imageBounds.getHeight(), true) };
                {
                    juce::Graphics g{ softwareImage };
                    //comparisonWindow->comparisonComponent.testComponent.paintEntireComponent(g, false);
                    g.setColour(juce::Colours::white);
                    g.setFont(20.0f);
                    g.drawText("This is text", imageBounds, juce::Justification::centred, false);
                }

                juce::NativeImageType nativeImageType;
                juce::Image nativeImage{ nativeImageType.create(juce::Image::ARGB, imageBounds.getWidth(), imageBounds.getHeight(), true) };
                {
                    juce::Graphics g{ nativeImage };
                    //comparisonWindow->comparisonComponent.testComponent.paintEntireComponent(g, false);
                    g.setColour(juce::Colours::white);
                    g.setFont(20.0f);
                    g.drawText("This is text", imageBounds, juce::Justification::centred, false);
                }

                comparisonWindow->comparisonComponent.compare(softwareImage, nativeImage);

//                 comparisonWindow->comparisonComponent.compare(juce::createSnapshotOfNativeWindow(softwareRendererWindow->getPeer()->getNativeHandle()),
//                     juce::createSnapshotOfNativeWindow(direct2DRendererWindow->getPeer()->getNativeHandle()));
                //comparisonWindow->comparisonComponent.compare(nativeImage, juce::createSnapshotOfNativeWindow(direct2DRendererWindow->getPeer()->getNativeHandle()));
            };
#endif
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

    

private:
    std::unique_ptr<TestWindow> softwareRendererWindow;
    std::unique_ptr<TestWindow> direct2DRendererWindow;
    std::unique_ptr<ComparisonWindow> comparisonWindow;
};

START_JUCE_APPLICATION (RendererComparisonApplication)
