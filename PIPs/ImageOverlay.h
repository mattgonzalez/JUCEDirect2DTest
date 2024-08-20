/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

  name:             ImageOverlay

  dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics, juce_gui_basics
  exporters:        VS2022, xcode_mac

  moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1
  defines:

  type:             Component
  mainClass:        ImageOverlay

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

class ImageOverlay : public juce::Component
{
public:
    ImageOverlay()
    {
        setSize(768, 768);
    }

    ~ImageOverlay() override = default;

    void resized() override
    {
        //
        // Create images; explicitly specify the image type
        //
        softwareImage = juce::Image{ juce::Image::ARGB, getWidth(), getHeight(), true, juce::SoftwareImageType{} };
        previousImage = juce::Image{ juce::Image::ARGB, getWidth(), getHeight(), true, juce::NativeImageType{} };
        composite = juce::Image{ juce::Image::ARGB, getWidth(), getHeight(), true, juce::NativeImageType{} };
    }

    void paint(juce::Graphics& g) override
    {
        g.drawImageAt(composite, 0, 0);
    }

    void animate()
    {
        auto now = juce::Time::getMillisecondCounterHiRes();
        auto elapsedMsec = now - lastMsec;
        if (elapsedMsec < 25.0)
        {
            return;
        }
        lastMsec = now;

        //
        // Generate dots
        //
        {
            juce::Image::BitmapData bitmapData{ softwareImage, juce::Image::BitmapData::writeOnly };

            radius += (float)bitmapData.width * 0.005f;
            if (radius >= (float)bitmapData.width * 0.707f)
                radius = 0.0f;

            hue += 0.01f;
            if (hue >= 1.0f)
                hue = 0.0f;

            //
            // Clear the software image
            //
            memset(bitmapData.data, 0, bitmapData.lineStride * softwareImage.getHeight());

            auto bounds = softwareImage.getBounds();
            auto center = bounds.toFloat().getCentre();
            auto color = juce::Colour::fromHSV(hue, 1.0f, 1.0f, 1.0f);
            for (float angle = 0.0f; angle < juce::MathConstants<float>::twoPi; angle += juce::MathConstants<float>::twoPi * 0.001f)
            {
                auto p = center.getPointOnCircumference(radius, angle).toInt();
                if (bounds.contains(p))
                {
                    bitmapData.setPixelColour(p.x, p.y, color);
                }
            }
        }

        //
        // Swap the composite image with the previous image
        //
        std::swap(composite, previousImage);

        //
        // Layer the new dots on top of the previous image onto the composite image
        //
        {
            juce::Graphics g{ composite };

            //
            // Clear the composite image
            //
            g.setColour(juce::Colours::transparentBlack);
            g.getInternalContext().fillRect(g.getClipBounds(), true);

            //
            // Paint the previous image onto the composite image with partial transparency
            //
            g.setColour(juce::Colours::white);
            g.beginTransparencyLayer(0.99f);
            g.drawImageAt(previousImage, 0, 0);
            g.endTransparencyLayer();

            //
            // Paint the software image over the previous image
            // 
            g.drawImageAt(softwareImage, 0, 0, false);
        }

        repaint();
    }

private:
    juce::VBlankAttachment attachment{ this, [this]() { animate(); } };
    double lastMsec = juce::Time::getMillisecondCounterHiRes();

    juce::Random random;
    juce::Image softwareImage, previousImage, composite;
    float radius = 0.0f;
    float hue = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImageOverlay)
};

