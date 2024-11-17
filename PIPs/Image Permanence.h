/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

  name:             Image Permanence

  dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics, juce_gui_basics, juce_dsp, juce_audio_basics, juce_audio_formats
  exporters:        VS2022, xcode_mac, linux_make

  moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1
  defines:

  type:             Component
  mainClass:        ImagePermanence

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

class ImagePermanence : public juce::Component
{
public:
    ImagePermanence()
    {
        setOpaque(true);

        //
        // The background component is buffered to an image and fills this entire component.
        //
        // The background component is opaque and will not change unless the window is resized
        //
        addAndMakeVisible(background);

        //
        // imageComponent repaints constantly and is not buffered to an image. Note that the imageComponent is a *sibling* of the background component.
        //
        addAndMakeVisible(imageComponent);

        imagePermanenceCombo.addItem("Permanent images", juce::Image::Permanence::permanent + 1);
        imagePermanenceCombo.addItem("Disposable images", juce::Image::Permanence::disposable + 1);
        addAndMakeVisible(imagePermanenceCombo);
        imagePermanenceCombo.setSelectedId(juce::Image::Permanence::disposable + 1, juce::dontSendNotification);
        imagePermanenceCombo.onChange = [this]
            {
                imageComponent.setImagePermanence(juce::Image::Permanence(imagePermanenceCombo.getSelectedId() - 1));
            };

        setSize(1024, 1024);
    }

    ~ImagePermanence() override = default;

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black);
    }

    void resized() override
    {
        background.setBounds(getLocalBounds());
        imageComponent.setBounds(getLocalBounds());

        imagePermanenceCombo.setBounds(10, 10, 250, 30);
    }

private:
    juce::ComboBox imagePermanenceCombo;

    struct BackgroundComponent : public juce::Component
    {
        BackgroundComponent()
        {
            setBufferedToImage(true);
            setOpaque(true);
        }

        void paint(juce::Graphics& g) override
        {
            g.fillCheckerBoard(getLocalBounds().toFloat(),
                getWidth() * 0.1f, getHeight() * 0.1f,
                Colours::lightgrey, Colours::darkgrey);
        }
    } background;

    struct ImageComponent : public juce::Component
    {
        ImageComponent()
        {
            setOpaque(false);
        }

        void paintPolkaDots()
        {
            polkaDotsImage = juce::Image{ juce::Image::ARGB, getWidth(), getHeight(), true, juce::NativeImageType{}, imagePermanence };

            {
                juce::Graphics g{ polkaDotsImage };
                juce::Random random;

                auto r = polkaDotsImage.getBounds().reduced(20).toFloat();
                for (int i = 0; i < 100; ++i)
                {
                    g.setColour(juce::Colour::fromHSV(random.nextFloat(), 1.0f, 1.0f, 0.5f));
                    float size = random.nextFloat() * 100.0f;
                    g.fillEllipse(random.nextFloat() * r.getWidth(),
                        random.nextFloat() * r.getHeight(),
                        size, size);
                }
            }
        }

        void paint(juce::Graphics& g) override
        {
            double elapsedSeconds = 0.0;

            {
                juce::ScopedTimeMeasurement stm{ elapsedSeconds };

                //
                // Use double buffering to create a persistence effect. Alternate drawing between outputImages[0] and outputImages[1].
                //
                // This avoids using multiplyAllAlphas, which will cause the Direct2D image to be mapped from the GPU -> CPU and cause a performance hit
                //
                auto& outputImage = outputImages[outputImageIndex];
                auto& previousOutputImage = outputImages[outputImageIndex ^ 1];

                //
                // Toggle the double buffer index
                //
                outputImageIndex ^= 1;

                //
                // Create images if necessary
                //
                if (polkaDotsImage.isNull() || polkaDotsImage.getBounds() != getLocalBounds())
                    paintPolkaDots();

                if (outputImage.isNull() || outputImage.getBounds() != polkaDotsImage.getBounds())
                    outputImage = juce::Image{ juce::Image::ARGB, polkaDotsImage.getWidth(), polkaDotsImage.getHeight(), true, juce::NativeImageType{}, imagePermanence };

                //
                // For each frame, paint the previous output image with slight transparency, then paint the polka dots image on top of that with full opacity
                //
                {
                    juce::Graphics imageGraphics{ outputImage };
                    imageGraphics.setColour(juce::Colours::transparentBlack);
                    imageGraphics.getInternalContext().fillRect(outputImage.getBounds(), true);

                    if (previousOutputImage.isValid())
                    {
                        imageGraphics.setOpacity(0.98f);
                        imageGraphics.drawImageAt(previousOutputImage, 0, 0);
                    }

                    imageGraphics.setOpacity(1.0f);
                    imageGraphics.drawImageTransformed(polkaDotsImage, juce::AffineTransform::rotation((float)angle.phase, 0.5f * (float)outputImage.getWidth(), 0.5f * (float)outputImage.getHeight()));
                }

                //
                // Draw the composited output image to the screen
                //
                g.drawImageAt(outputImage, 0, 0);
            }

            paintTimeMsecStats.addValue(elapsedSeconds * 1000.0);

            g.setColour(juce::Colours::black);
            g.fillRect(getLocalBounds().removeFromRight(450).removeFromTop(50));
            g.setColour(juce::Colours::white);
            g.setFont(g.getCurrentFont().withHeight(40.0f));
            g.drawText("Average " + juce::String{paintTimeMsecStats.getAverage(), 1} + " msec/frame", getLocalBounds(), juce::Justification::topRight);
        }

        void resized() override
        {
            paintPolkaDots();

            paintTimeMsecStats.reset();
        }

        juce::VBlankAttachment attachment{ this, [this]()
            {
                auto now = juce::Time::getMillisecondCounterHiRes();
                auto elapsedSeconds = (now - lastMsec) * 0.001;
                lastMsec = now;

                angle.advance(elapsedSeconds * juce::MathConstants<double>::twoPi * 0.2);

                repaint();
            } };

        void setImagePermanence(juce::Image::Permanence newPermanence)
        {
            imagePermanence = newPermanence;

            polkaDotsImage = {};
            for (auto& outputImage : outputImages)
                outputImage = {};

            paintTimeMsecStats.reset();
        }

    private:
        double lastMsec = juce::Time::getMillisecondCounterHiRes();
        juce::dsp::Phase<double> angle;

        juce::Image polkaDotsImage;
        std::array<juce::Image, 2> outputImages;
        int outputImageIndex = 0;
        juce::Image::Permanence imagePermanence = juce::Image::Permanence::disposable;
        juce::StatisticsAccumulator<double> paintTimeMsecStats;
    } imageComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ImagePermanence)
};

