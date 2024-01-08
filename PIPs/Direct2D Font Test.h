/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

  name:             Direct2D Font Test

  dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics, juce_gui_basics
  exporters:        VS2022

  moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1
  defines:

  type:             Component
  mainClass:        FontTest

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

class FontTest : public juce::Component, public juce::LookAndFeel_V4
{
public:
    FontTest()
    {
        setLookAndFeel(this);

        buttons.add(new TextButton{ "Short" });
        buttons.add(new TextButton{ "Longer Text" });
        buttons.add(new TextButton{ "Button With Very Long Text" });
        for (auto button : buttons)
        {
            addAndMakeVisible(button);
        }

        direct2DToggle.setToggleState(true, juce::dontSendNotification);
        direct2DToggle.onClick = [this]()
            {
                if (auto peer = getPeer())
                {
                    peer->setCurrentRenderingEngine(direct2DToggle.getToggleState() ? 1 : 0);
                }
            };
        addAndMakeVisible(direct2DToggle);

        setSize(512, 512);
    }

    ~FontTest() override
    {
        setLookAndFeel(nullptr);
    }

    void resized() override
    {
        int x = 10, y = 10;
        for (auto button : buttons)
        {
            button->setBounds(x, y, 100, 30);
            button->changeWidthToFitText();
            x = button->getRight();
        }

        direct2DToggle.setBounds(10, getHeight() - 40, 100, 30);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black);

        juce::Rectangle<int> r{ 10, 50, 0, 100 };

        auto paintText = [&](Typeface::Ptr typeface)
            {
                auto font = juce::Font{ typeface }.withHeight(30.0f);
                auto text = font.toString();

                auto width = font.getStringWidth(text);
                r.setWidth(width);

                g.setColour(juce::Colours::white);
                g.drawText(text, r, juce::Justification::centred);

                g.setColour(juce::Colours::pink);
                g.drawRect(r);

                r.translate(0, r.getHeight());
            };

        paintText(g.getCurrentFont().getTypefacePtr());
        paintText(robotoTypeface);
        paintText(openSansTypeface);
    }

    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override
    {
        return juce::Font{ openSansTypeface }.withHeight(24.0f);
    }

    juce::Typeface::Ptr robotoTypeface = juce::Typeface::createSystemTypefaceFor(BinaryData::RobotoMedium_ttf, BinaryData::RobotoMedium_ttfSize);
    juce::Typeface::Ptr openSansTypeface = juce::Typeface::createSystemTypefaceFor(BinaryData::OpenSansRegular_ttf, BinaryData::OpenSansRegular_ttfSize);

    juce::OwnedArray<juce::TextButton> buttons;
    juce::ToggleButton direct2DToggle{ "D2D" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FontTest)
};

