#pragma once
#include <JuceHeader.h>
#include "../Theme.h"
#include "../../PluginProcessor.h"

class UtilityBar final : public juce::Component
{
public:
    explicit UtilityBar (DiceFXAudioProcessor&);
    void paint (juce::Graphics&) override;
    void resized() override;
private:
    void updateValueLabels();
    juce::Slider inputKnob, mixKnob, outputKnob;
    juce::Label inputTitle, inputValue, mixTitle, mixValue, outputTitle, outputValue;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputKnobAttachment,
        mixKnobAttachment, outputKnobAttachment;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UtilityBar)
};
