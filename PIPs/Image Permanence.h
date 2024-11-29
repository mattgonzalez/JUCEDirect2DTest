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
    enum
    {
        softwareImage = 1,
        permanentNativeImage,
        disposableNativeImage
    };

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

        setSize(800, 800);
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
    }

private:
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
                Colours::darkgrey, Colours::lightgrey);
        }
    } background;

    struct ImageComponent : public juce::Component
    {
        ImageComponent()
        {
            setOpaque(false);

            modeCombo.addItem("Software renderer & images", softwareImage);
            modeCombo.addItem("D2D renderer / permanent D2D images", permanentNativeImage);
            modeCombo.addItem("D2D renderer / disposable D2D images", disposableNativeImage);

            addAndMakeVisible(modeCombo);
            modeCombo.setSelectedId(disposableNativeImage, juce::dontSendNotification);
            modeCombo.onChange = [this]
                {
                    setImageType(modeCombo.getSelectedId());
                };

            addAndMakeVisible(desaturateToggle);
            addAndMakeVisible(multiplyAllAlpha);
        }

        void paintPolkaDots()
        {
            polkaDotsImage = juce::Image{ juce::Image::ARGB, getWidth(), getHeight(), true, *imageType, imagePermanence };

            {
                juce::Graphics g{ polkaDotsImage };
                juce::Random random;

                auto r = polkaDotsImage.getBounds().reduced(20).toFloat();
                for (int i = 0; i < 100; ++i)
                {
                    g.setColour(juce::Colour::fromHSV(random.nextFloat(), 1.0f, 1.0f, 1.0f));
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
                // Create images if necessary
                //
                if (polkaDotsImage.isNull() || polkaDotsImage.getBounds() != getLocalBounds())
                    paintPolkaDots();

                //
                // Use Image::moveImageSection to animate the polka dots
                //
                {
                    auto clippedPolkaDotsImage = polkaDotsImage.getClippedImage({ 0, 0, 1, polkaDotsImage.getHeight() });
                    clippedPolkaDotsImage = clippedPolkaDotsImage.createCopy();

                    if (polkaDotsImage.isValid() && clippedPolkaDotsImage.isValid())
                    {
                        polkaDotsImage.moveImageSection(0, 0, 1, 0, polkaDotsImage.getWidth() - 1, polkaDotsImage.getHeight());

                        {
                            juce::Graphics imageG{ polkaDotsImage };
                            imageG.setColour(juce::Colours::transparentBlack);
                            imageG.getInternalContext().fillRect({ polkaDotsImage.getWidth() - 1, 0, 1, polkaDotsImage.getHeight() }, true);
                            imageG.setColour(juce::Colours::black);
                            imageG.drawImageAt(clippedPolkaDotsImage, polkaDotsImage.getWidth() - 1, 0);
                        }
                    }
                }

                //
                // Apply effects to polka dots
                //
                auto polkaDotsCopy = polkaDotsImage.createCopy();
                if (desaturateToggle.getToggleState())
                    polkaDotsCopy.desaturate();
                if (multiplyAllAlpha.getToggleState())
                    polkaDotsCopy.multiplyAllAlphas(0.3f);

                //
                // Draw the polka dots image to the screen
                //
                g.drawImageAt(polkaDotsCopy, 0, 0);
            }

            paintTimeMsecStats.addValue(elapsedSeconds * 1000.0);

            g.setColour(juce::Colours::black.withAlpha(0.8f));
            g.fillRect(0, 0, getWidth(), 90);
            g.setColour(juce::Colours::white);
            g.setFont(g.getCurrentFont().withHeight(40.0f));
            g.drawText("Average " + juce::String{paintTimeMsecStats.getAverage(), 1} + " msec/frame", getLocalBounds(), juce::Justification::topRight);
        }

        void resized() override
        {
            paintPolkaDots();

            paintTimeMsecStats.reset();

            modeCombo.setBounds(10, 10, 300, 30);
            desaturateToggle.setBounds(10, modeCombo.getBottom(), 250, 25);
            multiplyAllAlpha.setBounds(10, desaturateToggle.getBottom(), 250, 25);
        }

        juce::VBlankAttachment attachment{ this, [this]()
            {
                auto now = juce::Time::getMillisecondCounterHiRes();
                auto elapsedSeconds = (now - lastMsec) * 0.001;
                lastMsec = now;
                
                angle.advance(elapsedSeconds* juce::MathConstants<double>::twoPi * 0.2);

                repaint();
            } };

        void setImageType(int imageTypeID)
        {
            auto peer = getPeer();
            if (!peer)
                return;

            switch (imageTypeID)
            {
            case softwareImage:
            {
                imageType = std::make_unique<juce::SoftwareImageType>();
                imagePermanence = juce::Image::Permanence::permanent;
                peer->setCurrentRenderingEngine(0);
                break;
            }
            case permanentNativeImage:
            {
                imagePermanence = juce::Image::Permanence::permanent;
                imageType = std::make_unique<juce::NativeImageType>();
                peer->setCurrentRenderingEngine(1);
                break;
            }
            case disposableNativeImage:
            {
                imagePermanence = juce::Image::Permanence::disposable;
                imageType = std::make_unique<juce::NativeImageType>();
                peer->setCurrentRenderingEngine(1);
                break;
            }
            }

            polkaDotsImage = {};

            paintTimeMsecStats.reset();
        }

    private:
        double lastMsec = juce::Time::getMillisecondCounterHiRes();
        juce::dsp::Phase<double> angle;

        juce::Image polkaDotsImage;
        std::unique_ptr<juce::ImageType> imageType = std::make_unique<juce::NativeImageType>();
        juce::Image::Permanence imagePermanence = juce::Image::Permanence::disposable;
        juce::StatisticsAccumulator<double> paintTimeMsecStats;

        juce::ComboBox modeCombo;
        juce::ToggleButton desaturateToggle{ "Desaturate" };
        juce::ToggleButton multiplyAllAlpha{ "Multiply all alpha" };
    } imageComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ImagePermanence)
};

