/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

  name:             Image Format Conversion

  dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics, juce_gui_basics, juce_dsp, juce_audio_basics, juce_audio_formats
  exporters:        VS2022, xcode_mac, linux_make

  moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1
  defines:

  type:             Component
  mainClass:        ImageFormatConversion

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

class ImageFormatConversion : public juce::Component
{
public:
    enum
    {
        softwareImage = 1,
        permanentNativeImage,
        disposableNativeImage
    };

    ImageFormatConversion()
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

        setBounds(juce::Desktop::getInstance().getDisplays().getPrimaryDisplay()->userArea.reduced(50));
     }

    ~ImageFormatConversion() override = default;

    void paint(juce::Graphics&) override {}

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
        }

        void paint(juce::Graphics& g) override
        {
            double elapsedSeconds = 0.0;
            
            {
                juce::ScopedTimeMeasurement stm{ elapsedSeconds };

                //
                // Create images if necessary
                //
                if (sourceImage.isNull() || sourceImage.getBounds() != getLocalBounds())
                {
                    sourceImage = juce::Image{ juce::Image::ARGB, getWidth() / 3, getHeight() / 3, true, *imageType, imagePermanence };

                    {
                        juce::Graphics imageG{ sourceImage };
                        auto gradient = juce::ColourGradient{ juce::Colours::cyan, 
                            sourceImage.getBounds().toFloat().getCentre(), 
                            juce::Colours::black.withAlpha(0.8f), 
                            { 0.0f, (float)sourceImage.getHeight() * 0.5f},
                            true };
                        imageG.setGradientFill(gradient);
                        imageG.fillEllipse(sourceImage.getBounds().toFloat());
                    }
                }

                //
                // Convert between all possible bitmap formats
                //
                std::array<juce::Image::PixelFormat, 3> constexpr static formats{juce::Image::RGB, 
                    juce::Image::ARGB, 
                    juce::Image::SingleChannel };
                static juce::StringArray const formatNames{ "", "RGB", "ARGB", "SingleChannel" };
                int x = 0, y = 0;
                for (auto sourceFormat : formats)
                {
                    for (auto destFormat : formats)
                    {
                        g.drawImageAt(sourceImage.convertedToFormat(sourceFormat).convertedToFormat(destFormat), x, y);

                        g.setColour(juce::Colours::black);
                        g.drawText(formatNames[sourceFormat] + " to " + formatNames[destFormat], 
                            x, y, sourceImage.getWidth(), sourceImage.getHeight(), 
                            juce::Justification::centred);

                        x += sourceImage.getWidth();
                        if (x + sourceImage.getWidth() >= getWidth())
                        {
                            x = 0;
                            y += sourceImage.getHeight();
                        }
                    }
                }
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
            sourceImage = {};

            paintTimeMsecStats.reset();

            modeCombo.setBounds(10, 10, 300, 30);
        }

        juce::VBlankAttachment attachment{ this, [this]()
            {
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
                peer->setCurrentRenderingEngine(0);

                imagePermanence = juce::Image::Permanence::permanent;
                imageType = std::make_unique<juce::SoftwareImageType>();
                break;
            }
            case permanentNativeImage:
            {
                peer->setCurrentRenderingEngine(1);

                imagePermanence = juce::Image::Permanence::permanent;
                imageType = std::make_unique<juce::NativeImageType>();
                break;
            }
            case disposableNativeImage:
            {
                peer->setCurrentRenderingEngine(1);

                imagePermanence = juce::Image::Permanence::disposable;
                imageType = std::make_unique<juce::NativeImageType>();
                break;
            }
            }

            sourceImage = {};

            paintTimeMsecStats.reset();

            getParentComponent()->setVisible(true);
        }

    private:
        std::unique_ptr<juce::ImageType> imageType = std::make_unique<juce::NativeImageType>();
        juce::Image::Permanence imagePermanence = juce::Image::Permanence::disposable;
        juce::StatisticsAccumulator<double> paintTimeMsecStats;
        juce::Image sourceImage;
        juce::ComboBox modeCombo;
    } imageComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ImageFormatConversion)
};

