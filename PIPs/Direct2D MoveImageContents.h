/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

  name:             Direct2D Image Draw Test

  dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics, juce_gui_basics
  exporters:        VS2022

  moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1
  defines:

  type:             Component
  mainClass:        ImageDrawTest

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

class ImageDrawTest : public juce::Component, public juce::ImagePixelData::Listener
{
public:
    ImageDrawTest()
    {
        addAndMakeVisible(direct2DToggle);
        direct2DToggle.onClick = [this]()
            {
                if (auto peer = getPeer())
                {
                    peer->setCurrentRenderingEngine(direct2DToggle.getToggleState() ? 1 : 0);
                }
            };

        imagePermanenceCombo.addItem("Permanent images", juce::Image::Permanence::permanent + 1);
        imagePermanenceCombo.addItem("Disposable images", juce::Image::Permanence::disposable + 1);
        addAndMakeVisible(imagePermanenceCombo);
        imagePermanenceCombo.setSelectedId(juce::Image::Permanence::disposable + 1, juce::dontSendNotification);
        imagePermanenceCombo.onChange = [this]
            {
                createCachedImage();
            };

        setSize(1024, 1024);
    }

    ~ImageDrawTest() override = default;

    void parentHierarchyChanged()    override
    {
        if (auto peer = getPeer())
        {
            direct2DToggle.setToggleState(peer->getCurrentRenderingEngine() == 1, juce::dontSendNotification);
        }
    }

    void resized() override
    {
        imagePermanenceCombo.setBounds(10, 10, 250, 30);
        direct2DToggle.setBounds(imagePermanenceCombo.getBounds().translated(0, 30));

        createCachedImage();
    }

    void paint(juce::Graphics& g) override
    {
        double elapsedSeconds = 0.0;

        {
            juce::ScopedTimeMeasurement stm{ elapsedSeconds };

            int destX = 0;
            int sourceX = 1;
            cachedImage.moveImageSection(destX, 0, sourceX, 0, cachedImage.getWidth() - 1, cachedImage.getHeight());

            {
                juce::Graphics imageGraphics{ cachedImage };
                float hue = (float)std::sin(phase) * 0.5f + 0.5f;
                imageGraphics.setColour(juce::Colour::fromHSV(hue, 1.0f, 1.0f, hue));
                imageGraphics.getInternalContext().fillRect(cachedImage.getBounds().removeFromRight(1), true);
            }

            g.fillCheckerBoard(getLocalBounds().toFloat(),
                getWidth() * 0.1f, getHeight() * 0.1f,
                Colours::darkgrey, Colours::lightgrey);

            g.drawImageAt(cachedImage, (getWidth() - cachedImage.getWidth()) / 2, (getHeight() - cachedImage.getHeight()) / 2);
        }

        g.setColour(juce::Colours::white);
        g.setFont(g.getCurrentFont().withHeight(40.0f).boldened());
        g.drawFittedText(juce::String(elapsedSeconds * 1000.0, 2) + " msec", getLocalBounds(), juce::Justification::topRight, 1);
    }

    void animate()
    {
        auto now = juce::Time::getMillisecondCounterHiRes();
        auto elapsedSeconds = (now - lastMsec) * 0.001;
        lastMsec = now;

        {
            auto nextPhase = phase + elapsedSeconds * juce::MathConstants<double>::twoPi * 0.2;
            while (nextPhase >= juce::MathConstants<double>::twoPi)
                nextPhase -= juce::MathConstants<double>::twoPi;

            phase = nextPhase;
        }

        repaint();
    }

    void imageDataChanged(ImagePixelData*) override
    {
    }

    void imageDataBeingDeleted(ImagePixelData*) override
    {
        //
        // Cached image data can be deleted unexpectedly; when that happens, the bitmap data is gone
        // and will need to be recreated.
        //
        // The renderer will call imageDataBeingDeleted for any listeners attached to the ImagePixelData
        //
        // Note that the DXGI adapter may not be available yet when this callback happens so you'll need to
        // defer recreating your images later
        //
        // You can also call Image::isValid() or Image::isNull() to see if the bitmap data is still available.
        //
        // To test this, change the Windows display DPI while your app is running
        //
    }

private:
    juce::VBlankAttachment attachment{ this, [this]() { animate(); } };
    double lastMsec = juce::Time::getMillisecondCounterHiRes();
    double phase = 0.0;
    float position = 0.0f;
    float lastPosition = 0.0f;

    juce::ToggleButton direct2DToggle{ "Direct2D" };
    juce::ComboBox imagePermanenceCombo;

    //
    // Direct2D resources are generally more expensive to create than they are to draw.
    // If you have an Image you want to use more than once, keep that image as a member
    // variable, on the heap, or in the JUCE ImageCache. The renderer will keep the bitmap
    // data cached on the GPU as long as the Image's internal data exists.
    //
    // Drawing a cached image is much faster than drawing a non-cached image.
    //
    juce::Image cachedImage;
    juce::Rectangle<int> imagePaintArea;

    void createCachedImage()
    {
        imagePaintArea = getLocalBounds().reduced(100);

        std::unique_ptr<juce::ImageType> imageType;
        if (direct2DToggle.getToggleState())
        {
            imageType = std::make_unique<juce::NativeImageType>();
        }
        else
        {
            imageType = std::make_unique<juce::SoftwareImageType>();
        }

        cachedImage = juce::Image{ juce::Image::ARGB, imagePaintArea.getWidth(),
            imagePaintArea.getHeight(),
            true,
            *imageType,
            imagePermanenceCombo.getSelectedId() == juce::Image::Permanence::permanent + 1 ? juce::Image::Permanence::permanent : juce::Image::Permanence::disposable };
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImageDrawTest)
};

