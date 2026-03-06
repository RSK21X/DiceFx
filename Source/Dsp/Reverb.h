#pragma once

#include <JuceHeader.h>

class DiceReverb
{
public:
    enum Type
    {
        Room = 0,
        Hall = 1,
        Plate = 2
    };

    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        reverb.reset();
        sampleRate = spec.sampleRate;
        mixSmoothed.reset (sampleRate, 0.05);
        wetBuffer.setSize (static_cast<int> (spec.numChannels), static_cast<int> (spec.maximumBlockSize));
    }

    void reset() { reverb.reset(); }

    void setParameters (float newSize, float newDamp, float newMix, int newType, bool shouldEnable)
    {
        enabled = shouldEnable;
        mixSmoothed.setTargetValue (newMix);
        type = (newType >= Plate) ? Plate : (newType == Hall ? Hall : Room);

        juce::dsp::Reverb::Parameters params;
        params.width = 1.0f;
        params.freezeMode = 0.0f;

        switch (type)
        {
            case Hall:
                params.roomSize = juce::jmap (newSize, 0.5f, 1.0f);
                params.damping = juce::jmap (newDamp, 0.1f, 0.6f);
                break;
            case Plate:
                params.roomSize = juce::jmap (newSize, 0.35f, 0.8f);
                params.damping = juce::jmap (newDamp, 0.4f, 0.9f);
                break;
            case Room:
            default:
                params.roomSize = juce::jmap (newSize, 0.2f, 0.6f);
                params.damping = juce::jmap (newDamp, 0.2f, 0.8f);
                break;
        }
        params.wetLevel = 1.0f;
        params.dryLevel = 0.0f;

        reverb.setParameters (params);
    }

    void process (juce::AudioBuffer<float>& buffer)
    {
        if (! enabled)
            return;

        if (wetBuffer.getNumChannels() != buffer.getNumChannels()
            || wetBuffer.getNumSamples() != buffer.getNumSamples())
            wetBuffer.setSize (buffer.getNumChannels(), buffer.getNumSamples(), false, false, true);

        wetBuffer.makeCopyOf (buffer, true);

        juce::dsp::AudioBlock<float> block (wetBuffer);
        juce::dsp::ProcessContextReplacing<float> context (block);
        reverb.process (context);

        const int numChannels = buffer.getNumChannels();
        const int numSamples = buffer.getNumSamples();

        for (int channel = 0; channel < numChannels; ++channel)
        {
            auto* dry = buffer.getWritePointer (channel);
            const auto* wet = wetBuffer.getReadPointer (channel);

            for (int i = 0; i < numSamples; ++i)
            {
                const float mixValue = mixSmoothed.getNextValue();
                dry[i] = dry[i] * (1.0f - mixValue) + wet[i] * mixValue;
            }
        }
    }

private:
    double sampleRate = 44100.0;
    bool enabled = true;
    Type type = Room;
    juce::SmoothedValue<float> mixSmoothed;
    juce::dsp::Reverb reverb;
    juce::AudioBuffer<float> wetBuffer;
};
