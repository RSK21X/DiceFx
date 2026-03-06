#pragma once

#include <JuceHeader.h>
#include <cmath>

class Delay
{
public:
    enum Type
    {
        Digital = 0,
        Tape = 1,
        PingPong = 2
    };

    void prepare (double newSampleRate, int samplesPerBlock)
    {
        sampleRate = newSampleRate;
        feedbackSmoothed.reset (sampleRate, 0.05);
        mixSmoothed.reset (sampleRate, 0.05);
        updateFilter();
        juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32> (samplesPerBlock), 1 };
        delayLineL.prepare (spec);
        delayLineR.prepare (spec);
        reset();
    }

    void reset()
    {
        delayLineL.reset();
        delayLineR.reset();
        for (auto& filter : filters)
            filter.reset();
    }

    void setParameters (float newDelayMs,
                        float newFeedback,
                        float newMix,
                        float newFilter,
                        int newType,
                        bool shouldEnable)
    {
        enabled = shouldEnable;
        type = (newType == PingPong) ? PingPong : (newType == Tape ? Tape : Digital);
        delayMs = newDelayMs;
        feedbackSmoothed.setTargetValue (newFeedback);
        mixSmoothed.setTargetValue (newMix);
        filterTone = newFilter;
        updateFilter();

        const float delaySamples = juce::jlimit (1.0f, static_cast<float> (maxDelaySamples), delayMs * static_cast<float> (sampleRate) / 1000.0f);
        delayLineL.setDelay (delaySamples);
        delayLineR.setDelay (delaySamples);
    }

    void process (juce::AudioBuffer<float>& buffer)
    {
        if (! enabled)
            return;

        const int numChannels = buffer.getNumChannels();
        const int numSamples = buffer.getNumSamples();

        if (type == PingPong && numChannels >= 2)
        {
            auto* left = buffer.getWritePointer (0);
            auto* right = buffer.getWritePointer (1);
            auto& filterL = filters[0];
            auto& filterR = filters[1];

            for (int i = 0; i < numSamples; ++i)
            {
                const float feedbackValue = feedbackSmoothed.getNextValue();
                const float mixValue = mixSmoothed.getNextValue();

                float delayedL = delayLineL.popSample (0);
                float delayedR = delayLineR.popSample (0);
                delayedL = filterL.processSample (delayedL);
                delayedR = filterR.processSample (delayedR);

                const float dryL = left[i];
                const float dryR = right[i];

                left[i] = dryL * (1.0f - mixValue) + delayedL * mixValue;
                right[i] = dryR * (1.0f - mixValue) + delayedR * mixValue;

                delayLineL.pushSample (0, dryL + delayedR * feedbackValue);
                delayLineR.pushSample (0, dryR + delayedL * feedbackValue);
            }

            return;
        }

        for (int channel = 0; channel < numChannels; ++channel)
        {
            auto* data = buffer.getWritePointer (channel);
            auto& line = (channel % 2 == 0) ? delayLineL : delayLineR;
            auto& filter = filters[channel % 2];

            for (int i = 0; i < numSamples; ++i)
            {
                const float feedbackValue = feedbackSmoothed.getNextValue();
                const float mixValue = mixSmoothed.getNextValue();

                const float dry = data[i];
                float delayed = line.popSample (0);
                delayed = filter.processSample (delayed);

                if (type == Tape)
                    delayed = applySaturation (delayed);

                const float wet = delayed;
                data[i] = dry * (1.0f - mixValue) + wet * mixValue;

                float toDelay = dry + delayed * feedbackValue;
                if (type == Tape)
                    toDelay = applySaturation (toDelay);
                line.pushSample (0, toDelay);
            }
        }
    }

private:
    float applySaturation (float x) const
    {
        const float drive = 1.4f;
        return std::tanh (x * drive);
    }

    void updateFilter()
    {
        float minCutoff = 800.0f;
        float maxCutoff = 18000.0f;
        if (type == Tape)
        {
            minCutoff = 500.0f;
            maxCutoff = 9000.0f;
        }
        const float cutoff = juce::jmap (filterTone, 0.0f, 1.0f, minCutoff, maxCutoff);
        auto coeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, cutoff);
        for (auto& filter : filters)
        {
            filter.coefficients = coeffs;
            filter.reset();
        }
    }

    static constexpr int maxDelaySamples = 384000;

    double sampleRate = 44100.0;
    bool enabled = true;
    float delayMs = 380.0f;
    float filterTone = 0.6f;
    Type type = Digital;

    juce::SmoothedValue<float> feedbackSmoothed;
    juce::SmoothedValue<float> mixSmoothed;

    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLineL { maxDelaySamples };
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLineR { maxDelaySamples };

    juce::dsp::IIR::Filter<float> filters[2];
};
