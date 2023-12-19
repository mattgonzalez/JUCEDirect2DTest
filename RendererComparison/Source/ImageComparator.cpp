#include "ImageComparator.h"

ImageComparator::ImageComparator()
{
    setSize(600, 400);
    setWantsKeyboardFocus(true);

    addAndMakeVisible(ySlider);
    ySlider.setRange(-5.0, 5.0, 0.001);
    ySlider.onValueChange = [this]() { transformY = ySlider.getValue(); transform(); };
    addAndMakeVisible(scaleSlider);
    scaleSlider.setRange(0.8,1.2, 0.001);
    scaleSlider.onValueChange = [this]() { transformScale = scaleSlider.getValue(); transform(); };
}

ImageComparator::~ImageComparator()
{
}

void ImageComparator::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    juce::Image* images[] = { &sourceImage1, &sourceImage2, &redComparison, &greenComparison, &blueComparison, &alphaComparison };
    juce::StatisticsAccumulator<double>* stats[] = { nullptr, nullptr, &redStats, &greenStats, &blueStats, &alphaStats };
    juce::Rectangle<int> r{ 0, 0, getWidth() / 3, getHeight() / 2 }, lastR;
    int index = 0;
    for (auto const& image : images)
    {
        if (image->isValid())
        {
            g.drawImageWithin(*image, r.getX(), r.getY(), r.getWidth(), r.getHeight(), juce::RectanglePlacement::centred);

            if (stats[index] && stats[index]->getCount() > 0)
            {
                juce::String text;
                text << "Average: " << stats[index]->getAverage() << juce::newLine;
                text << "Max: " << stats[index]->getMaxValue() << juce::newLine;
                text << "Std.dev: " << stats[index]->getStandardDeviation() << juce::newLine;

                g.setColour(juce::Colours::darkgrey);
                g.fillRect(juce::Rectangle{ r.getX() + 40, r.getY() + 20, 200, 100 });

                g.setColour(juce::Colours::white);
                g.drawMultiLineText(text, r.getX() + 50, r.getY() + 35, r.getWidth(), juce::Justification::centredLeft);
            }
        }
        else
        {
            g.setColour(juce::Colours::white);
            g.drawText("Nope", r, juce::Justification::centred);
        }

        lastR = r;

        r.setX(r.getRight());
        if (r.getX() >= proportionOfWidth(0.75f))
        {
            r.setX(0);
            r.setY(r.getBottom());
        }

        ++index;
    }

    g.setOpacity(0.5f);
    r = lastR;
    g.drawImageWithin(sourceImage2, r.getX(), r.getY(), r.getWidth(), r.getHeight(), juce::RectanglePlacement::centred);

}

void ImageComparator::resized()
{
    ySlider.setBounds(10, 10, 300, 50);
    scaleSlider.setBounds(10, 70, 300, 50);
}

void ImageComparator::transform()
{
    if (original.isValid())
    {
        sourceImage2 = original.createCopy();
        {
            sourceImage2.clear(sourceImage2.getBounds());

            juce::Graphics g{ sourceImage2 };
            g.drawImageTransformed(original, juce::AffineTransform::translation(0.0f, transformY).scaled(1.0f, transformScale, sourceImage2.getWidth() * 0.5f, sourceImage2.getHeight() * 0.5f));
        }

        compare();
    }
}


void ImageComparator::compare(juce::Image softwareRendererSnapshot, juce::Image direct2DRendererSnapshot)
{
    if (softwareRendererSnapshot.isNull() || direct2DRendererSnapshot.isNull())
    {
        return;
    }

    original = direct2DRendererSnapshot.createCopy();

    sourceImage1 = softwareRendererSnapshot.createCopy();
    sourceImage2 = direct2DRendererSnapshot.createCopy();

    compare();
}


void ImageComparator::compare()
{
    if (sourceImage1.isNull() || sourceImage2.isNull())
    {
        return;
    }

    redComparison = juce::Image{ juce::Image::ARGB, sourceImage1.getWidth(), sourceImage1.getHeight(), true };
    blueComparison = juce::Image{ juce::Image::ARGB, sourceImage1.getWidth(), sourceImage1.getHeight(), true };
    greenComparison = juce::Image{ juce::Image::ARGB, sourceImage1.getWidth(), sourceImage1.getHeight(), true };
    alphaComparison = juce::Image{ juce::Image::ARGB, sourceImage1.getWidth(), sourceImage1.getHeight(), true };

    {
        juce::Image::BitmapData softwareRendererSnapshotData{ sourceImage1, juce::Image::BitmapData::readOnly };
        juce::Image::BitmapData direct2DRendererSnapshotData{ sourceImage2, juce::Image::BitmapData::readOnly };
        juce::Image::BitmapData redData{ redComparison, juce::Image::BitmapData::writeOnly };
        juce::Image::BitmapData greenData{ greenComparison, juce::Image::BitmapData::writeOnly };
        juce::Image::BitmapData blueData{ blueComparison, juce::Image::BitmapData::writeOnly };
        juce::Image::BitmapData alphaData{ alphaComparison, juce::Image::BitmapData::writeOnly };

        uint8_t constexpr zero = 0;
        float constexpr scale = 255.0f;
        int width = juce::jmin(softwareRendererSnapshotData.width, direct2DRendererSnapshotData.width);
        int height = juce::jmin(softwareRendererSnapshotData.height, direct2DRendererSnapshotData.height);
        for (int x = 0; x < width; ++x)
        {
            for (int y = 0; y < height; ++y)
            {
                auto c1 = softwareRendererSnapshotData.getPixelColour(x, y);
                auto c2 = direct2DRendererSnapshotData.getPixelColour(x, y);

                {
                    auto deltaFloat = std::abs(c1.getFloatRed() - c2.getFloatRed());
                    redStats.addValue(deltaFloat);
                    auto deltaInt = (uint8_t)(int)(scale * deltaFloat);
                    juce::Colour c{ deltaInt, zero, zero, 1.0f };
                    redData.setPixelColour(x, y, c);
                }

                {
                    auto deltaFloat = std::abs(c1.getFloatGreen() - c2.getFloatGreen());
                    greenStats.addValue(deltaFloat);
                    auto deltaInt = (uint8_t)(int)(scale * deltaFloat);
                    juce::Colour c{ zero, deltaInt, zero, 1.0f };
                    greenData.setPixelColour(x, y, c);
                }

                {
                    auto deltaFloat = std::abs(c1.getFloatBlue() - c2.getFloatBlue());
                    blueStats.addValue(deltaFloat);
                    auto deltaInt = (uint8_t)(int)(scale * deltaFloat);
                    juce::Colour c{ zero, zero, deltaInt, 1.0f };
                    blueData.setPixelColour(x, y, c);
                }

                {
                    auto brightnessDelta = std::abs(c1.getPerceivedBrightness() - c2.getPerceivedBrightness());
                    alphaData.setPixelColour(x, y, juce::Colour::greyLevel(brightnessDelta));
                }
            }
        }
    }

    repaint();
}

void ImageComparator::compare(juce::Component& sourceComponent)
{
    auto imageBounds = sourceComponent.getLocalBounds();
    juce::SoftwareImageType softwareImageType;
    juce::Image softwareImage{ softwareImageType.create(juce::Image::ARGB, imageBounds.getWidth(), imageBounds.getHeight(), true) };
    {
        juce::Graphics g{ softwareImage };
        sourceComponent.paintEntireComponent(g, false);
    }

    juce::NativeImageType nativeImageType;
    juce::Image nativeImage{ nativeImageType.create(juce::Image::ARGB, imageBounds.getWidth(), imageBounds.getHeight(), true) };
    {
        juce::Graphics g{ nativeImage };
        sourceComponent.paintEntireComponent(g, false);
    }

    compare(softwareImage, nativeImage);
}
