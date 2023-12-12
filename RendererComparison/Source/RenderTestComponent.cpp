#include "RenderTestComponent.h"

RenderTestComponent::RenderTestComponent(int64_t seed) :
    random(seed)
{
    editor.setMultiLine(true);
    addAndMakeVisible(editor);

    juce::String text;
    text.preallocateBytes(32768);
    for (int i = 0; i < 10000; ++i)
    {
        if ((i + 1) % 100 == 0)
        {
            text += juce::newLine;
        }
        text += (char)random.nextInt({ 32, 127 });
    }
    editor.setText(text);
    setSize (600, 400);
}

RenderTestComponent::~RenderTestComponent()
{
}

void RenderTestComponent::paint (juce::Graphics& g)
{
}

void RenderTestComponent::resized()
{
    editor.setBounds(getLocalBounds() * 0.5f);
}