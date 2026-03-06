#pragma once

#include <JuceHeader.h>
#include <cmath>

class Distortion
{
public:
    enum Type
    {
        Modern = 0,
        Vintage = 1,
        Hard = 2
    };

    void prepare (double newSampleRate)
    {
        sampleRate = newSampleRate;
        driveSmoothed.reset (sampleRate, 0.02);
        mixSmoothed.reset (sampleRate, 0.02);
        updateTone();
        reset();
    }

    void reset()
    {
        driveSmoothed.setCurrentAndTargetValue (driveSmoothed.getTargetValue());
        mixSmoothed.setCurrentAndTargetValue (mixSmoothed.getTargetValue());
        for (auto& filter : toneFilters)
            filter.reset();
    }

    void setParameters (float newDrive, float newMix, float newTone, int newType, bool shouldEnable)
    {
        enabled = shouldEnable;
        type = (newType >= Hard) ? Hard : (newType == Vintage ? Vintage : Modern);
        driveSmoothed.setTargetValue (newDrive);
        mixSmoothed.setTargetValue (newMix);
        tone = newTone;
        updateTone();
    }

    void process (juce::AudioBuffer<float>& buffer)
    {
        if (! enabled)
            return;

        const int numChannels = buffer.getNumChannels();
        const int numSamples = buffer.getNumSamples();

        for (int channel = 0; channel < numChannels; ++channel)
        {
            auto* data = buffer.getWritePointer (channel);
            auto& filter = toneFilters[channel % 2];

            for (int i = 0; i < numSamples; ++i)
            {
                const float driveValue = driveSmoothed.getNextValue();
                const float mixValue = mixSmoothed.getNextValue();
                const float gain = 1.0f + driveValue * 20.0f;

                const float dry = data[i];
                float wet = 0.0f;
                const float x = dry * gain;

                switch (type)
                {
                    case Vintage:
                    {
                        const float offset = 0.18f;
                        wet = std::tanh (x + offset) - std::tanh (offset);
                        wet *= 0.9f;
                        break;
                    }
                    case Hard:
                    {
                        const float drive = 1.0f + driveValue * 28.0f;
                        wet = juce::jlimit (-0.75f, 0.75f, dry * drive);
                        break;
                    }
                    case Modern:
                    default:
                        wet = std::tanh (x);
                        break;
                }
                wet = filter.processSample (wet);

                data[i] = dry * (1.0f - mixValue) + wet * mixValue;
            }
        }
    }

private:
    void updateTone()
    {
        const float minCutoff = 800.0f;
        float maxCutoff = 18000.0f;
        if (type == Vintage)
            maxCutoff = 12000.0f;
        else if (type == Hard)
            maxCutoff = 16000.0f;
        const float cutoff = juce::jmap (tone, 0.0f, 1.0f, minCutoff, maxCutoff);
        auto coeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, cutoff);

        for (auto& filter : toneFilters)
        {
            filter.coefficients = coeffs;
            filter.reset();
        }
    }

    double sampleRate = 44100.0;
    bool enabled = true;
    float tone = 0.7f;
    Type type = Modern;

    juce::SmoothedValue<float> driveSmoothed;
    juce::SmoothedValue<float> mixSmoothed;

    juce::dsp::IIR::Filter<float> toneFilters[2];
};
