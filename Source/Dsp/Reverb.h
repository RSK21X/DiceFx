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
        reverb.prepare (spec);
        reverb.reset();
        sampleRate = spec.sampleRate;
        mixSmoothed.reset (sampleRate, 0.05);
        wetBuffer.setSize (static_cast<int> (spec.numChannels), juce::jmax (1, static_cast<int> (spec.maximumBlockSize)));
    }

    void reset()
    {
        reverb.reset();
        mixSmoothed.setCurrentAndTargetValue (mixSmoothed.getTargetValue());
    }

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

        const int numChannels = buffer.getNumChannels();
        const int numSamples = buffer.getNumSamples();
        // Hosts may send smaller, irregular, or oversized buffers. Slice into the
        // storage allocated in prepare(), rather than resizing on the audio thread.
        for (int offset = 0; offset < numSamples;)
        {
            const int count = juce::jmin (wetBuffer.getNumSamples(), numSamples - offset);
            for (int channel = 0; channel < numChannels; ++channel)
                wetBuffer.copyFrom (channel, 0, buffer, channel, offset, count);
            auto block = juce::dsp::AudioBlock<float> (wetBuffer).getSubBlock (0, static_cast<size_t> (count));
            juce::dsp::ProcessContextReplacing<float> context (block);
            reverb.process (context);
            for (int i = 0; i < count; ++i)
            {
                const float mixValue = mixSmoothed.getNextValue();
                for (int channel = 0; channel < numChannels; ++channel)
                {
                    auto* dry = buffer.getWritePointer (channel, offset);
                    const auto* wet = wetBuffer.getReadPointer (channel);
                    dry[i] = dry[i] * (1.0f - mixValue) + wet[i] * mixValue;
                }
            }
            offset += count;
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
