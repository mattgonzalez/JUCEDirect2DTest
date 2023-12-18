/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

  name:             Direct2D Path Draw Test

  dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics, juce_gui_basics
  exporters:        VS2022

  moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1
  defines:

  type:             Component
  mainClass:        PathDrawTest

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

class PathDrawTest : public juce::Component
{
public:
    PathDrawTest()
    {
        modeCombo.onChange = [this]
            {
                bool strokeEnabled = modeCombo.getSelectedId() == Mode::strokePath;
                strokeThicknessSlider.setEnabled(strokeEnabled);
                jointStyleCombo.setEnabled(strokeEnabled);
                endCapStyleCombo.setEnabled(strokeEnabled);

                createPath();
            };
        modeCombo.addItem("fillPath", Mode::fillPath);
        modeCombo.addItem("strokePath", Mode::strokePath);
        addAndMakeVisible(modeCombo);
        modeCombo.setSelectedId(Mode::strokePath, juce::sendNotificationSync);

        addAndMakeVisible(transformToggle);
        transformToggle.onClick = [this]
            {
                transformCombo.setEnabled(transformToggle.getToggleState());
            };

        strokeThicknessSlider.setRange({ 1.0f, 50.0f }, 0.01);
        strokeThicknessSlider.setValue(25.0f, juce::dontSendNotification);
        addAndMakeVisible(strokeThicknessSlider);

        transformCombo.addItem("Translate", TransformType::translate);
        transformCombo.addItem("Scale", TransformType::scale);
        transformCombo.addItem("Shear", TransformType::shear);
        transformCombo.addItem("Rotate", TransformType::rotate);
        addAndMakeVisible(transformCombo);
        transformCombo.setSelectedId(TransformType::translate, juce::dontSendNotification);
        transformCombo.setEnabled(false);

        jointStyleCombo.addItem("Mitered", juce::PathStrokeType::mitered + 1);
        jointStyleCombo.addItem("Curved", juce::PathStrokeType::curved + 1);
        jointStyleCombo.addItem("Beveled", juce::PathStrokeType::beveled + 1);
        jointStyleCombo.setSelectedId(juce::PathStrokeType::beveled, juce::dontSendNotification);
        addAndMakeVisible(jointStyleCombo);

        endCapStyleCombo.addItem("Butt", juce::PathStrokeType::butt + 1);
        endCapStyleCombo.addItem("Square", juce::PathStrokeType::square + 1);
        endCapStyleCombo.addItem("Rounded", juce::PathStrokeType::rounded + 1);
        endCapStyleCombo.setSelectedId(juce::PathStrokeType::rounded, juce::dontSendNotification);
        addAndMakeVisible(endCapStyleCombo);

        setSize(1024, 1024);
    }

    ~PathDrawTest() override = default;

    void resized() override
    {
        juce::Rectangle<int> r{ 10, 10, 250, 30 };
        modeCombo.setBounds(r);

        r.translate(0, 40);
        transformToggle.setBounds(r.withWidth(100));
        transformCombo.setBounds(transformToggle.getBounds().translated(transformToggle.getWidth(), 0));

        r.translate(0, 40);
        strokeThicknessSlider.setBounds(r);

        r.translate(0, 40);
        jointStyleCombo.setBounds(r);

        r.translate(0, 40);
        endCapStyleCombo.setBounds(r);

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
    }

    void animate()
    {
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
    juce::Slider strokeThicknessSlider{ juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight };
    juce::ComboBox jointStyleCombo;
    juce::ComboBox endCapStyleCombo;

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
        auto radius = juce::jmin(area.getWidth(), area.getHeight()) * 0.3f;

        switch (modeCombo.getSelectedId())
        {
        case Mode::fillPath:
        {
            cachedPath.addPolygon(area.getCentre(), 4, radius, juce::MathConstants<float>::pi * 0.25f);
            break;
        }

        case Mode::strokePath:
        {
            juce::Rectangle<float> r = area.withSizeKeepingCentre(radius * 2.0f, radius * 2.0f);

            cachedPath.startNewSubPath(r.getTopLeft());
            cachedPath.lineTo(r.getTopRight());
            cachedPath.lineTo(r.getTopRight() + juce::Point<float>{ 0.0f, r.proportionOfHeight(0.9f) });

            cachedPath.startNewSubPath(r.getBottomRight());
            cachedPath.lineTo(r.getBottomLeft());
            cachedPath.lineTo(r.getBottomLeft() - juce::Point<float>{ 0.0f, r.proportionOfHeight(0.9f) });
            break;
        }
        }
    }

    juce::PathStrokeType createStrokeType() const noexcept
    {
        return juce::PathStrokeType{ (float)strokeThicknessSlider.getValue(),
                (juce::PathStrokeType::JointStyle)(jointStyleCombo.getSelectedId() - 1),
                (juce::PathStrokeType::EndCapStyle)(endCapStyleCombo.getSelectedId() - 1) };
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PathDrawTest)
};

