#include "ImageComparator.h"

ImageComparator::ImageComparator() :
    compareTask(*this)
{
    setSize(600, 400);
    setWantsKeyboardFocus(true);

    listBox.setModel(this);
    addAndMakeVisible(listBox);
    listBox.setRowHeight(60);

    addAndMakeVisible(folderCombo);
    folderCombo.onChange = [this]() { updateFileCombo(); };
    addAndMakeVisible(fileCombo);
    fileCombo.onChange = [this]() { compareSelectedFilePair(); };
}

ImageComparator::~ImageComparator()
{
    listBox.setModel(nullptr);
}

void ImageComparator::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    if (compareTask.isThreadRunning())
    {
        g.drawImageAt(compareTask.brightnessComparison, listBox.getRight(), 0);

        juce::ScopedLock locker{ compareTask.lock };
        for (auto it = compareTask.problemAreas.rbegin(); it != compareTask.problemAreas.rend(); ++it)
        {
            auto const& problemArea = *it;
            auto c = juce::Colours::red;
            g.setColour(c.withAlpha(0.75f));
            g.drawRect(problemArea.area.translated(listBox.getRight(), 0));
        }

        return;
    }

    juce::Image* images[] = { &sourceImage1,
        &sourceImage2,
        &compareTask.brightnessComparison
        /*&compareTask.edges,*/
        /*,
        &compareTask.redComparison,
        &compareTask.greenComparison,
        &compareTask.blueComparison, */
        /*,&compareTask.brightnessComparison */ };
    juce::Rectangle<int> r{ listBox.getRight(), 0, (getWidth() - listBox.getWidth()) / 3, getHeight() / 1}, lastR;
    for (auto const& image : images)
    {
        if (image->isValid())
        {
            g.setColour(juce::Colours::black);
            g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);

            auto row = listBox.getSelectedRow();
            if (row >= 0)
            {
                auto const& problemArea = compareTask.problemAreas[row];
                auto section = image->getClippedImage(problemArea.area).convertedToFormat(juce::Image::ARGB);

                auto imageArea = r.reduced(50, 50);
                g.drawImageWithin(section, imageArea.getX(), imageArea.getY(), imageArea.getWidth(), imageArea.getHeight(), juce::RectanglePlacement::centred);
            }
            else
            {
                g.drawImageWithin(image->convertedToFormat(juce::Image::ARGB), r.getX(), r.getY(), r.getWidth(), r.getHeight(), juce::RectanglePlacement::centred);

                for (auto it = compareTask.problemAreas.rbegin(); it != compareTask.problemAreas.rend(); ++it)
                {
                    auto const& problemArea = *it;

                    if (problemArea.getScore() > 2.0f)
                    {
                        juce::RectanglePlacement placement;

                        auto transform = placement.getTransformToFit(image->getBounds().toFloat(), r.toFloat());
                        auto area = problemArea.area.toFloat().transformedBy(transform);

                        //area = area.withSizeKeepingCentre(48.0f, 48.0f);
                        //auto c = problemArea.second > 50.0f ? juce::Colours::red : juce::Colours::darkgrey;
                        auto c = juce::Colours::red;
                        g.setColour(c.withAlpha(0.75f));
                        //g.drawRect(area);
                        g.fillEllipse(area);
                        g.setColour(juce::Colours::white);
                        g.setFont(g.getCurrentFont().boldened());
                        g.drawText(juce::String{ problemArea.getScore(), 1 }, area, juce::Justification::centred);
                    }
                }
            }
        }

        lastR = r;

        r.setX(r.getRight());
        if (r.getX() >= proportionOfWidth(0.75f))
        {
            r.setX(0);
            r.setY(r.getBottom());
        }
    }
}

void ImageComparator::resized()
{
    listBox.setBounds(0, 0, 300, getHeight());
    folderCombo.setBounds(listBox.getRight() + 10, 10, 300, 30);
    fileCombo.setBounds(folderCombo.getBounds().translated(0, 32));
}

void ImageComparator::updateFileCombo()
{
    fileCombo.clear();

    filePairs.clear();

    auto folder = mainDirectory.getChildFile(folderCombo.getText());
    auto files = folder.findChildFiles(juce::File::findFiles, false, "*.png");
    for (auto const& file : files)
    {
        auto filename = file.getFileNameWithoutExtension();
        if (filename.endsWith("GDI"))
        {
            auto gdiFile = file;
            filename = filename.replaceSection(filename.length() - 3, 3, "D2D");
            auto d2dFile = file.getSiblingFile(filename).withFileExtension("png");
            filePairs.add({ gdiFile, d2dFile });
        }
    }

    int id = 1;
    for (auto const& filePair : filePairs)
    {
        auto text = filePair.first.getFileNameWithoutExtension().upToFirstOccurrenceOf("GDI", false, false).trim();
        fileCombo.addItem(text, id++);
    }

    fileCombo.setSelectedId(1, juce::sendNotificationSync);
}

void ImageComparator::compareSelectedFilePair()
{
    if (compareTask.isThreadRunning())
    {
        return;
    }

    auto folder = mainDirectory.getChildFile(folderCombo.getText());
    auto filename = fileCombo.getText();
    auto gdiFile = folder.getChildFile(filename + " GDI").withFileExtension("png");
    auto d2dFile = folder.getChildFile(filename + " D2D").withFileExtension("png");

    if (gdiFile.existsAsFile() && d2dFile.existsAsFile())
    {
        sourceImage1 = {};
        sourceImage2 = {};
        juce::ImageCache::releaseUnusedImages();

        sourceImage1 = juce::ImageCache::getFromFile(gdiFile);
        sourceImage2 = juce::ImageCache::getFromFile(d2dFile);

        compareTask.problemAreas.clear();
        listBox.updateContent();

        compareTask.launchThread();
    }
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

juce::Image ImageComparator::applyEdgeDetect(juce::Image const& source)
{
    auto clone = source.createCopy();

    juce::Uuid uuid = (const uint8_t*)&CLSID_D2D1EdgeDetection;
    juce::Array<juce::var> values;

    auto addValue = [&](int index, juce::var value)
        {
            juce::DynamicObject::Ptr object = new juce::DynamicObject;
            object->setProperty("Index", index);
            object->setProperty("Value", value);
            values.add(juce::var{ object.get() });
        };

    addValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 1.0f);
    addValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 1.0f);
    addValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_PREWITT);
    addValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
    addValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

    clone.getProperties()->set("Direct2DEffectCLSID", uuid.toString());
    clone.getProperties()->set("Direct2DEffectValues", values);

    juce::Image output{ clone.getFormat(), clone.getWidth(), clone.getHeight(), false };
    {
        juce::Graphics g{ output };
        g.drawImageAt(clone, 0, 0);
    }
    return output;
}

juce::Image ImageComparator::applyGammaEffect(juce::Image const& source)
{
    auto clone = source.createCopy();

    juce::Uuid uuid = (const uint8_t*)&CLSID_D2D1GammaTransfer;
    juce::Array<juce::var> values;

    auto addValue = [&](int index, juce::var value)
        {
            juce::DynamicObject::Ptr object = new juce::DynamicObject;
            object->setProperty("Index", index);
            object->setProperty("Value", value);
            values.add(juce::var{ object.get() });
        };

     addValue(D2D1_GAMMATRANSFER_PROP_RED_AMPLITUDE, 0.8f);
     addValue(D2D1_GAMMATRANSFER_PROP_GREEN_AMPLITUDE, 0.8f);
     addValue(D2D1_GAMMATRANSFER_PROP_BLUE_AMPLITUDE, 0.8f);

    clone.getProperties()->set("Direct2DEffectCLSID", uuid.toString());
    clone.getProperties()->set("Direct2DEffectValues", values);

    juce::Image output{ clone.getFormat(), clone.getWidth(), clone.getHeight(), false };
    {
        juce::Graphics g{ output };
        g.drawImageAt(clone, 0, 0);
    }
    return output;
}

juce::Image ImageComparator::applyMorphologyEffect(juce::Image const& source)
{
    auto clone = source.createCopy();

    juce::Uuid uuid = (const uint8_t*)&CLSID_D2D1Morphology;
    juce::Array<juce::var> values;

    auto addValue = [&](int index, juce::var value)
        {
            juce::DynamicObject::Ptr object = new juce::DynamicObject;
            object->setProperty("Index", index);
            object->setProperty("Value", value);
            values.add(juce::var{ object.get() });
        };

    addValue(D2D1_MORPHOLOGY_PROP_MODE, D2D1_MORPHOLOGY_MODE_DILATE);
    addValue(D2D1_MORPHOLOGY_PROP_WIDTH, 2);
    addValue(D2D1_MORPHOLOGY_PROP_HEIGHT, 2);

    clone.getProperties()->set("Direct2DEffectCLSID", uuid.toString());
    clone.getProperties()->set("Direct2DEffectValues", values);

    juce::Image output{ clone.getFormat(), clone.getWidth(), clone.getHeight(), false };
    {
        juce::Graphics g{ output };
        g.drawImageAt(clone, 0, 0);
    }
    return output; 
}

juce::Image ImageComparator::applyBrightnessEffect(juce::Image const& source)
{
#if 0
    auto clone = source.createCopy();

    juce::Uuid uuid = (const uint8_t*)&CLSID_D2D1Brightness;
    juce::Array<juce::var> values;

    auto addValue = [&](int index, juce::var value)
        {
            juce::DynamicObject::Ptr object = new juce::DynamicObject;
            object->setProperty("Index", index);
            object->setProperty("Value", value);
            values.add(juce::var{ object.get() });
        };

    addValue(D2D1_BRIGHTNESS_PROP_BLACK_POINT, D2D1::Vector2F(0.0f, 0.2f));

    clone.getProperties()->set("Direct2DEffectCLSID", uuid.toString());
    clone.getProperties()->set("Direct2DEffectValues", values);

    juce::Image output{ clone.getFormat(), clone.getWidth(), clone.getHeight(), false };
    {
        juce::Graphics g{ output };
        g.drawImageAt(clone, 0, 0);
    }
    return output;
#endif
    return {};
}

void ImageComparator::compare(juce::Image softwareRendererSnapshot, juce::Image direct2DRendererSnapshot)
{
    if (softwareRendererSnapshot.isNull() || direct2DRendererSnapshot.isNull())
    {
        return;
    }

    //sourceImage1 = softwareRendererSnapshot.createCopy();
    //sourceImage2 = direct2DRendererSnapshot.createCopy();

    //compare();
    //compareTask.launchThread();
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

    {
        juce::Image::BitmapData softwareRendererSnapshotData{ imageComparator.sourceImage1, juce::Image::BitmapData::readOnly };
        juce::Image::BitmapData direct2DRendererSnapshotData{ imageComparator.sourceImage2, juce::Image::BitmapData::readOnly };
        juce::Image::BitmapData redData{ redComparison, juce::Image::BitmapData::writeOnly };
        juce::Image::BitmapData greenData{ greenComparison, juce::Image::BitmapData::writeOnly };
        juce::Image::BitmapData blueData{ blueComparison, juce::Image::BitmapData::writeOnly };
        juce::Image::BitmapData brightnessData{ brightnessComparison, juce::Image::BitmapData::writeOnly };

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
                    auto b1 = c1.getPerceivedBrightness();
                    auto b2 = c2.getPerceivedBrightness();
                    auto c = juce::jlimit(0.0f, 1.0f, std::abs(b1 - b2));
                    brightnessData.setPixelColour(x, y, juce::Colour::greyLevel(c));
                }
            }
        }
    }

#if 0
    {
        edges = juce::Image{ juce::Image::ARGB, brightnessComparison.getWidth(), brightnessComparison.getHeight(), true };

        juce::Uuid uuid = (const uint8_t*)&CLSID_D2D1EdgeDetection;
        juce::Array<juce::var> values;

        auto addValue = [&](int index, juce::var value)
            {
                juce::DynamicObject::Ptr object = new juce::DynamicObject;
                object->setProperty("Index", index);
                object->setProperty("Value", value);
                values.add(juce::var{ object.get() });
            };

        addValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 1.0f);
        addValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 1.0f);
        addValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
        addValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
        addValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

        brightnessComparison.getProperties()->set("Direct2DEffectCLSID", uuid.toString());
        brightnessComparison.getProperties()->set("Direct2DEffectValues", values);

        {
            juce::Graphics g{ edges };
            g.drawImageAt(brightnessComparison, 0, 0);
        }
    }
#endif

    findProblemAreas();

    {
        juce::Graphics g{ problemImage };
        g.setColour(juce::Colours::cyan);
        for (auto const& pa : problemAreas)
        {
            g.drawRect(pa.area);
        }

    }
    
    scoreProblemAreas(imageComparator.sourceImage1, imageComparator.sourceImage2);
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

void ImageComparator::compare(juce::File mainDirectory_)
{
    mainDirectory = mainDirectory_;

    folders = mainDirectory_.findChildFiles(juce::File::findDirectories, false);
    
    folderCombo.clear();
    int id = 1;
    for (const auto& folder : folders)
    {
        folderCombo.addItem(folder.getFileName(), id++);
    }

    folderCombo.setSelectedId(id - 1);

    updateFileCombo();
}

int ImageComparator::getNumRows()
{
    juce::ScopedLock locker{ compareTask.lock };
    if (compareTask.isThreadRunning())
    {
        return 0;
    }

    return (int)compareTask.problemAreas.size();
}

void ImageComparator::paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    juce::ScopedLock locker{ compareTask.lock };
    if (compareTask.isThreadRunning())
    {
        return;
    }

    if (rowIsSelected)
    {
        g.setColour(juce::Colours::darkgrey);
        g.fillRect(0, 0, width, height);
    }

    auto const& problemArea = compareTask.problemAreas[rowNumber];
    juce::String line = problemArea.area.toString();
    g.setColour(juce::Colours::white);
    juce::Rectangle<int> r{ 0, 0, width, height };
    auto topR = r.removeFromTop(height / 2);
    auto bottomR = r;
    g.drawText(line, topR.withWidth(width / 2), juce::Justification::centredLeft);

    g.setColour(juce::Colours::white);
    g.drawRect(0, 0, width, height);
    if (problemArea.getScore() > 2.0f)
    {
        g.setColour(juce::Colours::red);
        g.setFont(g.getCurrentFont().boldened());
    }
    g.drawText(juce::String{ problemArea.getScore(), 3 }, topR.removeFromRight(width / 2), juce::Justification::centredLeft);
    
    line = juce::String{ problemArea.scores[0].channels[4].rms, 3 };
    line << " " << juce::String{ problemArea.scores[1].channels[4].rms, 3 };
    g.drawText(line, bottomR, juce::Justification::centredLeft);

}

void ImageComparator::listBoxItemClicked(int row, const juce::MouseEvent&)
{
    if (lastRow == row && lastRow >= 0)
    {
        listBox.deselectAllRows();
        lastRow = -1;
    }
    else
    {
        lastRow = row;
    }
}

void ImageComparator::selectedRowsChanged(int)
{
    repaint();
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
    juce::Image::BitmapData problemData{ problemImage, juce::Image::BitmapData::writeOnly };

    problemAreas.clear();

    auto fillProblemArea = [&](juce::Rectangle<int> fillR)
        {
            for (int x = fillR.getX(); x < fillR.getRight(); ++x)
            {
                for (int y = fillR.getY(); y < fillR.getBottom(); ++y)
                {
                    problemData.setPixelColour(x, y, juce::Colours::pink.withAlpha(0.25f));
                }
            }
        };

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
            if (problemData.getPixelColour(sourceX, sourceY).getRed() > 0)
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

            fillProblemArea(r);

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

                juce::ScopedLock locker{ lock };
                bool added = false;
                if (intersection)
                {
                    for (auto& problemArea : problemAreas)
                    {
                        if (problemArea.area.intersects(expandedR))
                        {
                            problemArea.area = problemArea.area.getUnion(r);
                            fillProblemArea(problemArea.area);
                            added = true;
                            break;
                        }
                    }
                }

                if (!added)
                {
                    problemAreas.emplace_back(ProblemArea{ r, {} });
                    fillProblemArea(r);
                }

                juce::MessageManager::callAsync([this] { imageComparator.repaint(); });
            }
        }

    }
}


/*
void ImageComparator::CompareTask::scoreProblemAreas()
{
    problemImage = brightnessComparison.createCopy();
    juce::Graphics g{ problemImage };

    juce::Image::BitmapData brightnessData{ brightnessComparison, juce::Image::BitmapData::readOnly };

    setStatusMessage("Scoring problem areas...");
    double currentProgress = 0.0;

    for (auto& problemArea : problemAreas)
    {
        auto area = problemArea.first;
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

        problemArea = { area, areaRMS };

        g.setColour(juce::Colours::cyan);
        g.drawRect(area);
        g.setColour(juce::Colours::hotpink);
        g.drawText(juce::String{ areaRMS, 1 }, area, juce::Justification::centred);
    }

    scores.sort();
}
*/

void ImageComparator::CompareTask::scoreProblemAreas(juce::Image& sourceImage, juce::Image& sourceImage2)
{
    setStatusMessage("Scoring problem areas...");

    auto calc = [&](juce::Rectangle<int> area, juce::Image::BitmapData& data) -> Score
        {
            Score score;
            float divisorInverse = 1.0f / ((float)sourceImage.getWidth() * (float)sourceImage.getHeight());

            auto calcChannel = [&](int channel, float value)
                {
                    auto& channelScore = score.channels[channel];
                    channelScore.rms += value * value;
                    channelScore.total += value;
                };

            for (int x = area.getX(); x < area.getRight(); ++x)
            {
                for (int y = area.getY(); y < area.getBottom(); ++y)
                {
                    calcChannel(0, data.getPixelColour(x, y).getFloatRed());
                    calcChannel(1, data.getPixelColour(x, y).getFloatBlue());
                    calcChannel(2, data.getPixelColour(x, y).getFloatGreen());
                    calcChannel(3, data.getPixelColour(x, y).getFloatAlpha());
                    calcChannel(4, data.getPixelColour(x, y).getPerceivedBrightness());
                }
            }

            for (int channel = 0; channel < 4; ++channel)
            {
                auto& rms = score.channels[channel].rms;
                rms = std::sqrt(rms * divisorInverse);
            }

            return score;
        };

    {
        juce::Image::BitmapData sourceData{ sourceImage, juce::Image::BitmapData::readOnly };
        juce::Image::BitmapData source2Data{ sourceImage2, juce::Image::BitmapData::readOnly };
        double currentProgress = 0.0;

        for (auto& problemArea : problemAreas)
        {
            auto area = problemArea.area;
            setProgress(currentProgress);
            currentProgress += 1.0 / (double)problemAreas.size();

            if (threadShouldExit())
            {
                return;
            }

            auto score1 = calc(area, sourceData);
            auto score2 = calc(area, source2Data);

            problemArea.area = area;
            problemArea.scores[0] = score1;
            problemArea.scores[1] = score2;
        }
    }

     std::sort(problemAreas.begin(), problemAreas.end(),
         [](const ProblemArea& a, const ProblemArea& b)
         {
             return a.getScore() > b.getScore();
         });

#if 0
    juceGraphics g{ output1 };
        g.drawImageAt(imageComparator.sourceImage1, 0, 0);

        for (auto const& problemArea : problemAreas)
        {
            auto area = problemArea.first;
            auto score = problemArea.second;
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
        g.drawImageAt(imageComparator.sourceImage2, 0, 0);

        for (auto const& problemArea : problemAreas)
        {
            auto area = problemArea.first;
            auto score = problemArea.second;
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
#endif
}

float ImageComparator::CompareTask::ProblemArea::getScore() const
{
    double maxDelta = 0.0;
    double maxValue = 0.0;
    for (int channel = 0; channel < 5; ++channel)
    {
        auto delta = std::abs(scores[0].channels[channel].rms - scores[1].channels[channel].rms);
        if (delta > maxDelta)
        {
            maxDelta = delta;
            maxValue = juce::jmax(maxValue, scores[0].channels[channel].rms);
            maxValue = juce::jmax(maxValue, scores[1].channels[channel].rms);
        }
    }

    if (maxDelta == 0.0)
    {
        return 0.0f;
    }

    return (float)std::log10(maxDelta);
}
