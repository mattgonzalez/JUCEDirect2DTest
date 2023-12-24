#include "MainComponent.h"
#include "RenderTest.h"

RenderTest::RenderTest(DemoContentComponent* component_) :
    component(component_)
{
    Timer::callAfterDelay(100, [this]() { selectTest(); });
}

void RenderTest::selectTest()
{
    if (testIndex < tests.size())
    {
        auto const& test = tests[testIndex];
        component->setDemo(test.category.data(), test.demoIndex);

        Timer::callAfterDelay(100, [this]() { takeScreenshots(); 
            testIndex++;
            selectTest(); });
    }
}

void RenderTest::takeScreenshots()
{
    auto const& test = tests[testIndex];

    File outputFolder = File::getSpecialLocation(File::userDocumentsDirectory).getChildFile("JUCEDemoTest").getChildFile(subdirName);
    outputFolder.createDirectory();

    auto screenshot = [&](ImageType&& type, StringRef filename)
        {
            auto image = Image{ type.create(Image::ARGB, component->getWidth(), component->getHeight(), true) };
            {
                Graphics g{ image};
                component->paintEntireComponent(g, false);
            }

            File file{ outputFolder.getChildFile(filename).withFileExtension("png") };
            jassert(!file.existsAsFile());
            FileOutputStream stream{ file };
            pngFormat.writeImageToStream(image, stream);
        };

    screenshot(SoftwareImageType{}, String{ test.name.data() } + " GDI");
    screenshot(NativeImageType{}, String{ test.name.data() } + " D2D");
}
