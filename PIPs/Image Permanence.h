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
        nativeImageWithBackup,
        nativeImageNoBackup
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

            modeCombo.addItem("Software image", softwareImage);
            modeCombo.addItem("D2D images with backup", nativeImageWithBackup);
            modeCombo.addItem("D2D images no backup", nativeImageNoBackup);

            addAndMakeVisible(modeCombo);
            modeCombo.setSelectedId(softwareImage, juce::dontSendNotification);
            modeCombo.onChange = [this]
                {
                    setImageType(modeCombo.getSelectedId());
                };

            addAndMakeVisible(desaturateToggle);
            addAndMakeVisible(multiplyAllAlphaToggle);
            addAndMakeVisible(dropShadowToggle);
            addAndMakeVisible(glowToggle);

            dropShadowToggle.onClick = [this]()
                {
                    if (dropShadowToggle.getToggleState())
                        setComponentEffect(&dropShadowEffect);
                    else
                        setComponentEffect(nullptr);

                    glowToggle.setToggleState(false, juce::dontSendNotification);
                };
            glowToggle.onClick = [this]()
                {
                    if (glowToggle.getToggleState())
                        setComponentEffect(&glowEffect);
                    else
                        setComponentEffect(nullptr);

                    dropShadowToggle.setToggleState(false, juce::dontSendNotification);
                };

            dropShadowEffect.setShadowProperties(juce::DropShadow{ juce::Colours::black.withAlpha(0.9f), 20, { 10, 10 } });
            glowEffect.setGlowProperties(20.0f, juce::Colours::cyan.withAlpha(0.9f));

            addAndMakeVisible(direct2DToggle);
            direct2DToggle.setToggleState(true, juce::dontSendNotification);
            direct2DToggle.onClick = [this]()
                {
                    setImageType(modeCombo.getSelectedId());
                };
        }

        void paintPolkaDots()
        {
            std::function<void(juce::Graphics& g)> painter = [&](juce::Graphics& g)
                {
                    juce::Random random;

                    auto r = getLocalBounds().toFloat();
                    auto area = getWidth() * getHeight();
                    for (int i = 0; i < area / 5000; ++i)
                    {
                        g.setColour(juce::Colour::fromHSV(random.nextFloat(), 1.0f, 1.0f, 1.0f));
                        float size = random.nextFloat() * 100.0f;

                        g.fillEllipse(random.nextFloat() * r.getWidth(),
                            random.nextFloat() * r.getHeight(),
                            size, size);
                    }
                };

            bool backupEnabled = modeCombo.getSelectedId() == nativeImageWithBackup;
            if (polkaDotsSourceImage.getWidth() != getWidth() || polkaDotsSourceImage.getHeight() != getHeight())
            {
                polkaDotsSourceImage = juce::Image{ juce::Image::ARGB, getWidth(), getHeight(), true, *imageType };
                if (auto extensions = polkaDotsSourceImage.getPixelData()->getBackupExtensions())
                    extensions->setBackupEnabled(backupEnabled);

                sourceX = 0;

                juce::Graphics g{ polkaDotsSourceImage };
                painter(g);
            }

            if (outputImage.getWidth() != getWidth() || outputImage.getHeight() != getHeight())
            {
                outputImage = juce::Image{ juce::Image::ARGB, getWidth(), getHeight(), true, *imageType };
                if (auto extensions = outputImage.getPixelData()->getBackupExtensions())
                    extensions->setBackupEnabled(backupEnabled);
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
                paintPolkaDots();

                {
                    juce::Graphics ig{ outputImage };
                    ig.setOpacity(0.0f);
                    ig.getInternalContext().fillRect(outputImage.getBounds(), true);
                    ig.setOpacity(1.0f);
                    ig.drawImageAt(polkaDotsSourceImage, 0, 0);
                }

                //
                // Use Image::moveImageSection to animate the polka dots
                //
                outputImage.moveImageSection(0, 0, sourceX, 0, outputImage.getWidth() - sourceX, outputImage.getHeight());

                {
                    juce::Graphics ig{ outputImage };
                    ig.setColour(juce::Colours::transparentBlack);
                    ig.getInternalContext().fillRect({ outputImage.getWidth() - sourceX, 0, sourceX, outputImage.getHeight() }, true);
                    ig.setOpacity(1.0f);
                    ig.drawImageAt(polkaDotsSourceImage, outputImage.getWidth() - sourceX, 0);
                }

                sourceX = (sourceX + 1) % polkaDotsSourceImage.getWidth();

                if (desaturateToggle.getToggleState())
                    outputImage.desaturate();
                if (multiplyAllAlphaToggle.getToggleState())
                    outputImage.multiplyAllAlphas(0.3f);

                g.drawImageAt(outputImage, 0, 0);
            }

            paintTimeMsecStats.addValue(elapsedSeconds * 1000.0);

            g.setColour(juce::Colours::black.withAlpha(0.8f));
            g.fillRect(0, 0, getWidth(), 90);
            g.setColour(juce::Colours::white);
            g.setFont(g.getCurrentFont().withHeight(40.0f));
            g.drawText("Average " + juce::String{ paintTimeMsecStats.getAverage(), 1 } + " msec/frame", getLocalBounds(), juce::Justification::topRight);
        }

        void resized() override
        {
            paintPolkaDots();

            paintTimeMsecStats.reset();

            modeCombo.setBounds(10, 10, 300, 30);
            direct2DToggle.setBounds(modeCombo.getRight(), modeCombo.getY(), 80, 30);
            desaturateToggle.setBounds(10, modeCombo.getBottom(), 250, 25);
            multiplyAllAlphaToggle.setBounds(10, desaturateToggle.getBottom(), 250, 25);
            dropShadowToggle.setBounds(desaturateToggle.getRight(), desaturateToggle.getY(), 250, 25);
            glowToggle.setBounds(dropShadowToggle.getX(), dropShadowToggle.getBottom(), 250, 25);
        }

        juce::VBlankAttachment attachment{ this, [this]()
            {
                auto now = juce::Time::getMillisecondCounterHiRes();
                auto elapsedSeconds = (now - lastMsec) * 0.001;
                lastMsec = now;

                angle.advance(elapsedSeconds * juce::MathConstants<double>::twoPi * 0.2);

                repaint();
            } };

        void parentHierarchyChanged() override
        {
            if (auto peer = getPeer())
            {
                modeCombo.onChange();
            }
        }

        void setImageType(int imageTypeID)
        {
            auto peer = getPeer();
            if (!peer)
                return;

            // Hide the parent component to force the cached component images to reset
            getParentComponent()->setVisible(false);

            switch (imageTypeID)
            {
            case softwareImage:
            {
                imageType = std::make_unique<juce::SoftwareImageType>();
                break;
            }
            case nativeImageWithBackup:
            {
                imageType = std::make_unique<juce::NativeImageType>();
                break;
            }
            case nativeImageNoBackup:
            {
                imageType = std::make_unique<juce::NativeImageType>();
                break;
            }
            }

            peer->setCurrentRenderingEngine(direct2DToggle.getToggleState() ? 1 : 0);

            polkaDotsSourceImage = {};
            outputImage = {};

            paintTimeMsecStats.reset();

            getParentComponent()->setVisible(true);
        }

    private:
        double lastMsec = juce::Time::getMillisecondCounterHiRes();
        juce::dsp::Phase<double> angle;

        juce::Image polkaDotsSourceImage;
        int sourceX = 0;
        juce::Image outputImage;
        std::unique_ptr<juce::ImageType> imageType = std::make_unique<juce::NativeImageType>();
        juce::StatisticsAccumulator<double> paintTimeMsecStats;
        juce::DropShadowEffect dropShadowEffect;
        juce::GlowEffect glowEffect;

        juce::ToggleButton direct2DToggle{ "Direct2D" };
        juce::ComboBox modeCombo;
        juce::ToggleButton desaturateToggle{ "Desaturate" };
        juce::ToggleButton multiplyAllAlphaToggle{ "Multiply all alpha" };
        juce::ToggleButton dropShadowToggle{ "Drop shadow" };
        juce::ToggleButton glowToggle{ "Glow" };

    } imageComponent;

    struct MessageTest : public juce::HighResolutionTimer
    {
        MessageTest()
        {
            startTimer(10);
        }

        void hiResTimerCallback() override
        {
            asyncHandler1.triggerAsyncUpdate();
            asyncHandler2.triggerAsyncUpdate();
        }

        struct AsyncHandler : public juce::AsyncUpdater
        {
            void handleAsyncUpdate() override
            {
                juce::Thread::sleep(10);
            }
        };

        AsyncHandler asyncHandler1, asyncHandler2;
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ImagePermanence)
};

