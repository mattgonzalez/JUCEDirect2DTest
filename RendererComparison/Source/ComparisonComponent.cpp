#include "ComparisonComponent.h"

ComparisonComponent::ComparisonComponent(juce::Image& softwareRendererSnapshot_, juce::Image& direct2DRendererSnapshot_) :
    softwareRendererSnapshot(softwareRendererSnapshot_),
    direct2DRendererSnapshot(direct2DRendererSnapshot_)
{
    setSize (600, 400);
}

ComparisonComponent::~ComparisonComponent()
{
}

void ComparisonComponent::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    if (redComparison.isNull() && softwareRendererSnapshot.isValid() && direct2DRendererSnapshot.isValid())
    {
        compare();
    }

	//g.drawImageAt(alphaComparison, 0, 0);

    juce::Image* images[] = { /*&softwareRendererSnapshot, &direct2DRendererSnapshot,*/ &redComparison, & greenComparison, & blueComparison, & alphaComparison};
    juce::Rectangle<int> r{ 0, 0, getWidth() / 2, getHeight() / 2 };
    for (auto const & image : images)
    {
        if (image->isValid())
        {
            g.drawImageWithin(*image, r.getX(), r.getY(), r.getWidth(), r.getHeight(), juce::RectanglePlacement::centred);
        }
        else
        {
            g.setColour(juce::Colours::white);
            g.drawText("Nope", r, juce::Justification::centred);
        }

        r.setX(r.getRight());
        if (r.getX() >= proportionOfWidth(0.75f))
        {
            r.setX(0);
            r.setY(r.getBottom());
        }
    }
}

void ComparisonComponent::resized()
{
}

void ComparisonComponent::compare()
{
    redComparison = juce::Image{ juce::Image::ARGB, softwareRendererSnapshot.getWidth(), softwareRendererSnapshot.getHeight(), true };
    blueComparison = juce::Image{ juce::Image::ARGB, softwareRendererSnapshot.getWidth(), softwareRendererSnapshot.getHeight(), true };
    greenComparison = juce::Image{ juce::Image::ARGB, softwareRendererSnapshot.getWidth(), softwareRendererSnapshot.getHeight(), true };
    alphaComparison = juce::Image{ juce::Image::ARGB, softwareRendererSnapshot.getWidth(), softwareRendererSnapshot.getHeight(), true };

    {
	    juce::Image::BitmapData softwareRendererSnapshotData{ softwareRendererSnapshot, juce::Image::BitmapData::readOnly };
	    juce::Image::BitmapData direct2DRendererSnapshotData{ direct2DRendererSnapshot, juce::Image::BitmapData::readOnly };
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
	                auto deltaInt = (uint8_t)(int)(scale * deltaFloat);
	                juce::Colour c{ deltaInt, zero, zero, 1.0f };
	                redData.setPixelColour(x, y, c);
	            }
	
	            {
	                auto deltaFloat = std::abs(c1.getFloatGreen() - c2.getFloatGreen());
	                auto deltaInt = (uint8_t)(int)(scale * deltaFloat);
	                juce::Colour c{ zero, deltaInt, zero, 1.0f };	            
	                greenData.setPixelColour(x, y, c);
	            }
	            
	            {
	                auto deltaFloat = std::abs(c1.getFloatBlue() - c2.getFloatBlue());
	                auto deltaInt = (uint8_t)(int)(scale * deltaFloat);
	                juce::Colour c{ zero, zero, deltaInt, 1.0f };
	                blueData.setPixelColour(x, y, c);
	            }
	            
	            {
	                auto brightnessDelta = std::abs(c1.getFloatAlpha() - c2.getFloatAlpha());
	                alphaData.setPixelColour(x, y, juce::Colour::greyLevel(brightnessDelta));
	            }
	        }
	    } 
    }
}
