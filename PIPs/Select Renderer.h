/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

  name:             Select Renderer

  dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics, juce_gui_basics
  exporters:        VS2022

  moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1
  defines:

  type:             Component
  mainClass:        SelectRenderer

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

class SelectRenderer : public Component
{
public:
    SelectRenderer()
    {
        addAndMakeVisible(direct2DToggle);
        direct2DToggle.onClick = [this]()
            {
                if (auto peer = getPeer())
                {
                    peer->setCurrentRenderingEngine(direct2DToggle.getToggleState() ? 1 : 0);
                }
            };

        setSize(512, 512);
    }

    ~SelectRenderer() override = default;

    void resized() override
    {
        direct2DToggle.setBounds(10, 10, 250, 25);
    }

    void paint(Graphics& g) override
    {
        g.fillAll(Colour::greyLevel(0.1f));

        if (auto peer = getPeer())
        {
            int engine = peer->getCurrentRenderingEngine();

            g.setColour(Colours::white);
            g.setFont(50.0f);
            g.drawText(peer->getAvailableRenderingEngines()[engine], getLocalBounds(), juce::Justification::centred);
        }
    }

    void parentHierarchyChanged() override
    {
        //
        // parentHierarchyChanged will be called when the parent desktop window
        // component peer is created or destroyed
        //
        // Check to see if the peer exists and set the parent window to software
        // rendering mode
        //
        if (auto peer = getPeer())
        {
            peer->setCurrentRenderingEngine(0);
        }
    }

private:
    juce::ToggleButton direct2DToggle{ "Direct2D" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SelectRenderer)
};

