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
    juce::Image sourceImage1, sourceImage2, testImage;
    float transformY = 0.0f, transformScale = 1.0f;

    void transform();

    juce::StatisticsAccumulator<double> redStats;
    juce::StatisticsAccumulator<double> blueStats;
    juce::StatisticsAccumulator<double> greenStats;
    juce::StatisticsAccumulator<double> brightnessStats;

    class CompareTask : public juce::ThreadWithProgressWindow
    {
    public:
        CompareTask(ImageComparator& imageComparator_)
            : ThreadWithProgressWindow("Comparing...", true, true),
            imageComparator(imageComparator_)
        {
        }

        void run() override
        {
            compare();
        }

        void threadComplete(bool) override
        {
            imageComparator.repaint();
        }

        void compare();
        void findProblemAreas();
        void scoreProblemAreas();
        void scoreProblemAreas(juce::Image& sourceImage, juce::Image& outputImage);

        ImageComparator& imageComparator;

        juce::Image redComparison;
        juce::Image greenComparison;
        juce::Image blueComparison;
        juce::Image brightnessComparison;

        juce::Image output1, output2;

        juce::Image problemImage;
        juce::Array<juce::Rectangle<int>> problemAreas;
        juce::Array<float> scores;

    } compareTask;
       
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImageComparator)
};
