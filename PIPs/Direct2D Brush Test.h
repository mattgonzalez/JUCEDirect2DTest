/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

  name:             Direct2D Brush Test

  dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics, juce_gui_basics
  exporters:        VS2022

  moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1
  defines:

  type:             Component
  mainClass:        BrushTest

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

class BrushTest : public juce::Component, public juce::ImagePixelData::Listener
{
public:
    BrushTest()
    {
        addAndMakeVisible(direct2DToggle);
        direct2DToggle.onClick = [this] { getPeer()->setCurrentRenderingEngine(direct2DToggle.getToggleState() ? 1 : 0); };

        drawTypeCombo.addItem("fillRect", fillRect);
        drawTypeCombo.addItem("fillRectList", fillRectList);
        drawTypeCombo.addItem("drawRect", drawRect);
        drawTypeCombo.addItem("fillRoundedRectangle", fillRoundedRectangle);
        drawTypeCombo.addItem("drawRoundedRectangle", drawRoundedRectangle);
        drawTypeCombo.addItem("fillEllipse", fillEllipse);
        drawTypeCombo.addItem("drawEllipse", drawEllipse);
        drawTypeCombo.addItem("drawText", drawText);
        addAndMakeVisible(drawTypeCombo);
        drawTypeCombo.setSelectedId(fillRect + 1, juce::dontSendNotification);
        drawTypeCombo.onChange = [this]
            {
                createCachedImages();
                repaint();
            };

        fillTypeCombo.addItem("solid", solidBrush);
        fillTypeCombo.addItem("linearGradientBrush", linearGradientBrush);
        fillTypeCombo.addItem("radialGradientBrush", radialGradientBrush);
        addAndMakeVisible(fillTypeCombo);
        fillTypeCombo.setSelectedId(solidBrush + 1, juce::dontSendNotification);
        fillTypeCombo.onChange = [this]
            {
                createCachedImages();
                repaint();
            };

        transformCombo.addItem("No transform", TransformType::noTransform);
        transformCombo.addItem("Translate", TransformType::translate);
        transformCombo.addItem("Scale", TransformType::scale);
        transformCombo.addItem("Shear", TransformType::shear);
        transformCombo.addItem("Rotate", TransformType::rotate);
        addAndMakeVisible(transformCombo);
        transformCombo.setSelectedId(TransformType::noTransform, juce::dontSendNotification);
        transformCombo.onChange = [this]
            {
                createCachedImages();
                repaint();
            };

        brushTransformCombo.addItem("No brush transform", TransformType::noTransform);
        brushTransformCombo.addItem("Translate brush", TransformType::translate);
        brushTransformCombo.addItem("Scale brush", TransformType::scale);
        brushTransformCombo.addItem("Shear brush", TransformType::shear);
        brushTransformCombo.addItem("Rotate brush", TransformType::rotate);
        addAndMakeVisible(brushTransformCombo);
        brushTransformCombo.setSelectedId(TransformType::translate, juce::dontSendNotification);
        brushTransformCombo.onChange = [this]
            {
                createCachedImages();
                repaint();
            };

        setSize(1024, 512);
    }

    ~BrushTest() override = default;

    void resized() override
    {
        juce::Rectangle<int> r{ 10, 10, 250, 30 };
        direct2DToggle.setBounds(r);

        r.translate(0, 40);
        drawTypeCombo.setBounds(r);

        r.translate(0, 40);
        fillTypeCombo.setBounds(r);

        r.translate(0, 40);
        transformCombo.setBounds(r);

        r.translate(0, 40);
        brushTransformCombo.setBounds(r);

        createCachedImages();
    }

    void paint(juce::Graphics& g) override
    {
        //
        // Check if the cachedImage is still valid; if it's not valid, then the Direct2D bitmap is no longer
        // cached on the GPU and the image data is gone.
        //
        if (softwareImage.isNull() || direct2DImage.isNull())
        {
            return;
        }

        g.drawImageAt(softwareImage, 0, 0);
        g.drawImageAt(direct2DImage, softwareImage.getWidth(), 0);
    }

#if 0
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
        auto imageCenter = cachedImage.getBounds().getCentre().toFloat();
        auto componentCenter = getLocalBounds().getCentre().toFloat();
        switch (transformCombo.getSelectedId())
        {
        case TransformType::scale:
            animatedTransform = juce::AffineTransform::scale(clampedPosition, clampedPosition, imageCenter.x, imageCenter.y);
            break;

        case TransformType::translate:
            animatedTransform = juce::AffineTransform::translation(position * 50.0f, position * 50.0f);
            break;

        case TransformType::shear:
            animatedTransform = juce::AffineTransform::shear(position * 0.5f, position * 0.5f);
            break;

        case TransformType::rotate:
            animatedTransform = juce::AffineTransform::rotation((float)phase, imageCenter.x, imageCenter.y);
            break;
        }

        animatedTransform = animatedTransform.translated(componentCenter - imageCenter);

        repaint();
    }
#endif

    void imageDataChanged(ImagePixelData*) override
    {
    }

    void imageDataBeingDeleted(ImagePixelData*) override
    {
        //
        // Cached image data can be deleted unexpectedly; when that happens, the bitmap data is gone
        // and will need to be recreated.
        //
        // The renderer will call imageDataBeingDeleted for any listeners attached to the ImagePixelData
        //
        // Note that the DXGI adapter may not be available yet when this callback happens so you'll need to
        // defer recreating your images later
        //
        // You can also call Image::isValid() or Image::isNull() to see if the bitmap data is still available.
        //
        // To test this, change the Windows display DPI while your app is running
        //
    }

    void parentHierarchyChanged() override
    {
        if (auto peer = getPeer())
        {
            direct2DToggle.setToggleState(peer->getCurrentRenderingEngine() > 0, juce::dontSendNotification);
        }
    }

private:
    enum DrawType
    {
        fillRect = 1,
        fillRectList,
        drawRect,
        fillRoundedRectangle,
        drawRoundedRectangle,
        fillEllipse,
        drawEllipse,
        drawText
    };

    enum BrushType
    {
        solidBrush = 1,
        linearGradientBrush,
        radialGradientBrush,
        bitmapBrush
    };

    enum TransformType
    {
        noTransform = 1,
        scale,
        translate,
        shear,
        rotate
    };

    //juce::VBlankAttachment attachment{ this, [this]() { animate(); } };
    double lastMsec = juce::Time::getMillisecondCounterHiRes();
    juce::AffineTransform animatedTransform;
    double phase = 0.0;

    juce::ToggleButton direct2DToggle{ "Direct2D" };
    juce::ComboBox drawTypeCombo;
    juce::ComboBox fillTypeCombo;
    juce::ComboBox transformCombo;
    juce::ComboBox brushTransformCombo;

    //
    // Direct2D resources are generally more expensive to create than they are to draw.
    // If you have an Image you want to use more than once, keep that image as a member
    // variable, on the heap, or in the JUCE ImageCache. The renderer will keep the bitmap
    // data cached on the GPU as long as the Image's internal data exists.
    //
    // Drawing a cached image is much faster than drawing a non-cached image.
    //
    juce::Image softwareImage;
    juce::Image direct2DImage;

    void paintImage(Image& image)
    {
        Graphics g{ image };

        g.setColour(juce::Colours::white);
        g.drawRect(image.getBounds());

        FillType fillType;
        switch (fillTypeCombo.getSelectedId())
        {
        case solidBrush:
            fillType = FillType{ juce::Colours::orchid };
            break;

        case linearGradientBrush:
            fillType = FillType{ juce::ColourGradient{ juce::Colours::magenta, image.getWidth() * 0.25f, 0.0f, juce::Colours::cyan, getWidth() * 0.75f, (float)getHeight(), true} };
            break;

        case radialGradientBrush:
            fillType = FillType{ juce::ColourGradient{ juce::Colours::magenta,
                getLocalBounds().getCentre().toFloat(),
                juce::Colours::cyan,
                { getWidth() * 0.25f, 0.0f },
                true} };
            break;
        }

        switch (brushTransformCombo.getSelectedId())
        {
        case TransformType::noTransform:
            break;

        case TransformType::scale:
            fillType.transform = juce::AffineTransform::scale(2.0f, 3.0f, image.getBounds().getCentreX(), image.getBounds().getCentreY());
            break;

        case TransformType::translate:
            fillType.transform = juce::AffineTransform::translation(-100.0f, -200.0f);
            break;

        case TransformType::shear:
            fillType.transform = juce::AffineTransform::shear(1.1f, -0.8f);
            break;

        case TransformType::rotate:
            fillType.transform = juce::AffineTransform::rotation(-0.5f, image.getBounds().getCentreX(), image.getBounds().getCentreY());
            break;
        }

        g.setFillType(fillType);

        juce::AffineTransform transform;
        switch (transformCombo.getSelectedId())
        {
        case TransformType::noTransform:
            break;

        case TransformType::scale:
            transform = juce::AffineTransform::scale(0.5f, 0.5f, image.getBounds().getCentreX(), image.getBounds().getCentreY());
            break;

        case TransformType::translate:
            transform = juce::AffineTransform::translation(50.0f, 50.0f);
            break;

        case TransformType::shear:
            transform = juce::AffineTransform::shear(0.1f, 0.1f);
            break;

        case TransformType::rotate:
            transform = juce::AffineTransform::rotation(0.5f, image.getBounds().getCentreX(), image.getBounds().getCentreY());
            break;
        }
        g.addTransform(transform);

        int rectW = proportionOfWidth(0.35f);
        int rectH = proportionOfHeight(0.2f);
        switch (drawTypeCombo.getSelectedId())
        {
        case fillRect:
            g.fillRect(image.getBounds().withSizeKeepingCentre(rectW, rectH));
            break;

        case fillRectList:
        {
            juce::RectangleList<int> rects;
            auto horizontal = image.getBounds().withSizeKeepingCentre(rectW, rectH);
            rects.add(horizontal.translated(0, image.getHeight() / 4));
            rects.add(horizontal.translated(0, -image.getHeight() / 4));
            g.fillRectList(rects);
            break;
        }

        case drawRect:
            g.drawRect(image.getBounds().withSizeKeepingCentre(rectW, rectH), 2);
            break;

        case fillRoundedRectangle:
            g.fillRoundedRectangle(image.getBounds().withSizeKeepingCentre(rectW, rectH).toFloat(), 10.0f);
            break;

        case drawRoundedRectangle:
            g.drawRoundedRectangle(image.getBounds().withSizeKeepingCentre(rectW, rectH).toFloat(), 10.0f, 10.0f);
            break;

        case fillEllipse:
            g.fillEllipse(image.getBounds().withSizeKeepingCentre(rectW, rectH).toFloat());
            break;

        case drawEllipse:
            g.drawEllipse(image.getBounds().withSizeKeepingCentre(rectW, rectH).toFloat(), 2.0f);
            break;

        case drawText:
            g.setFont(juce::Font{ 100.0f, juce::Font::bold });
            g.drawText("TEXT", image.getBounds(), juce::Justification::centred, true);
            break;
        }

    }

    void createCachedImages()
    {
        softwareImage = Image{ Image::ARGB, getWidth() / 2, getHeight(), true, SoftwareImageType{} };
        paintImage(softwareImage);

        direct2DImage = Image{ Image::ARGB, getWidth() / 2, getHeight(), true, NativeImageType{} };
        paintImage(direct2DImage);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BrushTest)
};

