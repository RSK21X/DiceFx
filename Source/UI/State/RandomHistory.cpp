#include "RandomHistory.h"

#include <cmath>

RandomHistory::RandomHistory (juce::AudioProcessorValueTreeState& apvts)
    : state (apvts)
{
    seedCurrent();
}

DiceParameterSnapshot RandomHistory::captureCurrent() const
{
    DiceParameterSnapshot result;
    const auto& parameters = state.processor.getParameters();

    for (int i = 0; i < parameters.size(); ++i)
    {
        if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*> (parameters.getUnchecked (i)))
        {
            const auto id = ranged->getParameterID();
            if (isHistoryParameter (id))
                result.values.emplace_back (id, ranged->getValue());
        }
    }

    return result;
}

void RandomHistory::restore (const DiceParameterSnapshot& snapshot) const
{
    for (const auto& [id, value] : snapshot.values)
    {
        if (auto* parameter = state.getParameter (id))
        {
            parameter->beginChangeGesture();
            parameter->setValueNotifyingHost (value);
            parameter->endChangeGesture();
        }
    }
}

void RandomHistory::seedCurrent()
{
    snapshots.clear();
    snapshots.push_back (captureCurrent());
    cursor = 0;
}

bool RandomHistory::pushCurrent()
{
    const auto current = captureCurrent();
    if (cursor >= 0 && cursor < (int) snapshots.size() && isSame (snapshots[(size_t) cursor], current))
        return false;

    if (cursor + 1 < (int) snapshots.size())
        snapshots.erase (snapshots.begin() + cursor + 1, snapshots.end());

    snapshots.push_back (current);
    constexpr size_t maxHistory = 32;
    if (snapshots.size() > maxHistory)
        snapshots.erase (snapshots.begin());

    cursor = (int) snapshots.size() - 1;
    return true;
}

bool RandomHistory::previous()
{
    if (! canPrevious())
        return false;

    --cursor;
    restore (snapshots[(size_t) cursor]);
    return true;
}

bool RandomHistory::next()
{
    if (! canNext())
        return false;

    ++cursor;
    restore (snapshots[(size_t) cursor]);
    return true;
}

bool RandomHistory::canPrevious() const
{
    return cursor > 0 && cursor < (int) snapshots.size();
}

bool RandomHistory::canNext() const
{
    return cursor >= 0 && cursor + 1 < (int) snapshots.size();
}

bool RandomHistory::isHistoryParameter (const juce::String& id)
{
    return id != "random_amount" && id != "input_gain" && id != "output_gain";
}

bool RandomHistory::isSame (const DiceParameterSnapshot& a, const DiceParameterSnapshot& b)
{
    if (a.values.size() != b.values.size())
        return false;

    for (size_t i = 0; i < a.values.size(); ++i)
        if (a.values[i].first != b.values[i].first
            || std::abs (a.values[i].second - b.values[i].second) > 0.00001f)
            return false;

    return true;
}
