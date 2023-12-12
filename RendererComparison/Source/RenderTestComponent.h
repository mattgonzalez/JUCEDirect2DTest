#pragma once

#include <JuceHeader.h>

class RenderTestComponent : public juce::Component
{
public:
    RenderTestComponent(int64_t seed);
    ~RenderTestComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    juce::Random random;
    juce::String text;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RenderTestComponent)
};
