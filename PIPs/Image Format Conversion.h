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
        softwareImageType = 1,
        nativeImageType
    };

    enum
    {
        permanentImage = 1,
        disposableImage
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
            //setBufferedToImage(true);
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

            sourcePermanenceCombo.addItem("Permanent", permanentImage);
            sourcePermanenceCombo.addItem("Disposable", disposableImage);
            addAndMakeVisible(sourcePermanenceCombo);
            sourcePermanenceCombo.setSelectedId(permanentImage, juce::dontSendNotification);
            sourcePermanenceCombo.onChange = [this] { paintTimeMsecStats.reset();  };

            sourceImageTypeCombo.addItem("Software image", softwareImageType);
            sourceImageTypeCombo.addItem("Native image", nativeImageType);
            addAndMakeVisible(sourceImageTypeCombo);
            sourceImageTypeCombo.setSelectedId(softwareImageType, juce::dontSendNotification);
            sourceImageTypeCombo.onChange = sourcePermanenceCombo.onChange;

            convertedImageTypeCombo.addItem("Convert to software image", softwareImageType);
            convertedImageTypeCombo.addItem("Convert to native image", nativeImageType);
            addAndMakeVisible(convertedImageTypeCombo);
            convertedImageTypeCombo.setSelectedId(softwareImageType, juce::dontSendNotification);
            convertedImageTypeCombo.onChange = sourcePermanenceCombo.onChange;

            addAndMakeVisible(direct2DToggle);
            direct2DToggle.onClick = [this]
                {
                    paintTimeMsecStats.reset();

                    getPeer()->setCurrentRenderingEngine(direct2DToggle.getToggleState() ? 1 : 0);
                };
            direct2DToggle.setToggleState(true, juce::dontSendNotification);
        }

        void paint(juce::Graphics& g) override
        {
            double elapsedSeconds = 0.0;
            
            {
                juce::ScopedTimeMeasurement stm{ elapsedSeconds };

                //
                // Create images if necessary
                //
                std::unique_ptr<juce::ImageType> sourceImageType;
                
                if (sourceImageTypeCombo.getSelectedId() == softwareImageType)
                {
                    sourceImageType.reset(new juce::SoftwareImageType());
                }
                else
                {
                    sourceImageType.reset(new juce::NativeImageType());
                }

                auto sourcePermanence = sourcePermanenceCombo.getSelectedId() == permanentImage ? juce::Image::Permanence::permanent : juce::Image::Permanence::disposable;
                auto sourceImage = juce::Image{ juce::Image::ARGB, getWidth() / 3, getHeight() / 3, true, *sourceImageType, sourcePermanence };

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

                //
                // Convert between all possible bitmap formats
                //
                std::array<juce::Image::PixelFormat, 3> constexpr static formats{juce::Image::RGB, 
                    juce::Image::ARGB, 
                    juce::Image::SingleChannel };
                static juce::StringArray const formatNames{ "", "RGB", "ARGB", "SingleChannel" };
                int x = 0, y = 0;

                std::unique_ptr<juce::ImageType> convertedImageType;

                if (convertedImageTypeCombo.getSelectedId() == softwareImageType)
                {
                    convertedImageType.reset(new juce::SoftwareImageType());
                }
                else
                {
                    convertedImageType.reset(new juce::NativeImageType());
                }

                for (auto sourceFormat : formats)
                {
                    for (auto destFormat : formats)
                    {
                        auto convertedImage = convertedImageType->convert(sourceImage);
                        g.drawImageAt(convertedImage.convertedToFormat(sourceFormat).convertedToFormat(destFormat), x, y);

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
            paintTimeMsecStats.reset();

            sourceImageTypeCombo.setBounds(10, 10, 300, 30);
            sourcePermanenceCombo.setBounds(10, 50, 300, 30);
            convertedImageTypeCombo.setBounds(10, 90, 300, 30);
            direct2DToggle.setBounds(10, 130, 300, 30);
        }

        juce::VBlankAttachment attachment{ this, [this]()
            {
                repaint();
            } };

        void parentHierarchyChanged() override 
        {
        }

    private:
        juce::StatisticsAccumulator<double> paintTimeMsecStats;
        juce::ComboBox sourceImageTypeCombo;
        juce::ComboBox sourcePermanenceCombo;
        juce::ComboBox convertedImageTypeCombo;
        juce::ToggleButton direct2DToggle{ "D2D" };
    } imageComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ImageFormatConversion)
};

