/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

  name:             Many Components

  dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics, juce_gui_basics
  exporters:        VS2022

  moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1
  defines:

  type:             Component
  mainClass:        ManyComponents

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "StatTable.h"

class ManyComponents : public juce::Component, public juce::LookAndFeel_V4
{
public:
    ManyComponents()
    {
        setLookAndFeel(this);

        for (int i = 0; i < 2000; ++i)
        {
            auto button = std::make_unique<AnimatedButton>();
            addAndMakeVisible(button.get());
            buttons.add(std::move(button));
        }

        setSize(1024, 1024);
    }

    ~ManyComponents() override
    {
        setLookAndFeel(nullptr);
    }

    void resized() override
    {
        int buttonsPerRow = roundToInt(std::sqrt((float)buttons.size()));
        int numRows = buttons.size() / buttonsPerRow;
        int width = getWidth() / buttonsPerRow;
        int height = getHeight() / numRows;

        juce::Rectangle<int> r{ 0, 0, width, height };
        for (auto button : buttons)
        {
            button->setBounds(r);
            r.translate(width, 0);
            if (r.getX() >= getWidth())
            {
                r.setX(0);
                r.translate(0, height);
            }
        }

        statTable.setTopLeftPosition(getScreenPosition().translated(getWidth() - statTable.getWidth() - 5, getHeight() - statTable.getHeight() - 5));
    }

    void paint(juce::Graphics& g) override
    {
        //g.fillAll(juce::Colours::black);
    }

    struct AnimatedButton : public juce::Button
    {
        AnimatedButton() : juce::Button("AnimatedButton") {}
        ~AnimatedButton() override = default;

        void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
        {
            ColourGradient gradient{ juce::Colour{ (uint32)getX() }, 0.0f, 0.0f, juce::Colour{ (uint32)getX() }.withAlpha(0.0f), 0.0f, (float)getHeight(), false };
            //g.setColour(juce::Colour{ (uint32)getX() }.withAlpha(1.0f));
            g.fillRect(pos, 0.0f, (float)getWidth(), (float)getHeight());
            pos += 10.0f;
            if (pos >= (float)getWidth())
                pos = 0.0f;
        }

        float pos = 0.0f;
    };

    void parentHierarchyChanged() override
    {
        if (auto peer = getPeer())
        {
            statTable.addToDesktop(0, nullptr);
            statTable.setAlwaysOnTop(true);
        }
        else
        {
            statTable.removeFromDesktop();
        }
    }

    juce::VBlankAttachment attachment{ this, [this] { repaint(); } };

    juce::OwnedArray<AnimatedButton> buttons;
    StatTable statTable{ this };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ManyComponents)
};

