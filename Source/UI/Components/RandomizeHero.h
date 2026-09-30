#pragma once

#include <JuceHeader.h>
#include "../Theme.h"
#include "../../PluginProcessor.h"

class RandomizeHero final : public juce::Component,
                            private juce::Timer
{
public:
    explicit RandomizeHero (DiceFXAudioProcessor&);

    void paint (juce::Graphics&) override;
    void resized() override;

    void setHistoryAvailability (bool canPrevious, bool canNext);
    void flashRandomize();

    std::function<void()> onRandomize;
    std::function<void()> onPrevious;
    std::function<void()> onNext;

private:
    void timerCallback() override;

    DiceFXAudioProcessor& processor;
    juce::TextButton randomizeButton { "RANDOMIZE" };
    juce::Slider amountSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> amountAttachment;
    float pulse = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RandomizeHero)
};
