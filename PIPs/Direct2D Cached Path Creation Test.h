/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

  name:             Direct2D Cached Path Creation Test

  dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics, juce_gui_basics
  exporters:        VS2022

  moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1
  defines:          JUCE_DIRECT2D_METRICS=1

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
        setBufferedToImage(true);

        addAndMakeVisible(pathSegmentCountLabel);
        pathSegmentCountSlider.setRange({ 4.0, 100000.0 }, 1.0);
        pathSegmentCountSlider.setValue(1000.0, juce::dontSendNotification);
        addAndMakeVisible(pathSegmentCountSlider);
        pathSegmentCountSlider.setSkewFactor(0.25);
        pathSegmentCountSlider.onValueChange = [this] { createPath(); };

        addAndMakeVisible(originalPathSizeLabel);
        originalPathSizeSlider.setRange({ 1.0, 2000.0 }, 1.0);
        originalPathSizeSlider.setValue(500.0, juce::dontSendNotification);
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

        cacheToggle.setToggleState(path.isCacheEnabled(), dontSendNotification);
        addAndMakeVisible(cacheToggle);
        cacheToggle.onClick = [this]()
            {
                path.setCacheEnabled(cacheToggle.getToggleState());
                repaint();
            };

        addAndMakeVisible(direct2DToggle);
        direct2DToggle.onClick = [this]()
            {
                if (auto peer = getPeer())
                {
                    peer->setCurrentRenderingEngine(direct2DToggle.getToggleState() ? 1 : 0);
                }
            };

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

            r.translate(0, 30);
            cacheToggle.setBounds(strokeThicknessSlider.getX(), r.getY(), 120, 30);
        }

        transformScaleLabel.setBounds(0, getHeight() - 30, 50, 30);
        yScaleSlider.setBounds(0, 0, 50, transformScaleLabel.getY());
        xScaleSlider.setBounds(transformScaleLabel.getRight(), transformScaleLabel.getY(), getWidth() - transformScaleLabel.getRight(), transformScaleLabel.getHeight());

        direct2DToggle.setBounds(getWidth() / 2 - 60, 10, 120, 30);

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
            g.fillPath(path, transform);
            break;
        }

        case Mode::strokePath:
        {
            auto strokeType = createStrokeType();
            g.setColour(juce::Colours::red);
            g.strokePath(path, strokeType, transform);
            break;
        }
        }
    }

    void animate()
    {
        repaint();
    }

    void parentHierarchyChanged() override
    {
        if (auto peer = getPeer())
        {
            direct2DToggle.setToggleState(peer->getCurrentRenderingEngine() > 0, dontSendNotification);
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
    juce::ToggleButton cacheToggle{ "Cached" };
    juce::ToggleButton direct2DToggle{ "Direct2D" };
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
    juce::Path strokedPath;
    juce::Path flattenedPath;
    juce::Rectangle<int> pathPaintArea;

    void createPath()
    {
        if (getLocalBounds().isEmpty())
        {
            return;
        }

        path = {};
        float size = (float)originalPathSizeSlider.getValue();
        auto area = getLocalBounds().toFloat().withSizeKeepingCentre(size, size).withPosition(-size * 0.5f, -size * 0.5f);

        int numCycles = 12;
        float angle = 0.0f;
        float angleStep = juce::MathConstants<float>::twoPi / (float)pathSegmentCountSlider.getValue();
        float radius = area.getWidth() * 0.5f;
        float amplitude = radius * 0.25f;
        path.startNewSubPath(area.getCentreX(), area.getCentreY() - radius);
        angle += angleStep;

        while (angle < juce::MathConstants<float>::twoPi)
        {
            auto distance = amplitude * std::sin(angle * (float)numCycles);
            auto point = area.getCentre().getPointOnCircumference(distance + radius, angle);
            path.lineTo(point);
            angle += angleStep;
        }
        path.closeSubPath();

        cacheToggle.setToggleState(path.isCacheEnabled(), dontSendNotification);

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

