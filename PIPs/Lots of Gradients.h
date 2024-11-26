/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

  name:             Lots of Gradients

  dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics, juce_gui_basics, juce_dsp, juce_audio_basics, juce_audio_formats, juce_opengl, juce_gui_extra
  exporters:        VS2022, xcode_mac, linux_make

  moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1
  defines:

  type:             Component
  mainClass:        LotsOfGradients

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

class LotsOfGradients : public juce::Component
{
public:
    LotsOfGradients()
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
                imageComponent.setLotsOfGradients(juce::Image::Permanence(imagePermanenceCombo.getSelectedId() - 1));
            };

        setSize(1024, 1024);
    }

    ~LotsOfGradients() override = default;

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

        void paintGradients()
        {
            if (cachedImage.isNull() || cachedImage.getWidth() != getWidth() || cachedImage.getHeight())
                cachedImage = juce::Image{ juce::Image::ARGB, getWidth(), getHeight(), true, *imageType, LotsOfGradients };

            {
                juce::Graphics g{ cachedImage };

                float x = 0.0f;
                float w = (float)getWidth() * 2.0f / (float)linearGradients.size();
                float xStep = w * 0.5f;
                for (auto const& gradient : linearGradients)
                {
                    g.setGradientFill(gradient);
                    g.fillRect(x, 0.0f, w, (float)getHeight());
                    x += xStep;
                }

                x = 0.0f;
                float y = 0.0f;
                float yStep = gradientSize * 2.0f;
                for (auto& gradient : radialGradients)
                {
                    juce::FillType fillType{ gradient };
                    g.setFillType(fillType.transformed(juce::AffineTransform::translation(x, y)));
                    g.fillRect(cachedImage.getBounds().toFloat());
                    x += gradientSize;
                    if (x >= getWidth())
                    {
                        x = 0.0f;
                        y += yStep;
                    }
                }
            }
        }

        void paint(juce::Graphics& g) override
        {
            double elapsedSeconds = 0.0;

            {
                juce::ScopedTimeMeasurement stm{ elapsedSeconds };

                paintGradients();

                g.drawImageAt(cachedImage, 0, 0);
            }

            paintTimeMsecStats.addValue(elapsedSeconds * 1000.0);

            g.setColour(juce::Colours::black);
            g.fillRect(getLocalBounds().removeFromRight(450).removeFromTop(50));
            g.setColour(juce::Colours::white);
            g.setFont(g.getCurrentFont().withHeight(40.0f));
            g.drawText("Average " + juce::String{ paintTimeMsecStats.getAverage(), 1 } + " msec/frame", getLocalBounds(), juce::Justification::topRight);
        }

        void resized() override
        {
            paintGradients();

            paintTimeMsecStats.reset();

            linearGradients.clear();
            for (int i = 0; i < 128; ++i)
            {
                float hue = (float)i / 128.0f;
                linearGradients.emplace_back(juce::ColourGradient::vertical(
                    juce::Colours::black,
                    0.0f,
                    juce::Colour::fromHSV(hue, 1.0f, 1.0f, 0.5f),
                    (float)getHeight()));
            }

            radialGradients.clear();
            gradientSize = (float)getWidth() * 0.0625f;
            for (int i = 0; i < 128; ++i)
            {
                float hue = (float)i / 128.0f;
                radialGradients.emplace_back(juce::ColourGradient
                    {
                        juce::Colour::fromHSV(hue, 1.0f, 1.0f, 0.5f),
                        0.0f, 0.0f,
                        juce::Colours::transparentBlack,
                        gradientSize, gradientSize,
                        true
                    });
            }
        }

        juce::VBlankAttachment attachment{ this, [this]()
            {
                auto now = juce::Time::getMillisecondCounterHiRes();
                auto elapsedSeconds = (now - lastMsec) * 0.001;
                lastMsec = now;

                angle.advance(elapsedSeconds * juce::MathConstants<double>::twoPi * 0.2);

                repaint();
            } };

        void setLotsOfGradients(juce::Image::Permanence newPermanence)
        {
            LotsOfGradients = newPermanence;

            cachedImage = {};

            paintTimeMsecStats.reset();
        }

    private:
        double lastMsec = juce::Time::getMillisecondCounterHiRes();
        juce::dsp::Phase<double> angle;

        std::unique_ptr<juce::ImageType> imageType = std::make_unique<juce::NativeImageType>();
        juce::Image cachedImage;
        juce::Image::Permanence LotsOfGradients = juce::Image::Permanence::disposable;
        juce::StatisticsAccumulator<double> paintTimeMsecStats;

        float gradientSize = 0.0f;
        std::vector<juce::ColourGradient> linearGradients;
        std::vector<juce::ColourGradient> radialGradients;

    } imageComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LotsOfGradients)
};
