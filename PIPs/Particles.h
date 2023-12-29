/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

  name:             Particles

  dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics, juce_gui_basics
  exporters:        VS2022

  moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1
  defines:          

  type:             Component
  mainClass:        Particles

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

class Particles : public Component, public ImagePixelData::Listener
{
public:
    Particles()
    {
        {
            int numPoints = 50;
            starPath.addStar({ spriteSize * 0.5f, spriteSize * 0.5f }, numPoints, spriteSize * 0.15f, spriteSize * 0.45f);

            circlePath.addEllipse(Rectangle<float>{ spriteSize * 0.14f, spriteSize * 0.14f }.withCentre({ spriteSize * 0.5f, spriteSize * 0.5f }));
        }

        createSpriteImages();

        spriteCountLabel.attachToComponent(&spriteCountSlider, true);
        addAndMakeVisible(spriteCountLabel);

        spriteCountSlider.setRange(1.0, 100000.0, 1.0);
        spriteCountSlider.setSkewFactor(0.75);
        spriteCountSlider.setValue(100.0, dontSendNotification);
        spriteCountSlider.onValueChange = [this] { updateSpriteCount(); };
        addAndMakeVisible(spriteCountSlider);

        modeComboBox.addItem("Images", paintImages);
        modeComboBox.addItem("Filled Paths", paintFilledPaths);
        modeComboBox.addItem("Stroked Paths", paintStrokedPaths);
        modeComboBox.setSelectedId(paintImages, dontSendNotification);
        addAndMakeVisible(modeComboBox);

        updateSpriteCount();

        setSize(1024, 1024);
    }

    ~Particles() override = default;

    void updateSpriteCount()
    {
        int desiredCount = (int)spriteCountSlider.getValue();

        if (sprites.size() > desiredCount)
        {
            sprites.removeRange(desiredCount, sprites.size() - desiredCount, true);
        }

        Random random;
        while (sprites.size() < desiredCount)
        {
            auto sprite = std::make_unique<Sprite>();
            sprite->xVelocity = random.nextFloat() * 200.0f;
            sprite->yVelocity = random.nextFloat() * 200.0f;
            sprite->baseScale = 2.0f * random.nextFloat();
            sprite->scale = sprite->baseScale;
            sprites.add(sprite.release());
        }
    }

    void animate()
    {
        auto now = Time::getMillisecondCounterHiRes();
        auto elapsedSeconds = (now - lastMsec) * 0.001;
        lastMsec = now;

        auto bounds = getLocalBounds().toFloat().withTrimmedTop(50.0f);
        auto mousePosition = getMouseXYRelative().toFloat();
        bool mouseOverFlag = bounds.contains(mousePosition);

        for (auto sprite : sprites)
        {
            auto center = sprite->area.getCentre();
            auto mouseVector = Line<float>{ center, mousePosition };
            auto distance = mouseVector.getLength();
            distance = jmax(1.0f, distance);
            distance *= 100.0f;

            if (mouseOverFlag)
            {
                sprite->xVelocity += 100.0f * (center.x - mousePosition.x) / distance;
                sprite->yVelocity += 100.0f * (center.y - mousePosition.y) / distance;
            }

            sprite->xVelocity *= 0.999f;
            sprite->yVelocity *= 0.999f;

            float xDelta = (float)(sprite->xVelocity * elapsedSeconds);
            float yDelta = (float)(sprite->yVelocity * elapsedSeconds);
            auto destination = center.translated(xDelta, yDelta);

            if (bounds.contains(destination))
            {
                sprite->area.translate(xDelta, yDelta);
                continue;
            }

            if (destination.x < bounds.getX())
            {
                sprite->xVelocity = std::abs(sprite->xVelocity);
            }
            else if (destination.x > bounds.getWidth())
            {
                sprite->xVelocity = -std::abs(sprite->xVelocity);
            }

            if (destination.y < bounds.getY())
            {
                sprite->yVelocity = std::abs(sprite->yVelocity);
            }
            else if (destination.y > bounds.getHeight())
            {
                sprite->yVelocity = -std::abs(sprite->yVelocity);
            }

            destination = bounds.getConstrainedPoint(destination);
            sprite->area.setCentre(destination);
        }

        repaint();
    }

    void createSpriteImages()
    {
        images.clear();

        for (auto const& color : colors)
        {
            Image image{ Image::ARGB, spriteSize, spriteSize, true };
            image.getPixelData()->listeners.add(this);
            images.add(image);

            {
                Graphics g{ image };
                g.setColour(color);
                g.fillPath(starPath);
                g.setColour(Colours::darkgrey);
                g.strokePath(starPath, PathStrokeType{ 1.0f });
                g.fillPath(circlePath);
            }
        }
    }

    void paint(Graphics& g) override
    {
        g.setColour(Colours::white);
        g.drawRect(getLocalBounds());

        switch (modeComboBox.getSelectedId())
        {
        case paintImages:
            {
                int index = 0;
	            for (auto sprite : sprites)
	            {
                    auto const& image = images[index];
                    if (image.isValid())
                    {
                        auto point = sprite->area.getPosition();
                        g.drawImageAt(image, (int)point.x, (int)point.y);
                    }
                    index = (index + 1) % images.size();
	            }
	            break;
            }

        case paintFilledPaths:
            {
	            int index = 0;
	            for (auto sprite : sprites)
	            {
	                auto point = sprite->area.getPosition();
	                g.setColour(colors[index]);
	                g.fillPath(starPath, AffineTransform::translation(point));
	                g.setColour(Colours::darkgrey);
	                g.fillPath(circlePath, AffineTransform::translation(point));
	
	                index = (index + 1) % colors.size();
	            }
	            break;
            }

        case paintStrokedPaths:
            {
                int index = 0;
	            for (auto sprite : sprites)
	            {
	                auto point = sprite->area.getPosition();
                    g.setColour(colors[index]);
                    g.strokePath(starPath, PathStrokeType{ 1.5f }, AffineTransform::translation(point));

                    index = (index + 1) % colors.size();
                }
	            break;
            }
        }

        {
            g.setColour(Colours::black);
            std::array<Component*, 2> components{ &spriteCountLabel, &spriteCountSlider };
            for (auto component : components)
            {
                g.fillRect(component->getBounds());
            }
        }
    }

    void resized() override
    {
        spriteCountSlider.setBounds(proportionOfWidth(0.35f), 10, 200, 30);

        modeComboBox.setBounds(spriteCountSlider.getBounds().translated(spriteCountSlider.getWidth() + 10, 0).withWidth(300));
    }

    void imageDataChanged(ImagePixelData*) override {}

    void imageDataBeingDeleted(ImagePixelData*) override
    {
        MessageManager::callAsync([this] { createSpriteImages(); });
    }

private:
    VBlankAttachment attachment{ this, [this]() { animate(); } };
    double lastMsec = Time::getMillisecondCounterHiRes();
    
    Path starPath;
    Path circlePath;
    Array<Colour> const colors{ Colours::aquamarine, Colours::yellow, Colours::orange, Colours::coral };
    Array<Image> images;

    static int constexpr spriteSize = 256;
    struct Sprite
    {
        Rectangle<float> area{ spriteSize, spriteSize };
        float xVelocity;
        float yVelocity;
        float baseScale = 1.0f;
        float scale = 1.0f;
    };
    OwnedArray<Sprite> sprites{};

    Label spriteCountLabel{ {}, "Particles" };
    Slider spriteCountSlider{ Slider::SliderStyle::LinearHorizontal, Slider::TextEntryBoxPosition::TextBoxLeft };

    enum Mode
    {
        paintImages = 1,
        paintFilledPaths,
        paintStrokedPaths
    };
    ComboBox modeComboBox{ "Mode" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Particles)
};
