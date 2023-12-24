#pragma once

#include <JuceHeader.h>

class RenderTest
{
public:
    RenderTest(DemoContentComponent* component_);

private:
    DemoContentComponent* component;
    PNGImageFormat pngFormat;
    String subdirName = File::createLegalFileName(Time::getCurrentTime().toISO8601(false));

    void selectTest();
    void takeScreenshots();

    struct Test
    {
        std::string_view category;
        int demoIndex;
        std::string_view name;
    };

    size_t testIndex = 0;
    std::array<Test, 3> tests
    {
        Test
        {
            "GUI", 10, "Fonts"
        },

        {
            "GUI", 11, "Graphics"
        },

        {
            "GUI", 25, "Widgets"
        }
    };
};


