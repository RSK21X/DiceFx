#pragma once

#include <JuceHeader.h>
#include <cmath>

class Modulation
{
public:
    enum Type
    {
        Chorus = 0,
        Flanger = 1
    };

    void prepare (double newSampleRate, int samplesPerBlock)
    {
        sampleRate = newSampleRate;
        juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32> (samplesPerBlock), 1 };
        delayLineL.prepare (spec);
        delayLineR.prepare (spec);

        mixSmoothed.reset (sampleRate, 0.05);
        depthSmoothed.reset (sampleRate, 0.05);
        feedbackSmoothed.reset (sampleRate, 0.05);
        rateSmoothed.reset (sampleRate, 0.05);

        reset();
    }

    void reset()
    {
        delayLineL.reset();
        delayLineR.reset();
        phase = 0.0;
    }

    void setParameters (float newRate,
                        float newDepth,
                        float newMix,
                        float newFeedback,
                        int newType,
                        bool shouldEnable)
    {
        enabled = shouldEnable;
        type = (newType == Flanger) ? Flanger : Chorus;
        rateSmoothed.setTargetValue (juce::jlimit (0.0f, 1.0f, newRate));
        depthSmoothed.setTargetValue (juce::jlimit (0.0f, 1.0f, newDepth));
        mixSmoothed.setTargetValue (juce::jlimit (0.0f, 1.0f, newMix));
        feedbackSmoothed.setTargetValue (juce::jlimit (0.0f, 0.95f, newFeedback));
    }

    void process (juce::AudioBuffer<float>& buffer)
    {
        if (! enabled)
            return;

        const int numChannels = buffer.getNumChannels();
        const int numSamples = buffer.getNumSamples();

        const float baseMs = (type == Flanger) ? 2.0f : 18.0f;
        const float depthMs = (type == Flanger) ? 2.5f : 10.0f;
        const float feedbackScale = (type == Flanger) ? 1.0f : 0.0f;

        auto* left = buffer.getWritePointer (0);
        float* right = (numChannels > 1) ? buffer.getWritePointer (1) : nullptr;

        for (int i = 0; i < numSamples; ++i)
        {
            const float rateValue = rateSmoothed.getNextValue();
            const float depthValue = depthSmoothed.getNextValue();
            const float mixValue = mixSmoothed.getNextValue();
            const float feedbackValue = feedbackSmoothed.getNextValue() * feedbackScale;

            const double rateHz = juce::jmap (static_cast<double> (rateValue), 0.05, 6.0);
            const double phaseDelta = rateHz / sampleRate;
            const float lfo = std::sin (juce::MathConstants<double>::twoPi * phase);

            phase += phaseDelta;
            phase -= std::floor (phase);

            const float delayMs = baseMs + depthValue * depthMs * lfo;
            const float delaySamples = juce::jlimit (1.0f, static_cast<float> (maxDelaySamples - 1),
                                                     delayMs * static_cast<float> (sampleRate) / 1000.0f);

            delayLineL.setDelay (delaySamples);
            delayLineR.setDelay (delaySamples);

            const float dryL = left[i];
            const float delayedL = delayLineL.popSample (0);
            const float wetL = delayedL;
            left[i] = dryL * (1.0f - mixValue) + wetL * mixValue;
            delayLineL.pushSample (0, dryL + delayedL * feedbackValue);

            if (right != nullptr)
            {
                const float dryR = right[i];
                const float delayedR = delayLineR.popSample (0);
                const float wetR = delayedR;
                right[i] = dryR * (1.0f - mixValue) + wetR * mixValue;
                delayLineR.pushSample (0, dryR + delayedR * feedbackValue);
            }
        }
    }

private:
    static constexpr int maxDelaySamples = 48000;

    double sampleRate = 44100.0;
    bool enabled = false;
    Type type = Chorus;
    double phase = 0.0;

    juce::SmoothedValue<float> mixSmoothed;
    juce::SmoothedValue<float> depthSmoothed;
    juce::SmoothedValue<float> feedbackSmoothed;
    juce::SmoothedValue<float> rateSmoothed;

    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLineL { maxDelaySamples };
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLineR { maxDelaySamples };
};
