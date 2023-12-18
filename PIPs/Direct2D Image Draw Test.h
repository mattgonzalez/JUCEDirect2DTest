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
        resamplingQualityCombo.addItem("Low resampling quality", juce::Graphics::lowResamplingQuality + 1);
        resamplingQualityCombo.addItem("Medium resampling quality", juce::Graphics::mediumResamplingQuality + 1);
        resamplingQualityCombo.addItem("High resampling quality", juce::Graphics::highResamplingQuality + 1);
        addAndMakeVisible(resamplingQualityCombo);
        resamplingQualityCombo.setSelectedId(juce::Graphics::mediumResamplingQuality + 1, juce::dontSendNotification);

        modeCombo.addItem("drawImageAt", Mode::drawImageAt);
        modeCombo.addItem("drawImageWithin", Mode::drawImageWithin);
        modeCombo.addItem("drawImageTransformed", Mode::drawImageTransformed);
        addAndMakeVisible(modeCombo);
        modeCombo.setSelectedId(Mode::drawImageTransformed, juce::dontSendNotification);
        modeCombo.onChange = [this]
            {
                transformCombo.setEnabled(modeCombo.getSelectedId() == Mode::drawImageTransformed);
            };

        transformCombo.addItem("Translate", TransformType::translate);
        transformCombo.addItem("Scale", TransformType::scale);
        transformCombo.addItem("Shear", TransformType::shear);
        transformCombo.addItem("Rotate", TransformType::rotate);
        addAndMakeVisible(transformCombo);
        transformCombo.setSelectedId(TransformType::translate, juce::dontSendNotification);

        setSize(1024, 1024);
    }

    ~ImageDrawTest() override = default;

    void resized() override
    {
        resamplingQualityCombo.setBounds(10, 10, 250, 30);
        modeCombo.setBounds(resamplingQualityCombo.getBounds().translated(0, resamplingQualityCombo.getHeight() + 5));
        transformCombo.setBounds(modeCombo.getBounds().translated(0, modeCombo.getHeight() + 5));

        createCachedImage();
    }

    void paint(juce::Graphics& g) override
    {
        //
        // Check if the cachedImage is still valid; if it's not valid, then the Direct2D bitmap is no longer
        // cached on the GPU and the image data is gone.
        //
        if (cachedImage.isNull())
        {
            return;
        }

        g.setImageResamplingQuality((juce::Graphics::ResamplingQuality)(resamplingQualityCombo.getSelectedId() - 1));

        g.fillAll(juce::Colours::black);

        auto mousePos = getMouseXYRelative();
        mousePos.x = juce::jlimit(imagePaintArea.getX(), imagePaintArea.getRight(), mousePos.x);
        mousePos.y = juce::jlimit(imagePaintArea.getY(), imagePaintArea.getBottom(), mousePos.y);

        switch (modeCombo.getSelectedId())
        {
        case Mode::drawImageAt:
        {
            auto r = cachedImage.getBounds().withCentre(mousePos);
            g.drawImageAt(cachedImage, r.getX(), r.getY());
            break;
        }

        case Mode::drawImageWithin:
        {
            Rectangle<int> r = imagePaintArea.withSizeKeepingCentre(std::abs(mousePos.x - imagePaintArea.getCentreX()) * 2,
                std::abs(mousePos.y - imagePaintArea.getCentreY()) * 2);
            g.drawImageWithin(cachedImage, r.getX(), r.getY(), r.getWidth(), r.getHeight(),
                juce::RectanglePlacement::centred);
            break;
        }

        case Mode::drawImageTransformed:
        {
            g.drawImageTransformed(cachedImage, animatedTransform);
            break;
        }
        }
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

        auto position = (float)std::sin(phase);
        auto clampedPosition = juce::jlimit(0.0f, 1.0f, position * 0.5f + 0.5f);
        auto imageCenter = cachedImage.getBounds().getCentre().toFloat();
        auto componentCenter = getLocalBounds().getCentre().toFloat();
        switch (transformCombo.getSelectedId())
        {
        case TransformType::scale:
            animatedTransform = juce::AffineTransform::scale(clampedPosition, clampedPosition, imageCenter.x, imageCenter.y);
            break;

        case TransformType::translate:
            animatedTransform = juce::AffineTransform::translation(position * 50.0f, position * 50.0f);
            break;

        case TransformType::shear:
            animatedTransform = juce::AffineTransform::shear(position * 0.5f, position * 0.5f);
            break;

        case TransformType::rotate:
            animatedTransform = juce::AffineTransform::rotation((float)phase, imageCenter.x, imageCenter.y);
            break;
        }

        animatedTransform = animatedTransform.translated(componentCenter - imageCenter);

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
    enum Mode
    {
        drawImageAt = 1,
        drawImageWithin,
        drawImageTransformed
    };

    enum TransformType
    {
        scale = 1,
        translate,
        shear,
        rotate
    };

    juce::VBlankAttachment attachment{ this, [this]() { animate(); } };
    double lastMsec = juce::Time::getMillisecondCounterHiRes();
    juce::AffineTransform animatedTransform;
    double phase = 0.0;

    juce::ComboBox resamplingQualityCombo;
    juce::ComboBox modeCombo;
    juce::ComboBox transformCombo;

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

        cachedImage = juce::Image{ juce::Image::ARGB, imagePaintArea.getWidth(), imagePaintArea.getHeight(), true };

        {
            //
            // For DirectD images, drawing doesn't actually happen until the Graphics object goes
            // out of scope. So - surround the Graphics object with braces.
            //
            juce::Graphics g{ cachedImage };

            g.fillCheckerBoard(cachedImage.getBounds().toFloat(),
                cachedImage.getWidth() * 0.1f, cachedImage.getHeight() * 0.1f,
                Colours::lightgrey.withAlpha(0.5f), Colours::darkgrey.withAlpha(0.5f));

            g.setColour(juce::Colours::aliceblue);
            g.drawEllipse(cachedImage.getBounds().toFloat().reduced(10.0f), 5.0f);
        }

        //
        // Attach a listener to the image to be notified if the cached image is
        // unexpectedly deleted
        //
        cachedImage.getPixelData()->listeners.add(this);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImageDrawTest)
};

