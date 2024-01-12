#pragma once

class StatTable : public juce::TableListBoxModel, public juce::Component, public juce::Timer
{
private:
    Component::SafePointer<Component> owner;
    juce::TableListBox table;
    juce::TextButton resetStatsButton{ "Reset" };

    struct AccumulatorInfo
    {
        String name;
        int index;
        size_t lastCount;
    };

    juce::Array<AccumulatorInfo> accumulatorsInfo
    {
        AccumulatorInfo{
            "Paint duration (ms)",
            direct2d::PaintStats::messageThreadPaintDuration,
            0
        },

        {
            "Paint interval (ms)",
            direct2d::PaintStats::frameInterval,
            0
        },

        {
            "Create geometry (ms)",
            direct2d::PaintStats::createGeometryTime,
            0
        },

        {
            "Create filled GR (ms)",
            direct2d::PaintStats::createFilledGRTime,
            0
        },

        {
            "Create stroked GR (ms)",
            direct2d::PaintStats::createStrokedGRTime,
            0
        }

#if 0
        {
            "Build cmd list (ms)",
            juce::direct2d::PaintStats::buildCommandListDuration,
            0
        },

        {
            "Paint cmd list (ms)",
            juce::direct2d::PaintStats::paintCommandListDuration,
            0
        }
#endif
    };

    enum
    {
        nameColumn = 1,
        countColumn,
        averageColumn,
        standardDeviationColumn,
        maxColumn
    };

public:
    StatTable(Component* component) :
        owner(component)
    {
        setOpaque(true);
        setAlwaysOnTop(true);

        table.setModel(this);
        addAndMakeVisible(table);

        auto& header = table.getHeader();
        header.addColumn({}, nameColumn, 150, 150, 150, juce::TableHeaderComponent::notResizableOrSortable);
        header.addColumn("#", countColumn, 50, 50, 50, juce::TableHeaderComponent::notResizableOrSortable);
        header.addColumn("Avg", averageColumn, 50, 50, 50, juce::TableHeaderComponent::notResizableOrSortable);
        header.addColumn("Std-dev", standardDeviationColumn, 50, 50, 50, juce::TableHeaderComponent::notResizableOrSortable);
        header.addColumn("Max", maxColumn, 50, 50, 50, juce::TableHeaderComponent::notResizableOrSortable);

        table.addAndMakeVisible(resetStatsButton);
        resetStatsButton.onClick = [this]
            {
                if (auto paintStats = getPaintStats())
                {
                    paintStats->reset();
                }
            };

        setSize(350, 150);
        setVisible(true);

        startTimer(200);
    }

    ~StatTable() override
    {
        table.setModel(nullptr);
    }

    void parentHierarchyChanged() override
    {
        if (auto peer = getPeer())
        {
            peer->setCurrentRenderingEngine(0);
        }
    }

    void paint(Graphics&) override {}

    void resized() override
    {
        table.setBounds(getLocalBounds());
        resetStatsButton.setBounds(45, 2, 60, 25);
    }

    int getNumRows() override
    {
        return accumulatorsInfo.size();
    }

    void paintRowBackground(Graphics& g, int /*rowNumber*/, int width, int height, bool /*rowIsSelected*/) override
    {
        g.setColour(juce::Colours::darkgrey);
        g.fillRect(0, 0, width, height);
    }

    void paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool /*rowIsSelected*/) override
    {
        g.setColour(juce::Colours::white);

        if (auto paintStats = getPaintStats())
        {
            auto const& info = accumulatorsInfo[rowNumber];
            auto const& accum = paintStats->getAccumulator(info.index);

            switch (columnId)
            {
            case nameColumn:
                g.drawText(info.name, 0, 0, width - 5, height, juce::Justification::centredRight);
                break;

            case countColumn:
                g.drawText(juce::String{ accum.getCount() }, 5, 0, width, height, juce::Justification::centredLeft);
                break;

            case averageColumn:
                g.drawText(juce::String{ accum.getAverage(), 1 }, 5, 0, width, height, juce::Justification::centredLeft);
                break;

            case standardDeviationColumn:
                g.drawText(juce::String{ accum.getStandardDeviation(), 1 }, 5, 0, width, height, juce::Justification::centredLeft);
                break;

            case maxColumn:
                g.drawText(juce::String{ accum.getMaxValue(), 1 }, 5, 0, width, height, juce::Justification::centredLeft);
                break;
            }
        }
    }

    void timerCallback() override
    {
        bool repaintNeeded = false;

        if (auto paintStats = getPaintStats())
        {
            for (auto& info : accumulatorsInfo)
            {
                auto const& accum = paintStats->getAccumulator(info.index);
                if (accum.getCount() != info.lastCount)
                {
                    repaintNeeded = true;
                    info.lastCount = accum.getCount();
                }
            }
        }

        if (repaintNeeded)
        {
            repaint();
        }
    }

    void update()
    {
        table.updateContent();
    }

    direct2d::PaintStats* const getPaintStats() const
    {
        if (owner)
        {
            auto top = owner->getTopLevelComponent();
            if (auto direct2DObject = top->getProperties()["Direct2D"].getDynamicObject())
            {
                if (auto metricsObject = direct2DObject->getProperty("Metrics").getObject())
                {
                    return dynamic_cast<direct2d::PaintStats*>(metricsObject);
                }
            }
        }

        return nullptr;
    }
};

