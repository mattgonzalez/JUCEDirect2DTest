/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

  name:             Fakeform

  dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics, juce_gui_basics, juce_audio_basics, juce_audio_formats, juce_dsp
  exporters:        VS2022

  moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1
  defines:          JUCE_DIRECT2D_METRICS=1

  type:             Component
  mainClass:        FakeformDemo

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

struct Layout
{
    int trackHeight = 50;
    int trackGap = 5;
    int leftPanelWidth = 200;
    int measureWidth = 100;
    int bottomPanelHeight = 100;
    int pluginAreaWidth = 500;
};

struct Randomizer
{
    Randomizer(juce::Random& random_) : random(random_) {}
    juce::Random& random;

    auto makeRandomPoint(juce::Rectangle<int> r)
    {
        return juce::Point<int>{
            random.nextInt(r.getWidth()),
                random.nextInt(r.getHeight()) };
    }

    auto makeRandomPointWithin(juce::Rectangle<int> rect)
    {
        return juce::Point<int>{
            random.nextInt(rect.getWidth()) + rect.getX() - 1,
                random.nextInt(rect.getHeight()) + rect.getY() - 1};
    }

    auto makeRandomUnitRectangle()
    {
        return juce::Rectangle<float>{ random.nextFloat(), random.nextFloat(), random.nextFloat(), random.nextFloat() };
    }

    auto makeRandomRectangle(juce::Rectangle<int> r)
    {
        return juce::Rectangle<int>{
            random.nextInt(r.getWidth()),
                random.nextInt(r.getHeight()),
                random.nextInt(r.getWidth()),
                random.nextInt(r.getHeight()) };
    }

    auto makeRandomOverlappingRectangle(juce::Rectangle<int> other, juce::Rectangle<int> r)
    {
        if (other.isEmpty() || r.isEmpty())
        {
            return juce::Rectangle<int>{};
        }

        auto p1 = makeRandomPointWithin(other);
        auto p2 = makeRandomPoint(r);
        return juce::Rectangle<int>{ p1, p2 };
    }

    auto makeRandomColor()
    {
        return juce::Colour{ (uint8_t)random.nextInt(255),
            (uint8_t)random.nextInt(255),
            (uint8_t)random.nextInt(255),
            (uint8_t)random.nextInt(255)
        };
    }

    auto makeRandomFill(juce::Rectangle<int> r)
    {
        int type = random.nextInt({ 0, 2 });
        switch (type)
        {
        case 0:
        {
            return juce::FillType{ makeRandomColor() };
        }

        case 1:
        {
            juce::ColourGradient gradient
            {
                makeRandomColor(),
                makeRandomPoint(r).toFloat(),
                makeRandomColor(),
                makeRandomPoint(r).toFloat(),
                random.nextBool()
            };

            DBG("gradient " << gradient.getNumColours() << " " << gradient.getColourAtPosition(0.0).toDisplayString(true) << " " << gradient.getColourAtPosition(1.0).toDisplayString(true));
            DBG("    p1:" << gradient.point1.toString() << " -> " << gradient.point2.toString() << "  " << (int)gradient.isRadial);
            return juce::FillType{ gradient };
        }
        }

        return juce::FillType{};
    }


    auto makeRandomString()
    {
        auto length = random.nextInt(100);

        juce::String text;
        for (int i = 0; i < length; ++i)
        {
            text += (char)random.nextInt({ 32, 127 });
        }

        return text;
    }

    auto makeRandomImage(juce::Rectangle<int> r, juce::ImageType&& imageType)
    {
        auto format = (juce::Image::PixelFormat)random.nextInt({ 1, 4 });
        auto image = juce::Image{ imageType.create(format, r.getWidth(), r.getHeight(), true) };
        {
            juce::Graphics ig{ image };
            ig.fillCheckerBoard(image.getBounds().toFloat(),
                random.nextFloat() * (float)image.getWidth(),
                random.nextFloat() * (float)image.getHeight(),
                makeRandomColor(), makeRandomColor());
            ig.setColour(makeRandomColor());
            ig.drawRect(r);
        }

        return image;
    }
};

struct BigMeter : public Component
{
    ~BigMeter() override = default;

    void paint(Graphics& g) override
    {
        g.setGradientFill(gradient);

        int numChannels = 4;
        Rectangle<float> r{ 5.0f, 5.0f, 0.0f, float(getHeight() - 10) / (float)numChannels };
        for (int channel = 0; channel < numChannels; ++channel)
        {
            auto x = float(std::abs(std::sin(phase.phase + (double)(channel * MathConstants<double>::pi * 0.25))));
            g.fillRoundedRectangle(r.withWidth(x * (float)getWidth()), r.getHeight() * 0.5f);
            r.translate(0.0f, r.getHeight());
        }
    }

    void update(double elapsedSeconds)
    {
        double const radiansPerSecond = MathConstants<double>::twoPi * 0.5;
        phase.advance(radiansPerSecond * elapsedSeconds);

        repaint();
    }

    void resized() override
    {
        gradient = ColourGradient::horizontal(Colours::darkgrey, Colours::magenta, getLocalBounds().toFloat());
    }

    dsp::Phase<double> phase;
    ColourGradient gradient;
};

class MeasureComponent : public Component
{
public:
    MeasureComponent(Random& random_, Layout const& layout_, int index_) :
        layout(layout_),
        trackIndex(index_)
    {
        setName("clip " + String(trackIndex));
        Randomizer randomizer{ random_ };

        setOpaque(false);
        setRepaintsOnMouseActivity(true);
    }

    ~MeasureComponent() override = default;

    void paint(Graphics& g) override
    {
        g.setColour(backgroundColor);
        g.fillRoundedRectangle(getLocalBounds().toFloat(), (float)getHeight() * 0.1f);

        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.fillRectList(rects);

        auto textR = Rectangle<float>{ 0.0f, (float)getHeight() - 20.0f, 50.0f, 20.0f };
        g.fillRoundedRectangle(textR, 5.0f);
        g.setColour(juce::Colours::white);
        g.drawText(String{ repaintCount++ }, textR, Justification::centred);

        if (isMouseOver(true))
        {
            g.setColour(juce::Colours::white);
            g.drawRoundedRectangle(getLocalBounds().toFloat(), (float)getHeight() * 0.1f, 5.0f);
        }
    }

    void resized() override
    {
        rects.clear();

        float size = 4.0f;
        float step = 4.5f;
        for (float y = 0.0f; y < (float)getHeight(); y += step)
        {
            for (float x = 0.0f; x < (float)getWidth(); x += step)
            {
                rects.addWithoutMerging(Rectangle<float>{ x, y, size, size });
            }
        }
    }

    Layout const& layout;
    juce::Colour backgroundColor;
    int trackIndex = 0;
    //DropShadowEffect shadow;
    RectangleList<float> rects;
    double lastMsec = Time::getMillisecondCounterHiRes();
    float xOffset = 0;
    double pixelsPerSecond = 100.0;
    int repaintCount = 0;
};

class FakePluginComponent : public Component
{
public:
    FakePluginComponent(Layout const& layout_) :
        layout(layout_)
    {
        setOpaque(false);
    }

    ~FakePluginComponent() override = default;

    void paint(Graphics& g) override
    {
        g.addTransform(transform);
        g.setColour(juce::Colours::aliceblue);
        g.fillPath(p);
        g.setColour(juce::Colours::green);
        g.strokePath(p, PathStrokeType{ 2.0f });
    }

    void resized() override
    {
        p.clear();
        p.addStar(getLocalBounds().toFloat().getCentre(), 6, (float)getWidth() * 0.1f, (float)getWidth() * 0.2f);
    }

    void update(double elapsedSeconds)
    {
        double const radiansPerSecond = MathConstants<double>::twoPi * 0.5;

        phase.advance(radiansPerSecond * elapsedSeconds);

        transform = AffineTransform::rotation((float)phase.phase, getLocalBounds().toFloat().getCentreX(), getLocalBounds().toFloat().getCentreY());

        repaint();
    }

    Layout const& layout;
    Path p;
    juce::dsp::Phase<double> phase;
    AffineTransform transform;
};

class TrackMeasures : public Component
{
public:
    TrackMeasures(Random& random_, Layout const& layout_, int index_)
        : random(random_),
        layout(layout_),
        trackIndex(index_)
    {
        setOpaque(false);

        Randomizer randomizer{ random_ };
        for (int i = 0; i < 30; ++i)
        {
            auto measure = std::make_unique<MeasureComponent>(random, layout_, i);
            measure->backgroundColor = randomizer.makeRandomColor().withAlpha(0.5f);

            addAndMakeVisible(*measure);
            measures.add(std::move(measure));
        }
    }

    void paint(Graphics&) override
    {
    }

    void resized() override
    {
        int x = 0;
        int w = layout.measureWidth;
        for (auto measure : measures)
        {
            measure->setBounds(x, 0, w, getHeight());
            x += w;
        }
    }

    Random& random;
    Layout const& layout;
    int trackIndex = 0;
    OwnedArray<MeasureComponent> measures;
};

class TrackPlugins : public Component
{
public:
    TrackPlugins(Random& random_, Layout const& layout_, int index_)
        : random(random_),
        layout(layout_),
        trackIndex(index_)
    {
        setOpaque(false);

        for (int i = 0; i < 5; ++i)
        {
            auto fakePlugin = std::make_unique<FakePluginComponent>(layout_);
            addAndMakeVisible(*fakePlugin);
            fakePlugins.add(std::move(fakePlugin));
        }

        addAndMakeVisible(bigMeter);
    }

    void paint(Graphics& g) override
    {
        g.setColour(juce::Colours::hotpink);
        g.drawRect(getLocalBounds());
    }

    void resized() override
    {
        int x = 0;
        int w = getWidth() / (fakePlugins.size() + 1);

        for (auto fakePlugin : fakePlugins)
        {
            fakePlugin->setBounds(x, 0, w, getHeight());
            x += w;
        }

        bigMeter.setBounds(x, 0, w, getHeight());
    }

    void update(double elapsedSeconds)
    {
        for (auto fakePlugin : fakePlugins)
        {
            fakePlugin->update(elapsedSeconds);
        }

        bigMeter.update(elapsedSeconds);
    }

    Random& random;
    Layout const& layout;
    int trackIndex = 0;
    OwnedArray<FakePluginComponent> fakePlugins;
    BigMeter bigMeter;
};

class TrackComponent : public Component
{
public:
    TrackComponent(Random& random_, Layout const& layout_, int index_) :
        random(random_),
        layout(layout_),
        measures(random_, layout_, index_),
        plugins(random_, layout_, index_),
        trackIndex(index_)
    {
        setName("track " + String(trackIndex));
        setOpaque(false);

        addAndMakeVisible(measures);
        addAndMakeVisible(plugins);
    }

    void paint(Graphics& g) override
    {
        g.fillAll(juce::Colour::greyLevel(0.2f).withAlpha(0.5f));
    }

    void resized() override
    {
        measures.setBounds(getLocalBounds().withWidth(getWidth() - layout.pluginAreaWidth));
        plugins.setBounds(measures.getRight(), measures.getY(), getWidth() - measures.getRight(), measures.getHeight());
    }

    void update(double elapsedSeconds)
    {
        plugins.update(elapsedSeconds);
    }

    Random& random;
    Layout const& layout;
    TrackMeasures measures;
    TrackPlugins plugins;
    int trackIndex = 0;
    GlowEffect glow;
};

class Overlay : public Component
{
public:
    Overlay(Layout const& layout_) : layout(layout_)
    {
        setInterceptsMouseClicks(false, false);
    }

    ~Overlay() override = default;

    void paint(Graphics& g) override
    {
        g.setColour(juce::Colours::cyan);
        Rectangle<int> r{ cursorX - 2, 0, 4, getHeight() };
        if (getLocalBounds().contains(r))
        {
            g.fillRect(r);
            updateArea = r;
        }
        else
        {
            updateArea = {};
        }
    }

    void resized() override
    {
    }

    void update()
    {
        auto mousePos = getMouseXYRelative();
        if (mousePos.x != lastCursorX)
        {
            cursorX = mousePos.x;
            lastCursorX = cursorX;

            updateArea = updateArea.getUnion(Rectangle<int>{ cursorX - 2, 0, 4, getHeight() });
            if (updateArea.intersects(getLocalBounds()))
            {
                auto updateAreaX = cursorX - cursorX % layout.measureWidth;
                repaint(updateArea);
                getParentComponent()->repaint(updateAreaX, 0, layout.measureWidth, getHeight());
            }
        }
    }

    Layout const& layout;
    Rectangle<int> updateArea;
    int cursorX = 0;
    int lastCursorX = 0;
};

class TracksComponent : public Component
{
public:
    TracksComponent(Random& random_, Layout const& layout_) : layout(layout_), overlay(layout_)
    {
        setOpaque(false);

        for (int i = 0; i < 20; ++i)
        {
            auto track = std::make_unique<TrackComponent>(random_, layout_, i);
            addAndMakeVisible(*track);
            tracks.add(std::move(track));
        }

        addAndMakeVisible(overlay);
    }

    ~TracksComponent() override = default;

    void paint(Graphics&) override
    {
    }

    void paintOverChildren(Graphics& g) override
    {
        g.setColour(juce::Colour{ 0xffc0c0c0 });
        g.fillRect(playHeadPosition, 0.0f, 1.0f, (float)getHeight());
    }

    void resized() override
    {
        int trackHeight = layout.trackHeight;
        int y = layout.trackGap;
        for (auto track : tracks)
        {
            track->setBounds(0, y, getWidth(), trackHeight);
            y += trackHeight + layout.trackGap;
        }

        overlay.setBounds(getLocalBounds());
    }

    void update(double elapsedSeconds)
    {
        float playHeadPixelsPerSecond = 100.0f;

        int x = (int)playHeadPosition - (int)playHeadPosition % layout.measureWidth;
        auto r = Rectangle<int>{ x, 0, layout.measureWidth * 2, getHeight() };
        if (r.intersects(getLocalBounds()))
        {
            repaint(r);
        }

        playHeadPosition += (float)(playHeadPixelsPerSecond * elapsedSeconds);
        if (playHeadPosition >= (float)getWidth())
        {
            playHeadPosition = 0.0f;
        }

        for (auto track : tracks)
        {
            track->update(elapsedSeconds);
        }

        overlay.update();
    }

    Layout const& layout;
    OwnedArray<TrackComponent> tracks;
    Overlay overlay;
    float playHeadPosition = 0.0f;
};


class BottomPanel : public Component
{
public:
    BottomPanel()
    {
        setOpaque(false);

        addAndMakeVisible(bigMeter);
    }

    ~BottomPanel() override = default;

    void paint(Graphics& g) override
    {
        g.fillAll(juce::Colours::darkgrey);
        g.setColour(juce::Colours::lightgrey);
        g.drawRect(getLocalBounds());
    }

    void resized() override
    {
        bigMeter.setBounds(getLocalBounds().removeFromRight(getWidth() / 2));
    }

    void update(double elapsedSeconds)
    {
        bigMeter.update(elapsedSeconds);
    }

private:
    BigMeter bigMeter;
};

class FakeformDemo : public Component
{
public:
    FakeformDemo() :
        tracks{ random, layout }
    {
        setName("Fakeform");
        setOpaque(true);

        addAndMakeVisible(tracks);
        addAndMakeVisible(bottomPanel);

        setSize(800, 600);
    }

    ~FakeformDemo() override = default;

    void update()
    {
        auto now = Time::getHighResolutionTicks();
        if (Time::highResolutionTicksToSeconds(now - lastTicks) > 0.016)
        {
            auto elapsedSeconds = Time::highResolutionTicksToSeconds(now - lastTicks);
            tracks.update(elapsedSeconds);
            bottomPanel.update(elapsedSeconds);
            lastTicks = now;
        }

        auto mousePos = getMouseXYRelative();

        if (!callout)
        {
            calloutContent.setSize(300, 400);
            callout = std::make_unique<juce::CallOutBox>(calloutContent, getLocalBounds(), this);
            addAndMakeVisible(callout.get());
        }

        animator.animateComponent(callout.get(), Rectangle<int>{ mousePos.x, mousePos.y, 300, 400 }, 1.0, 1000, false, 100.0, 1000.0);
    }

    void paint(Graphics& g) override
    {
        //DBG("\nfakeform paint");

        if (getParentComponent()->isOpaque())
        {
            g.fillAll(juce::Colours::black);
        }
        //
        //g.fillCheckerBoard(getLocalBounds().toFloat(), 20.0f, 20.0f, juce::Colours::white, juce::Colours::lightgrey);

        RectangleList<float> rects;
        Rectangle<float> r{ 0.0f, 0.0f, 1.0f, (float)getHeight() };

        for (float x = (float)tracks.getX(); x < (float)tracks.getRight(); x += (float)layout.measureWidth * 0.125f)
        {
            rects.addWithoutMerging(r.withX(x));
        }
        g.setColour(juce::Colour{ 0xff202020 });
        g.fillRectList(rects);

        rects.clear();
        for (float x = (float)tracks.getX(); x < (float)tracks.getRight(); x += (float)layout.measureWidth * 0.5f)
        {
            rects.addWithoutMerging(r.withX((float)x));
        }
        g.setColour(juce::Colour{ 0xff404040 });
        g.fillRectList(rects);
    }

    void resized() override
    {
        tracks.setBounds(layout.leftPanelWidth, 0, getWidth() - layout.leftPanelWidth, getHeight() - layout.bottomPanelHeight);
        bottomPanel.setBounds(Rectangle<int>{ 0, getHeight() - layout.bottomPanelHeight, getWidth(), layout.bottomPanelHeight }.reduced(10));
    }

private:
    juce::VBlankAttachment vblank{ this, [this] { update(); } };
    int64 lastTicks = Time::getHighResolutionTicks();
    Random random{ 123456 };

    const Layout layout;

    juce::Label calloutContent{ "CALLOUT", "CALLOUT" };
    std::unique_ptr<juce::CallOutBox> callout;
    ComponentAnimator animator;
    TracksComponent tracks;
    BottomPanel bottomPanel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FakeformDemo)
};
