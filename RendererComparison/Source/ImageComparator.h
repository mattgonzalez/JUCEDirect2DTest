#pragma once

#include <JuceHeader.h>
#include "RenderTestComponent.h"

class ImageComparator : public juce::Component
{
public:
    ImageComparator();
    ~ImageComparator() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void compare(juce::Component& sourceComponent);
    void compare(juce::Image softwareRendererSnapshot_, juce::Image direct2DRendererSnapshot_);

private:
    juce::Image original;
    juce::Image sourceImage1, sourceImage2;

    juce::Image redComparison;
    juce::Image greenComparison;
    juce::Image blueComparison;
    juce::Image alphaComparison;

    juce::StatisticsAccumulator<double> redStats;
    juce::StatisticsAccumulator<double> blueStats;
    juce::StatisticsAccumulator<double> greenStats;
    juce::StatisticsAccumulator<double> alphaStats;

    void compare();
       
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImageComparator)
};
