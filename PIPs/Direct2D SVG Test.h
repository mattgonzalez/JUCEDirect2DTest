/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

  name:             Direct2D SVG Test

  dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics, juce_gui_basics
  exporters:        VS2022

  moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1
  defines:          JUCE_DIRECT2D_METRICS=1

  type:             Component
  mainClass:        SVGTest

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

class SVGTest : public juce::Component, public juce::FileDragAndDropTarget
{
public:
    SVGTest()
    {
        addAndMakeVisible(transformScaleLabel);

        yScaleSlider.setRange({ 0.01, 1000.0 }, 0.001);
        yScaleSlider.setSkewFactor(0.4);
        yScaleSlider.setValue(1.0, juce::dontSendNotification);
        addAndMakeVisible(yScaleSlider);
        yScaleSlider.onValueChange = [this] { repaint(); };

        xScaleSlider.setRange({ 0.01, 1000.0 }, 0.001);
        xScaleSlider.setSkewFactor(0.4);
        xScaleSlider.setValue(1.0, juce::dontSendNotification);
        addAndMakeVisible(xScaleSlider);
        xScaleSlider.onValueChange = [this] { repaint(); };

        setSize(1024, 1024);
    }

    ~SVGTest() override = default;

    void resized() override
    {
        transformScaleLabel.setBounds(0, getHeight() - 30, 50, 30);
        yScaleSlider.setBounds(0, 0, 50, transformScaleLabel.getY());
        xScaleSlider.setBounds(transformScaleLabel.getRight(), transformScaleLabel.getY(), getWidth() - transformScaleLabel.getRight(), transformScaleLabel.getHeight());

        brushImage = Image{ Image::ARGB, 200, 200, true };
        Graphics g{ brushImage };
        g.fillCheckerBoard(brushImage.getBounds().toFloat(), 100.0f, 100.0f, Colour{ 0xff111111 }, Colour{ 0xff222222 });
    }

    void paint(juce::Graphics& g) override
    {
        //
        // Using an Image brush is much faster than drawing the checkerboard
        //
        g.setTiledImageFill(brushImage, 0, 0, 1.0f);
        g.fillAll();

        if (svg)
        {
            auto drawableBounds = svg->getDrawableBounds();
            auto transformedR = Rectangle<float>{ drawableBounds.getWidth() * (float)xScaleSlider.getValue(), drawableBounds.getHeight() * (float)yScaleSlider.getValue() };
            transformedR.setCentre(getLocalBounds().getCentre().toFloat());

            auto transform = juce::AffineTransform::translation(transformedR.getCentre() - drawableBounds.getCentre());
            transform = transform.scaled((float)xScaleSlider.getValue(), (float)yScaleSlider.getValue(), transformedR.getCentreX(), transformedR.getCentreY());

            g.setColour(juce::Colours::cyan);
            g.fillPath(path, transform);
        }
    }

    void animate()
    {
        repaint();
    }

    void parentHierarchyChanged() override
    {
        if (auto peer = getPeer())
        {
            peer->setCurrentRenderingEngine(1);
        }
    }


    bool isInterestedInFileDrag(const StringArray& files) override
    {
        for (auto const& filename : files)
        {
            if (filename.endsWith(".svg"))
            {
                return true;
            }
        }

        return false;
    }


    void filesDropped(const StringArray& files, int, int) override
    {
        juce::File file = files[0];
        svg = juce::Drawable::createFromSVGFile(file);
        if (svg)
        {
            path = svg->getOutlineAsPath();
        }
        repaint();
    }

private:
    juce::ComboBox modeCombo;
    juce::Label transformScaleLabel{ {}, "Scale" };
    juce::Slider xScaleSlider{ juce::Slider::LinearHorizontal, juce::Slider::TextBoxLeft };
    juce::Slider yScaleSlider{ juce::Slider::LinearVertical, juce::Slider::TextBoxBelow };

    Image brushImage;
    std::unique_ptr<Drawable> svg;
    juce::Path path;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SVGTest)
};

