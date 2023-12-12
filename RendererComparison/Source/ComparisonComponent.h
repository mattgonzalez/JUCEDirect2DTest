#pragma once

#include <JuceHeader.h>

class ComparisonComponent : public juce::Component
{
public:
    ComparisonComponent(juce::Image& softwareRendererSnapshot_, juce::Image& direct2DRendererSnapshot_);
    ~ComparisonComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void compare();

private:
    juce::Image& softwareRendererSnapshot;
    juce::Image& direct2DRendererSnapshot;

    juce::Image redComparison;
    juce::Image greenComparison;
    juce::Image blueComparison;
    juce::Image alphaComparison;
       
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComparisonComponent)
};
