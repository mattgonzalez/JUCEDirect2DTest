/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

  name:             Direct2D Image Brush

  dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics, juce_gui_basics, juce_animation
  exporters:        VS2022

  moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1
  defines:          JUCE_DIRECT2D_METRICS=1

  type:             Component
  mainClass:        ImageBrushTest

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

class ImageBrushTest : public juce::Component
{
public:
    ImageBrushTest()
    {
        segmentWidthSlider.setRange(1.0, 1000.0, 0.01);
        segmentWidthSlider.setValue(100.0, juce::dontSendNotification);
        segmentWidthSlider.onValueChange = [this]
            {
                createImageFill();
                repaint();
            };
        addAndMakeVisible(segmentWidthSlider);

        setSize(1024, 1024);
    }

    ~ImageBrushTest() override = default;

    void resized() override
    {
        segmentWidthSlider.setBounds(10, 10, 250, 30);

        createImageFill();
    }

    void paint(juce::Graphics& g) override
    {
        g.setImageResamplingQuality(juce::Graphics::ResamplingQuality::highResamplingQuality);
        g.setFillType(imageFill);
        g.fillRect(0, 0, getWidth(), getHeight());

        g.setColour(juce::Colours::black.withAlpha(0.9f));
        g.fillRect(5, 5, 280, 50);
    }

private:
    juce::Slider segmentWidthSlider{ juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight };

    juce::FillType imageFill;

    void createImageFill()
    {
        //
        // The brush should be the width specified by the slider, but the image
        // has to be rounded down to the nearest pixel
        //
        int imageWidthPixels = (int)std::floor(segmentWidthSlider.getValue());

        //
        // Find the ratio of the desired width over the actual width
        //
        double horizontalScale = segmentWidthSlider.getValue() / (double)imageWidthPixels;

        auto image = juce::Image(juce::Image::PixelFormat::RGB, imageWidthPixels, imageWidthPixels, true);
        {
            //
            // Draw the vertical lines on the image; scale the width of the lines by the inverse of the horizontal scale
            // 
            juce::Graphics g{ image };
            g.setColour(juce::Colours::white);
            juce::Rectangle<float> r{ 0.0f, 0.0f, (float)(1.0 / horizontalScale), (float)image.getHeight() };
            g.fillRect(r);

            g.setColour(juce::Colours::darkgrey);
            r.translate((float)image.getWidth() * 0.25f, 0.0f);
            g.fillRect(r);
            r.translate((float)image.getWidth() * 0.25f, 0.0f);
            g.fillRect(r);
            r.translate((float)image.getWidth() * 0.25f, 0.0f);
            g.fillRect(r);
        }
        
        //
        // Set the fill with a scaling transform so the vertical lines appear to be 1 pixel wide
        //
        imageFill = juce::FillType{ image, juce::AffineTransform::scale((float)horizontalScale, 1.0f) };
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ImageBrushTest)
};

