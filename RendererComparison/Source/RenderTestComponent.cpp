#include "RenderTestComponent.h"

RenderTestComponent::RenderTestComponent(int64_t seed) :
    random(seed)
{
    text.preallocateBytes(32768);
    for (int i = 0; i < 10000; ++i)
    {
        if ((i + 1) % 100 == 0)
        {
            text += juce::newLine;
        }
        text += (char)random.nextInt({ 32, 127 });
    }
    setSize (600, 400);
}

RenderTestComponent::~RenderTestComponent()
{
}

void RenderTestComponent::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.setGradientFill({ juce::Colours::orange, getLocalBounds().getTopLeft().toFloat(),
        juce::Colours::cyan, getLocalBounds().getBottomRight().toFloat(),
        false });
    g.drawMultiLineText(text, 0, 0, getWidth(), juce::Justification::horizontallyJustified);
}

void RenderTestComponent::resized()
{
}
