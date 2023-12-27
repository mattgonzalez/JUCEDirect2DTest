/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

  name:             Direct2D Cached Path Creation Test

  dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics, juce_gui_basics
  exporters:        VS2022

  moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1
  defines:

  type:             Component
  mainClass:        CachedPathCreationTest

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

class CachedPathCreationTest : public juce::Component
{
public:
    CachedPathCreationTest()
    {
        addAndMakeVisible(pathSegmentCountLabel);
        pathSegmentCountSlider.setRange({ 4.0, 100000.0 }, 1.0);
        pathSegmentCountSlider.setValue(100.0, juce::dontSendNotification);
        addAndMakeVisible(pathSegmentCountSlider);
        pathSegmentCountSlider.setSkewFactor(0.25);
        pathSegmentCountSlider.onDragEnd = [this] { createPath(); };

        addAndMakeVisible(originalPathSizeLabel);
        originalPathSizeSlider.setRange({ 1.0, 2000.0 }, 1.0);
        originalPathSizeSlider.setValue(10.0, juce::dontSendNotification);
        addAndMakeVisible(originalPathSizeSlider);
        originalPathSizeSlider.onValueChange = [this] { createPath(); };

        addAndMakeVisible(transformScaleLabel);
        yScaleSlider.setRange({ 0.1, 1000.0 }, 0.1);
        yScaleSlider.setValue(1.0, juce::dontSendNotification);
        addAndMakeVisible(yScaleSlider);
        yScaleSlider.onValueChange = [this] { repaint(); };

        xScaleSlider.setRange({ 0.1, 1000.0 }, 0.1);
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

                createPath();
            };
        modeCombo.addItem("fillPath", Mode::fillPath);
        modeCombo.addItem("strokePath", Mode::strokePath);
        addAndMakeVisible(modeCombo);
        modeCombo.setSelectedId(Mode::strokePath, juce::sendNotificationSync);

        setSize(1024, 1024);
    }

    ~CachedPathCreationTest() override = default;

    void resized() override
    {
        {
            modeCombo.setBounds(getWidth() - 130, 10, 120, 30);

            juce::Rectangle<int> r{ getWidth() - 300, modeCombo.getBottom() + 5, 120, 30 };
            originalPathSizeLabel.setBounds(r.withWidth(120));
            originalPathSizeSlider.setBounds(r.withX(originalPathSizeLabel.getRight()).withWidth(getWidth() - originalPathSizeLabel.getRight()));

            r.translate(0, 30);
            pathSegmentCountLabel.setBounds(r.withWidth(120));
            pathSegmentCountSlider.setBounds(r.withX(pathSegmentCountLabel.getRight()).withWidth(getWidth() - pathSegmentCountLabel.getRight()));

            r.translate(0, 30);
            strokeThicknessLabel.setBounds(r.withWidth(120));
            strokeThicknessSlider.setBounds(r.withX(strokeThicknessLabel.getRight()).withWidth(getWidth() - strokeThicknessLabel.getRight()));
        }
        
        transformScaleLabel.setBounds(0, getHeight() - 30, 50, 30);
        yScaleSlider.setBounds(0, 0, 50, transformScaleLabel.getY());
        xScaleSlider.setBounds(transformScaleLabel.getRight(), transformScaleLabel.getY(), getWidth() - transformScaleLabel.getRight(), transformScaleLabel.getHeight());

        createPath();
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
        auto transform = juce::AffineTransform::scale(xScale, yScale).translated(getWidth() * 0.5f, getHeight() * 0.5f);
        switch (modeCombo.getSelectedId())
        {
        case Mode::fillPath:
        {
            g.setColour(juce::Colours::orchid);
            g.fillPath(cachedPath, transform);
            break;
        }

        case Mode::strokePath:
        {
            auto strokeType = createStrokeType();
            g.setColour(juce::Colours::red);
            g.strokePath(cachedPath, strokeType, transform);
            break;
        }
        }

#if 0
        g.setColour(juce::Colours::white);
        juce::Rectangle<int> textR = getLocalBounds().removeFromBottom(75).reduced(20, 0).withHeight(25);

        auto printStat = [&](juce::StatisticsAccumulator<double> const& statistics, StringRef name)
            {
                String line{ name };
                line << statistics.getCount();
                g.drawText(line, textR, juce::Justification::centredLeft);

                line = "Max (msec):";
                line << statistics.getMaxValue() * 1000.0;
                g.drawText(line, textR.withX(250), juce::Justification::centredLeft);

                line = "Avg (msec):";
                line << statistics.getAverage() * 1000.0;
                g.drawText(line, textR.withX(500), juce::Justification::centredLeft);

                line = juce::String{ juce::CharPointer_UTF8("\xcf\x83") };
                line << " (msec):" << statistics.getStandardDeviation();
                g.drawText(line, textR.withX(750), juce::Justification::centredLeft);
                textR.translate(0, textR.getHeight());
            };

        printStat(cachedPath.geometryCreationTime, "Geometry creation: ");
        printStat(cachedPath.filledGeometryRealizationCreationTime, "Filled geometry creation: ");
        printStat(cachedPath.strokedGeometryRealizationCreationTime, "Stroked geometry creation: ");
#endif
    }

    void parentHierarchyChanged() override
    {
        if (auto peer = getPeer())
        {
            peer->setCurrentRenderingEngine(1);
        }
    }

private:
    enum Mode
    {
        fillPath = 1,
        strokePath
    };

    juce::ComboBox modeCombo;
    juce::Label pathSegmentCountLabel{ {}, "Path segments" };
    juce::Label strokeThicknessLabel{ {}, "Stroke thickness" };
    juce::Label originalPathSizeLabel{ {}, "Path original size" };
    juce::Label transformScaleLabel{ {}, "Scale" };
    juce::Slider pathSegmentCountSlider{ juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight };
    juce::Slider strokeThicknessSlider{ juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight };
    juce::Slider originalPathSizeSlider{ juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight };
    juce::Slider xScaleSlider{ juce::Slider::LinearHorizontal, juce::Slider::TextBoxLeft };
    juce::Slider yScaleSlider{ juce::Slider::LinearVertical, juce::Slider::TextBoxBelow };

    //
    // Direct2D resources are generally more expensive to create than they are to draw.
    // If you have an Path you want to use more than once, keep it as a member
    // variable or on the heap. The renderer will create and cache a geometry
    // realization for the Path if you draw the same Path more than once.
    //
    // Drawing a cached geometry realization is much faster than drawing a non-cached Path.
    //
    juce::Path cachedPath;
    juce::Rectangle<int> pathPaintArea;

    void createPath()
    {
        if (getLocalBounds().isEmpty())
        {
            return;
        }

        cachedPath.clear();
        float size = (float)originalPathSizeSlider.getValue();
        auto area = getLocalBounds().toFloat().withSizeKeepingCentre(size, size).withPosition(-size * 0.5f, -size * 0.5f);
        cachedPath.addStar(area.getCentre(), (int)pathSegmentCountSlider.getValue(),
            size * 0.445f,
            size * 0.455f);

        repaint();
    }

    juce::PathStrokeType createStrokeType() const noexcept
    {
        return juce::PathStrokeType{ (float)strokeThicknessSlider.getValue(),
                juce::PathStrokeType::JointStyle::curved,
                juce::PathStrokeType::EndCapStyle::rounded };
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CachedPathCreationTest)
};

