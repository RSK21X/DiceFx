#pragma once

#include <JuceHeader.h>

struct DiceParameterSnapshot
{
    std::vector<std::pair<juce::String, float>> values;
};

class RandomHistory final
{
public:
    explicit RandomHistory (juce::AudioProcessorValueTreeState&);

    DiceParameterSnapshot captureCurrent() const;
    void restore (const DiceParameterSnapshot&) const;

    void seedCurrent();
    bool pushCurrent();
    bool previous();
    bool next();

    bool canPrevious() const;
    bool canNext() const;
    int size() const { return (int) snapshots.size(); }

private:
    static bool isHistoryParameter (const juce::String& parameterID);
    static bool isSame (const DiceParameterSnapshot&, const DiceParameterSnapshot&);

    juce::AudioProcessorValueTreeState& state;
    std::vector<DiceParameterSnapshot> snapshots;
    int cursor = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RandomHistory)
};
