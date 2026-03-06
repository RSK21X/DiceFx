#pragma once

#include <JuceHeader.h>

class Randomizer
{
public:
    Randomizer (juce::AudioProcessorValueTreeState& state, juce::ValueTree& lockStateTree)
        : apvts (state), lockState (lockStateTree)
    {
    }

    void randomize (float amount)
    {
        if (amount <= 0.0f)
            return;

        const auto& params = apvts.processor.getParameters();
        for (int i = 0; i < params.size(); ++i)
        {
            auto* param = params.getUnchecked (i);
            auto* ranged = dynamic_cast<juce::RangedAudioParameter*> (param);
            if (ranged == nullptr)
                continue;

            const auto paramID = ranged->getParameterID();
            if (paramID == "random_amount")
                continue;
            if (paramID == "input_gain" || paramID == "output_gain")
                continue;

            if (isLocked (paramID))
                continue;

            param->beginChangeGesture();

            if (auto* boolParam = dynamic_cast<juce::AudioParameterBool*> (param))
            {
                if (rng.nextFloat() <= amount)
                {
                    const float newValue = rng.nextBool() ? 1.0f : 0.0f;
                    param->setValueNotifyingHost (boolParam->convertTo0to1 (newValue));
                }
            }
            else if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*> (param))
            {
                if (rng.nextFloat() <= amount)
                {
                    const int idx = rng.nextInt (choiceParam->choices.size());
                    param->setValueNotifyingHost (choiceParam->convertTo0to1 (static_cast<float> (idx)));
                }
            }
            else
            {
                const float currentValue = ranged->convertFrom0to1 (param->getValue());
                const float randomValue = ranged->convertFrom0to1 (rng.nextFloat());
                const float newValue = juce::jmap (amount, currentValue, randomValue);
                const float normalized = ranged->convertTo0to1 (newValue);
                param->setValueNotifyingHost (normalized);
            }

            param->endChangeGesture();
        }
    }

    bool isLocked (const juce::String& paramID) const
    {
        if (auto liveLocks = apvts.state.getChildWithName ("LOCKS"); liveLocks.isValid())
            return liveLocks.getProperty (paramID, false);

        return lockState.getProperty (paramID, false);
    }

private:
    juce::AudioProcessorValueTreeState& apvts;
    juce::ValueTree& lockState;
    juce::Random rng;
};
