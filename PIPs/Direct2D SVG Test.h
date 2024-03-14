/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

  name:             Direct2D SVG Path Test

  dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics, juce_gui_basics
  exporters:        VS2022

  moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1
  defines:          JUCE_DIRECT2D_METRICS=1

  type:             Component
  mainClass:        SVGPathTest

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

class SVGPathTest : public juce::Component, public juce::FileDragAndDropTarget
{
public:
    SVGPathTest()
    {
        addAndMakeVisible(transformScaleLabel);

        yScaleSlider.setRange({ 0.1, 1000.0 }, 0.1);
        yScaleSlider.setSkewFactor(0.5f);
        yScaleSlider.setValue(1.0, juce::dontSendNotification);
        addAndMakeVisible(yScaleSlider);
        yScaleSlider.onValueChange = [this] { repaint(); };

        xScaleSlider.setRange({ 0.1, 1000.0 }, 0.1);
        xScaleSlider.setSkewFactor(0.5f);
        xScaleSlider.setValue(1.0, juce::dontSendNotification);
        addAndMakeVisible(xScaleSlider);
        xScaleSlider.onValueChange = [this] { repaint(); };

        addAndMakeVisible(strokeThicknessLabel);
        strokeThicknessSlider.setRange({ 1.0, 50.0 }, 0.01);
        strokeThicknessSlider.setValue(1.0, juce::dontSendNotification);
        addAndMakeVisible(strokeThicknessSlider);
        strokeThicknessSlider.onValueChange = [this] { repaint(); };

        modeCombo.onChange = [this]
            {
                bool strokeEnabled = modeCombo.getSelectedId() == Mode::strokePath;
                strokeThicknessSlider.setEnabled(strokeEnabled);
            };
        modeCombo.addItem("fillPath", Mode::fillPath);
        modeCombo.addItem("strokePath", Mode::strokePath);
        addAndMakeVisible(modeCombo);
        modeCombo.setSelectedId(Mode::strokePath, juce::sendNotificationSync);

        cacheToggle.setToggleState(true, dontSendNotification);
        addAndMakeVisible(cacheToggle);
        cacheToggle.onClick = [this]()
            {
                path.setCacheEnabled(cacheToggle.getToggleState());
                repaint();
            };

        setSize(1024, 1024);
    }

    ~SVGPathTest() override = default;

    void resized() override
    {
        {
            modeCombo.setBounds(getWidth() - 130, 10, 120, 30);

            juce::Rectangle<int> r{ getWidth() - 300, modeCombo.getBottom() + 5, 120, 30 };
            strokeThicknessLabel.setBounds(r.withWidth(120));
            strokeThicknessSlider.setBounds(r.withX(strokeThicknessLabel.getRight()).withWidth(getWidth() - strokeThicknessLabel.getRight()));

            r.translate(0, 30);
            cacheToggle.setBounds(strokeThicknessSlider.getX(), r.getY(), 120, 30);
        }

        transformScaleLabel.setBounds(0, getHeight() - 30, 50, 30);
        yScaleSlider.setBounds(0, 0, 50, transformScaleLabel.getY());
        xScaleSlider.setBounds(transformScaleLabel.getRight(), transformScaleLabel.getY(), getWidth() - transformScaleLabel.getRight(), transformScaleLabel.getHeight());
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black);

        //
        // Fill or stroke the Path member variable to take advantage of
        // geometry caching
        //
        auto xScale = (float)xScaleSlider.getValue();
        auto yScale = (float)yScaleSlider.getValue();
        auto localBounds = getLocalBounds().toFloat();
        auto pathBounds = path.getBounds();
        auto transform = juce::AffineTransform::translation(getLocalBounds().getCentre().toFloat() - pathBounds.getCentre()).
            scaled(xScale, yScale, localBounds.getCentreX(), localBounds.getCentreY());

        switch (modeCombo.getSelectedId())
        {
        case Mode::fillPath:
        {
            g.setColour(juce::Colours::orchid);
            g.fillPath(path, transform);
            break;
        }

        case Mode::strokePath:
        {
            auto strokeType = createStrokeType();
            g.setColour(juce::Colours::cyan);
            g.strokePath(path, strokeType, transform);
            break;
        }
        }
    }

    void animate()
    {
        repaint();
    }

    bool isInterestedInFileDrag(const StringArray& files) override
    {
        File file{ files[0] };
        return file.hasFileExtension("svg");
    }

    void filesDropped(const StringArray& files, int, int) override
    {
        File file{ files[0] };
        auto svg = juce::Drawable::createFromImageFile(file);
        if (svg)
        {
            path = svg->getOutlineAsPath();
        }
    }

private:
    enum Mode
    {
        fillPath = 1,
        strokePath
    };

    juce::ComboBox modeCombo;
    juce::Label strokeThicknessLabel{ {}, "Stroke thickness" };
    juce::Label transformScaleLabel{ {}, "Scale" };
    juce::Slider strokeThicknessSlider{ juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight };
    juce::Slider xScaleSlider{ juce::Slider::LinearHorizontal, juce::Slider::TextBoxLeft };
    juce::Slider yScaleSlider{ juce::Slider::LinearVertical, juce::Slider::TextBoxBelow };
    juce::ToggleButton cacheToggle{ "Cached" };
    juce::VBlankAttachment attachment{ this, [this]() { animate(); } };
    double lastMsec = juce::Time::getMillisecondCounterHiRes();

    //
    // Direct2D resources are generally more expensive to create than they are to draw.
    // If you have an Path you want to use more than once, keep it as a member
    // variable or on the heap. The renderer will create and cache a geometry
    // realization for the Path if you draw the same Path more than once.
    //
    // Drawing a cached geometry realization is much faster than drawing a non-cached Path.
    //
    juce::Path path;
    juce::Rectangle<int> pathPaintArea;

    juce::PathStrokeType createStrokeType() const noexcept
    {
        return juce::PathStrokeType{ (float)strokeThicknessSlider.getValue(),
                juce::PathStrokeType::JointStyle::curved,
                juce::PathStrokeType::EndCapStyle::rounded };
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SVGPathTest)
};

