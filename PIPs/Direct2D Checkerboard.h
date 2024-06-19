/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

  name:             Direct2D Checkerboard

  dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics, juce_gui_basics, juce_animation
  exporters:        VS2022

  moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1
  defines:          JUCE_DIRECT2D_METRICS=1

  type:             Component
  mainClass:        Checkerboard

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

class Checkerboard : public juce::Component
{
public:
    Checkerboard()
    {
        modeCombo.addItem("drawCheckerboard", (int)Mode::drawCheckerboard);
        modeCombo.addItem("Image fill", (int)Mode::setFillType);
        addAndMakeVisible(modeCombo);
        modeCombo.setSelectedId((int)Mode::drawCheckerboard, juce::dontSendNotification);

        checkSizeSlider.setRange(2.0, 100.0, 1.0);
        checkSizeSlider.setValue(100.0, juce::dontSendNotification);
        checkSizeSlider.onValueChange = [this]
            {
                createCachedImage();
            };
        addAndMakeVisible(checkSizeSlider);

        updater.addAnimator(animator);
        animator.start();

        setSize(1024, 1024);
    }

    ~Checkerboard() override = default;

    void resized() override
    {
        modeCombo.setBounds(10, 10, 250, 30);
        checkSizeSlider.setBounds(10, 50, 250, 30);
        createCachedImage();
    }

    void paint(juce::Graphics& g) override
    {
        auto x = std::sin(position * juce::MathConstants<float>::twoPi) * 100.0f;
        auto checkSize = (float) checkSizeSlider.getValue();
        switch (modeCombo.getSelectedId())
        {
        case (int)Mode::drawCheckerboard:
        {
            g.fillAll(juce::Colours::darkgrey);
            g.setColour(juce::Colours::lightgrey);
            g.addTransform(juce::AffineTransform::translation(x - checkSize, 0.0f));
            g.fillRectList(list);
            break;
        }

        case (int)Mode::setFillType:
        {
            g.setFillType(juce::FillType{ checkerImage, juce::AffineTransform::translation(x, 0.0f) });
            g.fillAll();
            break;
        }
        }
    }

private:
    float position = 0.0f;
    juce::VBlankAnimatorUpdater updater{ this };
    Animator animator = juce::ValueAnimatorBuilder{}
        .withEasing(Easings::createLinear())
        .runningInfinitely()
        .withValueChangedCallback([this](float value)
            {
                position = value * 0.1f;
                repaint();
            })
        .build();

    enum class Mode
    {
        drawCheckerboard = 1,
        setFillType
    };
    juce::ComboBox modeCombo;
    juce::Slider checkSizeSlider{ juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight };

    juce::Image checkerImage;
    juce::RectangleList<float> list;

    void createCachedImage()
    {
        auto checkSize = (float)checkSizeSlider.getValue();;
        checkerImage = juce::Image(juce::Image::PixelFormat::RGB, juce::roundToInt(checkSize) * 2, juce::roundToInt(checkSize) * 2, true);
        juce::Graphics g{ checkerImage };
        g.fillCheckerBoard(checkerImage.getBounds().toFloat(), checkSize, checkSize, Colours::lightgrey, Colours::darkgrey);

        list.clear();
        list.ensureStorageAllocated(((float)getWidth() / checkSize) * ((float)getHeight() / checkSize));
        for (float x= 0.0f; x < getWidth(); x += checkSize * 2.0f)
        {
            for (float y = 0.0f; y < (float)getHeight(); y += checkSize * 2.0f)
            {
                list.addWithoutMerging({ x, y, checkSize, checkSize });
                list.addWithoutMerging({ x + checkSize, y + checkSize, checkSize, checkSize });
            }
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Checkerboard)
};

