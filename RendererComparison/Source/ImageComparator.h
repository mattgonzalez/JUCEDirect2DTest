#pragma once

#include <d2d1_2.h>
#include <d2d1effects_2.h>
#include <JuceHeader.h>
#include "RenderTestComponent.h"

class ImageComparator : public juce::Component, public juce::ListBoxModel
{
public:
    ImageComparator();
    ~ImageComparator() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void compare(juce::Component& sourceComponent);
    void compare(juce::Image softwareRendererSnapshot_, juce::Image direct2DRendererSnapshot_);
    void compare(juce::File mainDirectory);
    void compareSelectedFilePair();
    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    void selectedRowsChanged(int) override;
    void listBoxItemClicked(int row, const juce::MouseEvent&) override;

private:
    int lastFileIndex = -1;
    juce::File mainDirectory;
    juce::Array<juce::File> folders;
    juce::Array<std::pair<juce::File, juce::File>> filePairs;
    juce::Image sourceImage1, sourceImage2;
    float transformY = 0.0f, transformScale = 1.0f;
    juce::ListBox listBox;
    juce::ComboBox folderCombo;
    juce::ComboBox fileCombo;
    int lastRow = -1;

    void updateFileCombo();
    void transform();

    juce::StatisticsAccumulator<double> redStats;
    juce::StatisticsAccumulator<double> blueStats;
    juce::StatisticsAccumulator<double> greenStats;
    juce::StatisticsAccumulator<double> brightnessStats;

    struct ChannelScore
    {
        double total = 0.0;
        double rms = 0.0;
    };

    struct Score
    {
        ChannelScore channels[5];
    };

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
            imageComparator.listBox.updateContent();
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
        juce::Image difference;
        juce::Image edges;

        juce::Image problemImage;

        juce::CriticalSection lock;

        struct ProblemArea
        {
            juce::Rectangle<int> area;
            Score scores[2];

            float getScore() const;
        };
        std::vector <ProblemArea> problemAreas;

    } compareTask;
       
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImageComparator)
};
