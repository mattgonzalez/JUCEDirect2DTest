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
        addAndMakeVisible(transformToggle);
        transformToggle.onClick = [this]
            {
                transformCombo.setEnabled(transformToggle.getToggleState());
            };

        pathSegmentCountSlider.setRange({ 100.0f, 100000.0f }, 1.0);
        pathSegmentCountSlider.setValue(100.0f, juce::dontSendNotification);
        addAndMakeVisible(pathSegmentCountSlider);
        pathSegmentCountSlider.onValueChange = [this] { createPath(); };

        strokeThicknessSlider.setRange({ 1.0f, 50.0f }, 0.01);
        strokeThicknessSlider.setValue(5.0f, juce::dontSendNotification);
        addAndMakeVisible(strokeThicknessSlider);
        strokeThicknessSlider.onValueChange = [this] { repaint(); };

        transformCombo.addItem("Translate", TransformType::translate);
        transformCombo.addItem("Scale", TransformType::scale);
        transformCombo.addItem("Shear", TransformType::shear);
        transformCombo.addItem("Rotate", TransformType::rotate);
        addAndMakeVisible(transformCombo);
        transformCombo.setSelectedId(TransformType::translate, juce::dontSendNotification);
        transformCombo.setEnabled(false);

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
        juce::Rectangle<int> r{ 10, 10, 250, 30 };
        modeCombo.setBounds(r);

        r.translate(0, 40);
        transformToggle.setBounds(r.withWidth(100));
        transformCombo.setBounds(transformToggle.getBounds().translated(transformToggle.getWidth(), 0));

        r.translate(0, 40);
        pathSegmentCountSlider.setBounds(r);

        r.translate(0, 40);
        strokeThicknessSlider.setBounds(r);

        createPath();
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black);

        juce::AffineTransform transform;
        if (transformToggle.getToggleState())
        {
            transform = animatedTransform;
        }

        //
        // Fill or stroke the Path member variable to take advantage of
        // geometry caching
        //
        switch (modeCombo.getSelectedId())
        {
        case Mode::fillPath:
        {
            g.setColour(juce::Colours::aquamarine);
            g.fillPath(cachedPath, transform);
            break;
        }

        case Mode::strokePath:
        {
            auto strokeType = createStrokeType();
            g.setColour(juce::Colours::aquamarine);
            g.strokePath(cachedPath, strokeType, transform);
            break;
        }
        }

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
    }

    void animate()
    {
        if (transformToggle.getToggleState() == false)
        {
            return;
        }

        auto now = juce::Time::getMillisecondCounterHiRes();
        auto elapsedSeconds = (now - lastMsec) * 0.001;
        lastMsec = now;

        {
            auto nextPhase = phase + elapsedSeconds * juce::MathConstants<double>::twoPi * 0.2;
             while (nextPhase >= juce::MathConstants<double>::twoPi)
                 nextPhase -= juce::MathConstants<double>::twoPi;

            phase = nextPhase;
        }

        auto position = (float)std::sin(phase);
        auto clampedPosition = juce::jlimit(0.0f, 1.0f, position * 0.5f + 0.5f);
        auto center = getLocalBounds().getCentre().toFloat();
        switch (transformCombo.getSelectedId())
        {
        case TransformType::scale:
            animatedTransform = juce::AffineTransform::scale(clampedPosition, clampedPosition, center.x, center.y);
            break;

        case TransformType::translate:
            animatedTransform = juce::AffineTransform::translation(position * 50.0f, position * 50.0f);
            break;

        case TransformType::shear:
            animatedTransform = juce::AffineTransform::shear(position * 0.5f, position * 0.5f);
            break;

        case TransformType::rotate:
            animatedTransform = juce::AffineTransform::rotation((float)phase, center.x, center.y);
            break;
        }

        repaint();
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

    enum TransformType
    {
        scale = 1,
        translate,
        shear,
        rotate
    };

    juce::VBlankAttachment attachment{ this, [this]() { animate(); } };
    double lastMsec = juce::Time::getMillisecondCounterHiRes();
    juce::AffineTransform animatedTransform;
    double phase = 0.0;

    juce::ComboBox modeCombo;
    juce::ToggleButton transformToggle{ "Transform" };
    juce::ComboBox transformCombo;
    juce::Slider pathSegmentCountSlider{ juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight };
    juce::Slider strokeThicknessSlider{ juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight };

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
        cachedPath.clear();
        auto area = getLocalBounds().toFloat();
        if (area.isEmpty())
        {
            return;
        }

        int numCycles = 1.0f;
        float numSegments = (float)pathSegmentCountSlider.getValue() * 0.5f;
        float pixelsPerSegment = area.getWidth() / numSegments;
        float phasePerSegment = (numCycles * juce::MathConstants<float>::twoPi) / numSegments;
        float upperY = 100.0f + area.getCentreY();
        float amplitude = 50.0f;

        cachedPath.startNewSubPath(0.0f, upperY);
        float angle = 0.0f;
        float x = pixelsPerSegment;
        for (; x < area.getWidth(); x += pixelsPerSegment)
        {
            float y = amplitude * std::sin(angle * juce::MathConstants<float>::twoPi);
            y += upperY;
            cachedPath.lineTo(x, y);
            angle += phasePerSegment;
        }

        float lowerY = area.getCentreY() - 100.0f;
        x -= pixelsPerSegment;
        angle -= phasePerSegment;
        for (; x > 0.0f; x -= pixelsPerSegment)
        {
            float y = amplitude * std::sin(angle * juce::MathConstants<float>::twoPi);
            y += lowerY;
            cachedPath.lineTo(x, y);
            angle -= phasePerSegment;
        }

        cachedPath.closeSubPath();

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

