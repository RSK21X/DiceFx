#include "UtilityBar.h"

UtilityBar::UtilityBar (DiceFXAudioProcessor& processor)
{
    for (auto* knob : { &inputKnob, &mixKnob, &outputKnob })
    {
        addAndMakeVisible (*knob);
        knob->setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        knob->setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        knob->setScrollWheelEnabled (false);
        knob->setComponentID ("utility-knob");
        knob->onValueChange = [this] { updateValueLabels(); };
    }
    inputKnob.setTooltip ("Input gain");
    mixKnob.setTooltip ("Dry / wet mix");
    outputKnob.setTooltip ("Output gain");
    inputTitle.setText ("IN", juce::dontSendNotification);
    mixTitle.setText ("MIX", juce::dontSendNotification);
    outputTitle.setText ("OUT", juce::dontSendNotification);
    for (auto* label : { &inputTitle, &inputValue, &mixTitle, &mixValue, &outputTitle, &outputValue })
    {
        addAndMakeVisible (*label);
        label->setFont (DiceTheme::regularFont (11));
        label->setColour (juce::Label::textColourId, DiceTheme::textPrimary);
        label->setJustificationType (juce::Justification::centredLeft);
    }
    inputKnobAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processor.getAPVTS(), "input_gain", inputKnob);
    mixKnobAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processor.getAPVTS(), "mix", mixKnob);
    outputKnobAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processor.getAPVTS(), "output_gain", outputKnob);
    updateValueLabels();
}
void UtilityBar::updateValueLabels()
{
    inputValue.setText (juce::String (inputKnob.getValue(), 1) + " dB", juce::dontSendNotification);
    mixValue.setText (juce::String (juce::roundToInt ((float) mixKnob.getValue() * 100)) + "%", juce::dontSendNotification);
    outputValue.setText (juce::String (outputKnob.getValue(), 1) + " dB", juce::dontSendNotification);
}
void UtilityBar::paint (juce::Graphics& g)
{
    DiceTheme::drawPanel (g, getLocalBounds().toFloat(), DiceTheme::background, 4);
}
void UtilityBar::resized()
{
    auto layout = [] (int x, juce::Slider& knob, juce::Label& title, juce::Label& value)
    {
        knob.setBounds (x, 5, 34, 34);
        title.setBounds (x + 38, 3, 64, 18);
        value.setBounds (x + 38, 21, 64, 18);
    };
    layout (8, inputKnob, inputTitle, inputValue);
    layout (124, mixKnob, mixTitle, mixValue);
    layout (240, outputKnob, outputTitle, outputValue);
}
