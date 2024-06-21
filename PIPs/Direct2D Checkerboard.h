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

        checkWidthSlider.setRange(2.0, 1000.0, 1.0);
        checkWidthSlider.setValue(100.0, juce::dontSendNotification);
        checkWidthSlider.onValueChange = [this]
            {
                createCachedImage();
            };
        addAndMakeVisible(checkWidthSlider);

        checkHeightSlider.setRange(2.0, 1000.0, 1.0);
        checkHeightSlider.setValue(100.0, juce::dontSendNotification);
        checkHeightSlider.onValueChange = [this]
            {
                createCachedImage();
            };
        addAndMakeVisible(checkHeightSlider);

        direct2DToggle.setToggleState(true, juce::dontSendNotification);
        addAndMakeVisible(direct2DToggle);
        direct2DToggle.onClick = [this]()
            {
                if (auto peer = getPeer())
                {
                    peer->setCurrentRenderingEngine(direct2DToggle.getToggleState() ? 1 : 0);
                }
            };

        updater.addAnimator(animator);
        animator.start();

        setSize(1024, 1024);
    }

    ~Checkerboard() override = default;

    void resized() override
    {
        modeCombo.setBounds(10, 10, 250, 30);
        checkWidthSlider.setBounds(10, 50, 250, 30);
        checkHeightSlider.setBounds(10, 90, 250, 30);
        direct2DToggle.setBounds(10, 120, 150, 30);

        createCachedImage();
    }

    void paint(juce::Graphics& g) override
    {
        double elapsed = 0.0;

        {
            juce::ScopedTimeMeasurement set{ elapsed };
            juce::Graphics::ScopedSaveState state{ g };

	        auto x = std::sin(position * juce::MathConstants<float>::twoPi) * 100.0f;
	        auto checkSize = (float) checkWidthSlider.getValue();
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

        g.setColour(juce::Colours::black.withAlpha(0.9f));
        g.fillRect(5, 5, 280, 150);
        g.setColour(juce::Colours::white.withAlpha(0.9f));
        g.fillRect(getLocalBounds().removeFromBottom(30));
        g.setColour(juce::Colours::black);
        g.setFont(juce::FontOptions{ 30.0f, juce::Font::bold });
        g.drawText(juce::String{ elapsed * 1000.0, 3 } + " ms", getLocalBounds(), juce::Justification::centredBottom);
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
    juce::Slider checkWidthSlider{ juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight };
    juce::Slider checkHeightSlider{ juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight };
    juce::ToggleButton direct2DToggle{ "Direct2D" };

    juce::Image checkerImage;
    juce::RectangleList<float> list;

    void createCachedImage()
    {
        auto checkWidth = (float)checkWidthSlider.getValue();;
        auto checkHeight = (float)checkHeightSlider.getValue();;
        checkerImage = juce::Image(juce::Image::PixelFormat::RGB, juce::roundToInt(checkWidth) * 2, juce::roundToInt(checkHeight) * 2, true);
        juce::Graphics g{ checkerImage };
        g.fillCheckerBoard(checkerImage.getBounds().toFloat(), checkWidth, checkHeight * 2.0f, Colours::lightgrey, Colours::darkgrey);

        list.clear();
        list.ensureStorageAllocated(roundToInt(((float)getWidth() / checkWidth) * ((float)getHeight() / checkHeight)));
        for (float x= 0.0f; x < getWidth(); x += checkWidth * 2.0f)
        {
            for (float y = 0.0f; y < (float)getHeight(); y += checkHeight * 2.0f)
            {
                list.addWithoutMerging({ x, y, checkWidth, checkHeight });
                list.addWithoutMerging({ x + checkWidth, y + checkHeight, checkWidth, checkHeight });
            }
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Checkerboard)
};

