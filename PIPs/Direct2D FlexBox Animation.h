/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

  name:             Direct2D FlexBox Animation

  dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics, juce_gui_basics
  exporters:        VS2022

  moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1
  defines:

  type:             Component
  mainClass:        FlexBoxAnimation

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

class ComponentTransformAnimator
{
public:
    ComponentTransformAnimator(juce::Component& component_) :
        component(component_)
    {
    }

    void setDestination(juce::Point<float> destination_, double pixelsPerMsec_)
    {
        destination = destination_;
        maxPixelsPerMsec = pixelsPerMsec_;
        if (currentPixelsPerMsec <= 0.0)
        {
            currentPixelsPerMsec = pixelsPerMsec_ * 0.01;
        }
    }

    void animate(double currentTimeMsec)
    {
        auto currentPosition = component.getPosition().toFloat().transformedBy(component.getTransform());
        if (destination.getDistanceFrom(currentPosition) < 1.0f)
        {
            component.setTransform(juce::AffineTransform::translation(destination));
            currentPixelsPerMsec = 0.0;
            return;
        }

        //
        // Move towards the destination
        //
        auto elapsedMsec = currentTimeMsec - lastMsec;
        lastMsec = currentTimeMsec;

        juce::Line<float> vector{ currentPosition, destination };
        auto travelDistance = (float)(currentPixelsPerMsec * elapsedMsec);
        travelDistance = juce::jmin(travelDistance, vector.getLength());
        component.setTransform(juce::AffineTransform::translation(vector.getPointAlongLine(travelDistance)));

        currentPixelsPerMsec *= 1.1;
        currentPixelsPerMsec = juce::jmin(maxPixelsPerMsec, currentPixelsPerMsec);
    }

private:
    juce::Component& component;
    juce::Point<float> destination;
    double lastMsec = juce::Time::getMillisecondCounterHiRes();
    double currentPixelsPerMsec = 0.0;
    double maxPixelsPerMsec = 0.0;
};

class FlexBoxAnimation : public juce::Component
{
public:
    FlexBoxAnimation()
    {
        int constexpr size = 64;

        for (int i = 0; i < 64; ++i)
        {
            auto c = flexComponents.add(new FlexComponent);
            c->index = i;
            addAndMakeVisible(c);

            c->setBounds(0, 0, size, size);

            FlexItem item;
            item.flexBasis = (float)size;
            flexBox.items.add(item);
        }

        flexBox.flexDirection = FlexBox::Direction::row;
        flexBox.flexWrap = FlexBox::Wrap::wrap;
        flexBox.alignContent = FlexBox::AlignContent::stretch;
        flexBox.alignItems = FlexBox::AlignItems::stretch;
        flexBox.justifyContent = FlexBox::JustifyContent::spaceAround;

        setSize(1024, 1024);
    }

    ~FlexBoxAnimation() override = default;

    void resized() override
    {
        flexBox.performLayout(getLocalBounds().reduced(20));

        for (int i = 0; i < flexComponents.size(); ++i)
        {
            auto* c = flexComponents[i];
            auto const& item = flexBox.items[i];
            c->restPosition = item.currentBounds.getPosition();
        }
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour{ 0xff111111 });
    }

    void animate()
    {
        auto now = juce::Time::getMillisecondCounterHiRes();
        auto mousePos = getMouseXYRelative();
        if (getLocalBounds().contains(mousePos))
        {
            for (auto flexComponent : flexComponents)
            {
                flexComponent->animator.setDestination(mousePos.toFloat(), 0.4);
            }
        }
        else
        {
            for (auto flexComponent : flexComponents)
            {
                flexComponent->animator.setDestination(flexComponent->restPosition, 1.0);
            }
        }

        for (auto flexComponent : flexComponents)
        {
            flexComponent->animator.animate(now);
        }

        repaint();
    }

private:
    juce::VBlankAttachment attachment{ this, [this]() { animate(); } };
    double lastMsec = juce::Time::getMillisecondCounterHiRes();
    juce::AffineTransform animatedTransform;

    struct FlexComponent : juce::Component
    {
        void resized() override
        {
            path.clear();

            auto r = getLocalBounds().toFloat();
            path.addStar(r.getCentre(),
                index + 2,
                r.getWidth() * 0.35f,
                r.getWidth() * 0.45f);
        }

        void paint(juce::Graphics& g) override
        {
            g.setColour(juce::Colours::darkcyan);
            g.fillPath(path);

            g.setColour(juce::Colours::black);
            g.drawText(String{ index + 1 }, getLocalBounds(), juce::Justification::centred);
        }

        int index = -1;
        juce::Path path;

        ComponentTransformAnimator animator{ *this };
        juce::Point<float> restPosition;
    };

    juce::OwnedArray<FlexComponent> flexComponents;
    juce::FlexBox flexBox;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FlexBoxAnimation)
};

