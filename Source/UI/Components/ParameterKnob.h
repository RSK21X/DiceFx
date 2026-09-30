#pragma once

#include <JuceHeader.h>
#include "../Theme.h"

class ParameterKnob final : public juce::Component,
                            private juce::Timer
{
public:
    enum class Size { Main, Small, Mini };

    ParameterKnob (juce::AudioProcessorValueTreeState& apvts,
                   const juce::String& paramID,
                   const juce::String& labelText,
                   Size size);

    void resized() override;
    void paint (juce::Graphics&) override;

    void setDimmed (bool shouldDim);
    void flash();
    void refreshDisplay();

    juce::Slider& getSlider() { return slider; }
    const juce::String& getParameterID() const { return parameterID; }

private:
    void timerCallback() override;
    void updateValueLabel();
    juce::String formatValue (double value) const;

    juce::AudioProcessorValueTreeState& state;
    juce::String parameterID;
    Size knobSize;

    juce::Label label;
    juce::Slider slider;
    juce::Label valueLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;

    bool dimmed = false;
    float flashAmount = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParameterKnob)
};
