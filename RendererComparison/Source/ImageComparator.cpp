#include "ImageComparator.h"

ImageComparator::ImageComparator() :
    compareTask(*this)
{
    setSize(600, 400);
    setWantsKeyboardFocus(true);
}

ImageComparator::~ImageComparator()
{
}

void ImageComparator::paint(juce::Graphics& g)
{
    juce::GlowEffect effect;

    g.fillAll(juce::Colours::black);

    if (compareTask.isThreadRunning())
    {
        return;
    }

    juce::Image* images[] = { &compareTask.output1,
        &compareTask.output2 /*,
        &compareTask.redComparison,
        &compareTask.greenComparison,
        &compareTask.blueComparison, */
        /*,&compareTask.brightnessComparison */ };
    juce::StatisticsAccumulator<double>* stats[] = { /* nullptr, nullptr, &redStats, &greenStats, &blueStats, &brightnessStats*/ nullptr , nullptr, nullptr };
    juce::Rectangle<int> r{ 0, 0, getWidth() / 2, getHeight() }, lastR;
    int index = 0;
    for (auto const& image : images)
    {
        if (image->isValid())
        {
            g.drawImageWithin(*image, r.getX(), r.getY(), r.getWidth(), r.getHeight(), juce::RectanglePlacement::centred);
              //g.drawImageAt(*image, r.getX(), r.getY());

            auto testImage = image->createCopy();
            testImage.clear(testImage.getBounds(), juce::Colours::transparentBlack);
            {
                juce::Graphics tg{ testImage };
                tg.getInternalContext().applyEffect(effect, *image, 1.0f, 1.0f);
            }

            //g.drawImageWithin(testImage, r.getX(), r.getY(), r.getWidth(), r.getHeight(), juce::RectanglePlacement::centred);

#if 0
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
#endif
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

    if (compareTask.brightnessComparison.isValid())
    {
        //g.drawImageAt(compareTask.problemImage, 0, 0);
        //g.drawImageWithin(compareTask.output1, 0, 0, getWidth(), getHeight(), juce::RectanglePlacement::centred);
#if 0
        auto problemImage = compareTask.brightnessComparison.createCopy();
        problemImage.clear(problemImage.getBounds(), juce::Colours::transparentBlack);
        auto problemStepIndex = compareTask.problemSteps.size() - 1;

        compareTask.problemStepIndex = juce::jmin(compareTask.problemStepIndex + 1, compareTask.problemSteps.size() - 1);

        {
            juce::Graphics pg{ problemImage };
            int areaIndex = 0;
            for (auto const& area : compareTask.problemSteps[problemStepIndex].areas)
            {
                pg.setColour(juce::Colours::red.withAlpha(0.25f));
                pg.drawRect(area);

                auto score = compareTask.problemSteps[problemStepIndex].scores[areaIndex++];
                pg.setColour(juce::Colours::hotpink);
                pg.drawText(juce::String{ score, 1 }, area, juce::Justification::centred);
            }

            pg.setColour(juce::Colours::white);
            pg.drawText(juce::String{ problemStepIndex }, problemImage.getBounds(), juce::Justification::centred);
        }

        r = lastR;
        //g.drawImageAt(problemImage, 0, 0);
        g.drawImageWithin(problemImage, r.getX(), r.getY(), r.getWidth(), r.getHeight(), juce::RectanglePlacement::centred);
#endif
    }

    //juce::Timer::callAfterDelay(20, [this]() { repaint(); });
    //repaint();
}

void ImageComparator::resized()
{
}

void ImageComparator::transform()
{
    /*
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
    */
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

    //compare();
    compareTask.launchThread();
}

void ImageComparator::CompareTask::compare()
{
    if (imageComparator.sourceImage1.isNull() || imageComparator.sourceImage2.isNull())
    {
        return;
    }

    redComparison = juce::Image{ juce::Image::ARGB, imageComparator.sourceImage1.getWidth(), imageComparator.sourceImage1.getHeight(), true };
    blueComparison = juce::Image{ juce::Image::ARGB, imageComparator.sourceImage1.getWidth(), imageComparator.sourceImage1.getHeight(), true };
    greenComparison = juce::Image{ juce::Image::ARGB, imageComparator.sourceImage1.getWidth(), imageComparator.sourceImage1.getHeight(), true };
    brightnessComparison = juce::Image{ juce::Image::ARGB, imageComparator.sourceImage1.getWidth(), imageComparator.sourceImage1.getHeight(), true };
    output1 = juce::Image{ juce::Image::ARGB, imageComparator.sourceImage1.getWidth(), imageComparator.sourceImage1.getHeight(), true };
    output2 = juce::Image{ juce::Image::ARGB, imageComparator.sourceImage2.getWidth(), imageComparator.sourceImage2.getHeight(), true };

    {
        juce::Image::BitmapData softwareRendererSnapshotData{ imageComparator.sourceImage1, juce::Image::BitmapData::readOnly };
        juce::Image::BitmapData direct2DRendererSnapshotData{ imageComparator.sourceImage2, juce::Image::BitmapData::readOnly };
        juce::Image::BitmapData redData{ redComparison, juce::Image::BitmapData::writeOnly };
        juce::Image::BitmapData greenData{ greenComparison, juce::Image::BitmapData::writeOnly };
        juce::Image::BitmapData blueData{ blueComparison, juce::Image::BitmapData::writeOnly };
        juce::Image::BitmapData alphaData{ brightnessComparison, juce::Image::BitmapData::writeOnly };
        juce::Image::BitmapData out1Data{ output1, juce::Image::BitmapData::writeOnly };
        juce::Image::BitmapData out2Data{ output2, juce::Image::BitmapData::writeOnly };

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
                    imageComparator.redStats.addValue(deltaFloat);
                    auto deltaInt = (uint8_t)(int)(scale * deltaFloat);
                    juce::Colour c{ deltaInt, zero, zero, 1.0f };
                    redData.setPixelColour(x, y, c);
                }

                {
                    auto deltaFloat = std::abs(c1.getFloatGreen() - c2.getFloatGreen());
                    imageComparator.greenStats.addValue(deltaFloat);
                    auto deltaInt = (uint8_t)(int)(scale * deltaFloat);
                    juce::Colour c{ zero, deltaInt, zero, 1.0f };
                    greenData.setPixelColour(x, y, c);
                }

                {
                    auto deltaFloat = std::abs(c1.getFloatBlue() - c2.getFloatBlue());
                    imageComparator.blueStats.addValue(deltaFloat);
                    auto deltaInt = (uint8_t)(int)(scale * deltaFloat);
                    juce::Colour c{ zero, zero, deltaInt, 1.0f };
                    blueData.setPixelColour(x, y, c);
                }

                {
                    auto brightnessDelta = std::abs(c1.getPerceivedBrightness() - c2.getPerceivedBrightness());
                    auto c = std::abs(c1.getPerceivedBrightness() - c2.getPerceivedBrightness());
                    alphaData.setPixelColour(x, y, juce::Colour::greyLevel(c));

                    auto o1 = juce::jmax(0.0f, c - c1.getPerceivedBrightness());
                    out1Data.setPixelColour(x, y, juce::Colour::greyLevel(o1));
                    auto o2 = juce::jmax(0.0f, c - c2.getPerceivedBrightness());
                    out2Data.setPixelColour(x, y, juce::Colour::greyLevel(o2));
                }

            }
        }
    }

    findProblemAreas();
    scoreProblemAreas(imageComparator.sourceImage1, imageComparator.sourceImage2);
    //scoreProblemAreas(imageComparator.sourceImage2, output2);

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

void ImageComparator::CompareTask::findProblemAreas()
{
    setStatusMessage("Finding problem areas...");

    //problemSteps.clear();
    //problemSteps.ensureStorageAllocated(32768);

    juce::Image::BitmapData brightnessData{ brightnessComparison, juce::Image::BitmapData::readOnly };

    constexpr int minWidth = 1;
    constexpr int minHeight = 1;
    juce::Rectangle<int> problemAreaBounds;
    double currentProgress = 0.0;
    double progressStep = 1.0 / (double)brightnessData.width;

    problemImage = juce::Image{ juce::SoftwareImageType{}.create(juce::Image::ARGB, brightnessData.width, brightnessData.height, true) };
    problemImage.clear(problemImage.getBounds(), juce::Colours::black);

    problemAreas.clear();

    for (int sourceX = 0; sourceX < brightnessData.width; ++sourceX)
    {
        if (threadShouldExit())
        {
            return;
        }

        setProgress(currentProgress);
        currentProgress += progressStep;

        for (int sourceY = 0; sourceY < brightnessData.height; ++sourceY)
        {
            auto level = brightnessData.getPixelColour(sourceX, sourceY).getRed(); // red, green, and blue are all the same
            if (level == 0)
            {
                continue;
            }

            juce::Rectangle<int> area;
            int bottom = sourceY + 1;
            for (; bottom < brightnessData.height; ++bottom)
            {
                if (brightnessData.getPixelColour(sourceX, bottom).getRed() == 0)
                {
                    break;
                }
            }

            auto height = bottom - sourceY + 1;
            auto centerY = (height - sourceY) / 2 + sourceY;

            int left = sourceX - 1;
            for (; left >= 0; --left)
            {
                if (brightnessData.getPixelColour(left, centerY).getRed() == 0)
                {
                    break;
                }
            }

            int right = sourceX + 1;
            for (; right < brightnessData.width; ++right)
            {
                if (brightnessData.getPixelColour(right, centerY).getRed() == 0)
                {
                    break;
                }
            }

            auto width = juce::jmax(right - left + 1, minWidth);
            height = juce::jmax(height, minHeight);

            juce::Rectangle<int> r{ sourceX, sourceY, width, height };
            r = r.getIntersection(brightnessData.getBounds());

            {
                for (int x = r.getX(); x < r.getRight(); ++x)
                {
                    for (int y = r.getY(); y < r.getBottom(); ++y)
                    {
                        problemImage.setPixelAt(x, y, juce::Colours::pink.withAlpha(0.25f));
                    }
                }
            }

            auto expandedR = r.expanded(1).getIntersection(problemImage.getBounds());
            bool intersection = false;
            {
                for (int x = expandedR.getX(); x < expandedR.getRight(); ++x)
                {
                    auto topC = brightnessData.getPixelColour(x, expandedR.getY());
                    auto bottomC = brightnessData.getPixelColour(x, expandedR.getBottom() - 1);
                    if (topC.getRed() != 0 || bottomC.getRed() != 0)
                    {
                        intersection = true;
                        break;
                    }
                }

                if (!intersection)
                {
                    for (int y = expandedR.getY(); y < expandedR.getBottom(); ++y)
                    {
                        auto leftC = brightnessData.getPixelColour(expandedR.getX(), y);
                        auto rightC = brightnessData.getPixelColour(expandedR.getRight() - 1, y);
                        if (leftC.getRed() != 0 || rightC.getRed() != 0)
                        {
                            intersection = true;
                            break;
                        }
                    }
                }

                bool added = false;
                if (intersection)
                {
                    for (auto& problemArea : problemAreas)
                    {
                        if (problemArea.intersects(expandedR))
                        {
                            problemArea = problemArea.getUnion(r);
                            added = true;
                            break;
                        }
                    }
                }

                if (!added)
                {
                    problemAreas.add(r);
                }
            }
        }
    }

    {
        //juce::Graphics g{ problemImage };

        for (auto& problemArea : problemAreas)
        {
            //g.setColour(juce::Colours::cyan);
            //g.drawRect(problemArea);
        }
    }
}

void ImageComparator::CompareTask::scoreProblemAreas()
{
    problemImage = brightnessComparison.createCopy();
    juce::Graphics g{ problemImage };

    juce::Image::BitmapData brightnessData{ brightnessComparison, juce::Image::BitmapData::readOnly };

    setStatusMessage("Scoring problem areas...");
    double currentProgress = 0.0;

    scores.clear();

    for (auto area : problemAreas)
    {
        setProgress(currentProgress);
        currentProgress += 1.0 / (double)problemAreas.size();

        if (threadShouldExit())
        {
            return;
        }

        auto areaRMS = 0.0f;
        for (int x = area.getX(); x < area.getRight(); ++x)
        {
            for (int y = area.getY(); y < area.getBottom(); ++y)
            {
                auto level = brightnessData.getPixelColour(x, y).getRed(); // red, green, and blue are all the same
                areaRMS += level * level;
            }
        }

        areaRMS /= area.getWidth() * area.getHeight();
        areaRMS = std::sqrt(areaRMS);
        scores.add(areaRMS);

        g.setColour(juce::Colours::cyan);
        g.drawRect(area);
        g.setColour(juce::Colours::hotpink);
        g.drawText(juce::String{ areaRMS, 1 }, area, juce::Justification::centred);
    }

    scores.sort();
}

void ImageComparator::CompareTask::scoreProblemAreas(juce::Image& sourceImage, juce::Image& sourceImage2)
{
    {
        juce::Graphics g{ output1 };

        g.setColour(juce::Colours::black);
        g.fillAll();

        //g.setOpacity(0.5f);
        //g.drawImageAt(sourceImage, 0, 0);
    }

    juce::Array<float> scores;

    auto calcRMS = [&](juce::Rectangle<int> area, juce::Image::BitmapData& data)
        {
            auto areaRMS = 0.0f;
            for (int x = area.getX(); x < area.getRight(); ++x)
            {
                for (int y = area.getY(); y < area.getBottom(); ++y)
                {
                    auto level = data.getPixelColour(x, y).getRed(); // red, green, and blue are all the same
                    areaRMS += level * level;
                }
            }

            areaRMS /= area.getWidth() * area.getHeight();
            areaRMS = std::sqrt(areaRMS);

            return areaRMS;
        };

    {
        juce::Image::BitmapData sourceData{ sourceImage, juce::Image::BitmapData::readOnly };
        juce::Image::BitmapData source2Data{ sourceImage2, juce::Image::BitmapData::writeOnly };
        double currentProgress = 0.0;

        for (auto area : problemAreas)
        {
            setProgress(currentProgress);
            currentProgress += 1.0 / (double)problemAreas.size();

            if (threadShouldExit())
            {
                return;
            }

            auto rms1 = calcRMS(area, sourceData);
            auto rms2 = calcRMS(area, source2Data);

            scores.add(std::abs(rms1 - rms2));
        }
    }

    {
        juce::Graphics g{ output1 };
        g.setOpacity(0.5f);
        g.drawImageAt(imageComparator.sourceImage1, 0, 0);
        g.setOpacity(1.0f);

        int index = 0;
        for (auto area : problemAreas)
        {
            auto score = scores[index++];
            if (score < 15.0f || area.getWidth() < 5 || area.getHeight() < 5)
            {
                continue;
            }

            g.setColour(juce::Colours::red.withAlpha(0.25f));
            g.fillEllipse(area.toFloat().withSizeKeepingCentre(32.0f, 32.0f));
            g.setColour(juce::Colours::black);
            g.drawText(juce::String{ score, 1 }, area, juce::Justification::centred);
            g.drawEllipse(area.toFloat().withSizeKeepingCentre(32.0f, 32.0f), 2.0f);
        }
    }

    {
        juce::Graphics g{ output2 };
        g.setOpacity(0.5f);
        g.drawImageAt(imageComparator.sourceImage2, 0, 0);
        g.setOpacity(1.0f);

        int index = 0;
        for (auto area : problemAreas)
        {
            auto score = scores[index++];
            if (score < 15.0f || area.getWidth() < 5 || area.getHeight() < 5)
            {
                continue;
            }

            g.setColour(juce::Colours::red.withAlpha(0.25f));
            g.fillEllipse(area.toFloat().withSizeKeepingCentre(32.0f, 32.0f));
            g.setColour(juce::Colours::black);
            g.drawText(juce::String{ score, 1 }, area, juce::Justification::centred);
            g.drawEllipse(area.toFloat().withSizeKeepingCentre(32.0f, 32.0f), 2.0f);
        }
    }
}