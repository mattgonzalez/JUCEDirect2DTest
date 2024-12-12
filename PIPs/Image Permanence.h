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

            modeCombo.addItem("Software image", softwareImage);
            modeCombo.addItem("Permanent D2D images", permanentNativeImage);
            modeCombo.addItem("Disposable D2D images", disposableNativeImage);

            addAndMakeVisible(modeCombo);
            modeCombo.setSelectedId(disposableNativeImage, juce::dontSendNotification);
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
            polkaDotsLegacyImage = {};
            polkaDotsTransientImage = {};

            std::function<void(juce::Graphics& g)> painter = [&](juce::Graphics& g)
                {
                    juce::Random random;

                    auto r = getLocalBounds().toFloat();
                    for (int i = 0; i < 100; ++i)
                    {
                        g.setColour(juce::Colour::fromHSV(random.nextFloat(), 1.0f, 1.0f, 1.0f));
                        float size = random.nextFloat() * 100.0f;

                        g.fillEllipse(random.nextFloat() * r.getWidth(),
                            random.nextFloat() * r.getHeight(),
                            size, size);
                    }
                };

            if (modeCombo.getSelectedId() == disposableNativeImage)
            {
                if (polkaDotsTransientImage.getWidth() != getWidth() || polkaDotsTransientImage.getHeight() != getHeight())
                {
                    polkaDotsTransientImage.setProperties(Image::ARGB, getWidth(), getHeight(), true);
                    polkaDotsTransientImage.modify(painter);
                }
            }
            else
            {
                if (polkaDotsLegacyImage.getWidth() != getWidth() || polkaDotsLegacyImage.getHeight() != getHeight())
                {
                    polkaDotsLegacyImage = juce::Image{ juce::Image::ARGB, getWidth(), getHeight(), true, *imageType };
                    juce::Graphics g{ polkaDotsLegacyImage };
                    painter(g);
                }
            }
        }

        void paint(juce::Graphics& g) override
        {
            double elapsedSeconds = 0.0;
            
            //return;

            {
                juce::ScopedTimeMeasurement stm{ elapsedSeconds };

                //
                // Create images if necessary
                //
                paintPolkaDots();

                //
                // Use Image::moveImageSection to animate the polka dots
                //
                /*
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
                */

                //
                // Apply effects to polka dots
                //
#if 0
                auto copyTransientImage = [](const juce::TransientImage& source)
                    {
                        auto clone = juce::TransientImage{};
                        clone.setProperties(Image::ARGB, source.getWidth(), source.getHeight(), true);
                        clone.modify([&](juce::Graphics& g)
                            {
                                source.paintToContext(g, {});
                            });

                        return clone;
                    };


                auto polkaDotsCopy = copyTransientImage(polkaDotsTransientImage);
                if (desaturateToggle.getToggleState())
                    polkaDotsCopy.desaturate();
                if (multiplyAllAlphaToggle.getToggleState())
                    polkaDotsCopy.multiplyAllAlphas(0.3f);

                //
                // Draw the polka dots image to the screen
                //
                g.drawImageAt(polkaDotsImage, 0, 0);
#endif

                polkaDotsTransientImage.paintToContext(g, {});
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
                
                angle.advance(elapsedSeconds* juce::MathConstants<double>::twoPi * 0.2);

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
            case permanentNativeImage:
            {
                imageType = std::make_unique<juce::NativeImageType>();
                break;
            }
            case disposableNativeImage:
            {
                imageType = std::make_unique<juce::NativeImageType>();
                break;
            }
            }

            peer->setCurrentRenderingEngine(direct2DToggle.getToggleState() ? 1 : 0);

            polkaDotsLegacyImage = {};
            polkaDotsTransientImage.reset();

            paintTimeMsecStats.reset();

            getParentComponent()->setVisible(true);
        }

    private:
        double lastMsec = juce::Time::getMillisecondCounterHiRes();
        juce::dsp::Phase<double> angle;

        juce::Image polkaDotsLegacyImage;
        juce::TransientImage polkaDotsTransientImage;
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

