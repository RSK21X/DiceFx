#pragma once

#include <JuceHeader.h>

class LFO
{
public:
    struct Division
    {
        const char* label;
        double beats;
    };

    void prepare (double newSampleRate)
    {
        sampleRate = newSampleRate;
        phase = 0.0;
    }

    void reset() { phase = 0.0; }

    void setSync (bool shouldSync) { sync = shouldSync; }
    void setRateValue (float newValue) { rateValue = juce::jlimit (0.0f, 1.0f, newValue); }
    void setTempo (double newBpm) { bpm = newBpm > 0.0 ? newBpm : 120.0; }

    float getNextValue (int numSamples)
    {
        const double rateHz = getRateHz();
        const double phaseDelta = rateHz / sampleRate;

        const float value = std::sin (juce::MathConstants<double>::twoPi * phase);

        phase += phaseDelta * numSamples;
        phase -= std::floor (phase);

        return value;
    }

    double getRateHz() const
    {
        if (sync)
        {
            const int index = getDivisionIndexFromValue (rateValue, 0.0f, 1.0f);
            const double beatSeconds = 60.0 / bpm;
            const double durationSeconds = beatSeconds * getDivisions()[index].beats;
            return 1.0 / durationSeconds;
        }

        const double minHz = 0.05;
        const double maxHz = 20.0;
        return juce::jmap (static_cast<double> (rateValue), minHz, maxHz);
    }

    static const std::vector<Division>& getDivisions()
    {
        static const std::vector<Division> divisions = {
            { "1/1", 4.0 },
            { "1/2", 2.0 },
            { "1/4", 1.0 },
            { "1/8", 0.5 },
            { "1/16", 0.25 },
            { "1/32", 0.125 },
            { "1/8T", 1.0 / 3.0 },
            { "1/16T", 1.0 / 6.0 },
            { "1/8D", 0.75 },
            { "1/16D", 0.375 }
        };

        return divisions;
    }

    static int getDivisionIndexFromValue (float value, float minValue, float maxValue)
    {
        const auto& divisions = getDivisions();
        const float normalized = juce::jlimit (0.0f, 1.0f, (value - minValue) / (maxValue - minValue));
        const int index = static_cast<int> (std::round (normalized * static_cast<float> (divisions.size() - 1)));
        return juce::jlimit (0, static_cast<int> (divisions.size() - 1), index);
    }

    static double tempoDivisionToMs (int index, double bpm)
    {
        const auto& divisions = getDivisions();
        const int safeIndex = juce::jlimit (0, static_cast<int> (divisions.size() - 1), index);
        const double beatSeconds = 60.0 / (bpm > 0.0 ? bpm : 120.0);
        return beatSeconds * divisions[safeIndex].beats * 1000.0;
    }

private:
    double sampleRate = 44100.0;
    double bpm = 120.0;
    double phase = 0.0;
    float rateValue = 0.5f;
    bool sync = true;
};

