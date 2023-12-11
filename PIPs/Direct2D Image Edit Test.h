/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

  name:             Direct2D Image Edit Test

  dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics, juce_gui_basics
  exporters:        VS2022

  moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1
  defines:          JUCE_DIRECT2D=1

  type:             Component
  mainClass:        ImageEditTest

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

class ImageEditTest : public juce::Component, public juce::ImagePixelData::Listener
{
public:
    ImageEditTest()
    {
        resamplingQualityCombo.addItem("Low resampling quality", juce::Graphics::lowResamplingQuality + 1);
        resamplingQualityCombo.addItem("Medium resampling quality", juce::Graphics::mediumResamplingQuality + 1);
        resamplingQualityCombo.addItem("High resampling quality", juce::Graphics::highResamplingQuality + 1);
        addAndMakeVisible(resamplingQualityCombo);
        resamplingQualityCombo.setSelectedId(juce::Graphics::mediumResamplingQuality + 1, juce::dontSendNotification);

        modeCombo.addItem("clear", Mode::clear);
        modeCombo.addItem("rescaled", Mode::rescaled);
        modeCombo.addItem("createCopy", Mode::createCopy);
        modeCombo.addItem("convertedToFormat", Mode::convertedToFormat);
        modeCombo.addItem("getClippedImage", Mode::getClippedImage);
        addAndMakeVisible(modeCombo);
        modeCombo.setSelectedId(Mode::clear, juce::dontSendNotification);
        modeCombo.onChange = [this]
            {
                createCachedImages();
                repaint();
            };

        setSize(1024, 1024);
    }

    ~ImageEditTest() override = default;

    void resized() override
    {
        resamplingQualityCombo.setBounds(10, 10, 250, 30);
        modeCombo.setBounds(resamplingQualityCombo.getBounds().translated(0, resamplingQualityCombo.getHeight() + 5));

        createCachedImages();
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black);

        //
        // Check if the images are still valid; if not, then the Direct2D bitmap is no longer
        // cached on the GPU and the image data is gone.
        //
        if (cachedImage.isNull() || editedImage.isNull())
        {
            return;
        }

        {
            auto leftRect = getLocalBounds().removeFromLeft(getWidth() / 2);
            leftRect = leftRect.withSizeKeepingCentre(cachedImage.getWidth(), cachedImage.getHeight());
            g.drawImageAt(cachedImage, leftRect.getX(), leftRect.getY());
        }

        {
            auto rightRect = getLocalBounds().removeFromRight(getWidth() / 2);
            rightRect = rightRect.withSizeKeepingCentre(editedImage.getWidth(), editedImage.getHeight());
            g.drawImageAt(editedImage, rightRect.getX(), rightRect.getY());
        }
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
        clear = 1,
        rescaled,
        createCopy,
        convertedToFormat,
        getClippedImage
    };

    juce::ComboBox resamplingQualityCombo;
    juce::ComboBox modeCombo;

    //
    // Direct2D resources are generally more expensive to create than they are to draw.
    // If you have an Image you want to use more than once, keep that image as a member
    // variable, on the heap, or in the JUCE ImageCache. The renderer will keep the bitmap
    // data cached on the GPU as long as the Image's internal data exists.
    //
    // Drawing a cached image is much faster than drawing a non-cached image.
    //
    juce::Image cachedImage;
    juce::Image editedImage;

    void createCachedImages()
    {
        auto imageSize = getLocalBounds().removeFromLeft(getWidth() / 2).reduced(50);
        cachedImage = juce::Image{ juce::Image::ARGB, imageSize.getWidth(), imageSize.getHeight(), true };

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

        //
        // Create the edited image
        //
        switch (modeCombo.getSelectedId())
        {
        case Mode::clear:
        {
            editedImage = cachedImage.createCopy();
            editedImage.clear(editedImage.getBounds().reduced(50), juce::Colours::limegreen);
            break;
        }

        case Mode::rescaled:
        {
            imageSize *= 0.75f;
            editedImage = cachedImage.rescaled(imageSize.getWidth(), imageSize.getHeight(), (juce::Graphics::ResamplingQuality)resamplingQualityCombo.getSelectedId());
            break;
        }

        case Mode::createCopy:
        {
            editedImage = cachedImage.createCopy();
            break;
        }

        case Mode::convertedToFormat:
        {
            editedImage = cachedImage.convertedToFormat(juce::Image::SingleChannel);
            break;
        }

        case Mode::getClippedImage:
        {
            imageSize *= 0.75f;
            editedImage = cachedImage.getClippedImage(imageSize);
            break;
        }
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ImageEditTest)
};

