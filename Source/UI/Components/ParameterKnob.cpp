#include "ParameterKnob.h"
#include "../../Mod/LFO.h"

#include <cmath>

namespace
{
juce::Font labelFont (ParameterKnob::Size size)
{
    if (size == ParameterKnob::Size::Main)
        return DiceTheme::mediumFont (11.0f);

    return DiceTheme::regularFont (11.0f);
}

juce::Font valueFont (ParameterKnob::Size size)
{
    if (size == ParameterKnob::Size::Main)
        return DiceTheme::mediumFont (14.0f);

    if (size == ParameterKnob::Size::Small)
        return DiceTheme::regularFont (12.0f);

    return DiceTheme::regularFont (11.0f);
}
}

ParameterKnob::ParameterKnob (juce::AudioProcessorValueTreeState& apvts,
                              const juce::String& paramID,
                              const juce::String& labelText,
                              Size size)
    : state (apvts), parameterID (paramID), knobSize (size)
{
    addAndMakeVisible (label);
    addAndMakeVisible (slider);
    addAndMakeVisible (valueLabel);

    label.setText (labelText, juce::dontSendNotification);
    label.setFont (labelFont (knobSize));
    label.setColour (juce::Label::textColourId, DiceTheme::textSecondary);
    label.setJustificationType (juce::Justification::centred);
    label.setMinimumHorizontalScale (0.65f);

    valueLabel.setFont (valueFont (knobSize));
    valueLabel.setColour (juce::Label::textColourId, DiceTheme::textPrimary);
    valueLabel.setJustificationType (juce::Justification::centred);
    valueLabel.setMinimumHorizontalScale (0.55f);

    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    slider.setComponentID (knobSize == Size::Main ? "knob-main"
                                                  : (knobSize == Size::Small ? "knob-small" : "knob-mini"));
    slider.setTooltip (paramID);
    slider.setScrollWheelEnabled (false);
    slider.onValueChange = [this]
    {
        updateValueLabel();
        repaint();
    };

    if (auto* parameter = dynamic_cast<juce::RangedAudioParameter*> (state.getParameter (parameterID)))
    {
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (state, parameterID, slider);
        slider.setTooltip (parameter->getName (256));
    }

    updateValueLabel();
    if (parameterID == "delay_time" || parameterID == "lfo_rate")
        startTimerHz (15);
}

void ParameterKnob::resized()
{
    auto area = getLocalBounds();
    if (knobSize == Size::Mini)
    {
        slider.setBounds (0, (getHeight() - 34) / 2, 34, 34);
        label.setBounds (38, 12, getWidth() - 38, 18);
        valueLabel.setBounds (38, 31, getWidth() - 38, 18);
        label.setJustificationType (juce::Justification::centredLeft);
        valueLabel.setJustificationType (juce::Justification::centredLeft);
        return;
    }
    const int labelHeight = 16;
    const int valueHeight = 18;

    label.setBounds (area.removeFromTop (labelHeight));
    valueLabel.setBounds (area.removeFromBottom (valueHeight));

    const int desiredDiameter = knobSize == Size::Mini ? 30 : 48;
    const int diameter = juce::jmax (12, juce::jmin (desiredDiameter,
                                                     juce::jmin (area.getWidth() - 4,
                                                                 area.getHeight())));
    slider.setBounds (area.withSizeKeepingCentre (diameter, diameter));
}

void ParameterKnob::paint (juce::Graphics& g)
{
    if (flashAmount <= 0.0f)
        return;

    auto ring = slider.getBounds().toFloat().reduced (4.0f);
    g.setColour (DiceTheme::accent.withAlpha (flashAmount * 0.48f));
    g.drawEllipse (ring, 2.0f);
}

void ParameterKnob::setDimmed (bool shouldDim)
{
    dimmed = shouldDim;
    const auto alpha = dimmed ? 0.75f : 1.0f;
    label.setAlpha (alpha);
    valueLabel.setAlpha (alpha);
    slider.setAlpha (alpha);
    repaint();
}

void ParameterKnob::flash()
{
    flashAmount = 1.0f;
    startTimerHz (30);
    repaint();
}

void ParameterKnob::refreshDisplay()
{
    updateValueLabel();
    repaint();
}

void ParameterKnob::timerCallback()
{
    flashAmount -= 0.09f;
    if (flashAmount <= 0.0f)
    {
        flashAmount = 0.0f;
        if (parameterID != "delay_time" && parameterID != "lfo_rate")
            stopTimer();
    }

    updateValueLabel();
    repaint();
}

void ParameterKnob::updateValueLabel()
{
    valueLabel.setText (formatValue (slider.getValue()), juce::dontSendNotification);
}

juce::String ParameterKnob::formatValue (double value) const
{
    if (parameterID == "input_gain" || parameterID == "output_gain")
    {
        auto text = juce::String (value, 1);
        if (value > 0.0)
            text = "+" + text;
        return text + " dB";
    }

    if (parameterID == "delay_time")
    {
        if (auto* sync = state.getRawParameterValue ("delay_sync"); sync != nullptr && sync->load() > 0.5f)
        {
            const auto index = LFO::getDivisionIndexFromValue ((float) value, 1.0f, 2000.0f);
            return LFO::getDivisions()[(size_t) index].label;
        }

        return juce::String (std::round (value)) + " ms";
    }

    if (parameterID == "lfo_rate")
    {
        if (auto* sync = state.getRawParameterValue ("lfo_sync"); sync != nullptr && sync->load() > 0.5f)
        {
            const auto index = LFO::getDivisionIndexFromValue ((float) value, 0.0f, 1.0f);
            return LFO::getDivisions()[(size_t) index].label;
        }

        return juce::String (juce::jmap (value, 0.0, 1.0, 0.05, 20.0), 2) + " Hz";
    }

    if (parameterID == "mod_rate")
        return juce::String (juce::jmap (value, 0.0, 1.0, 0.05, 6.0), 2) + " Hz";

    if (parameterID == "mix" || parameterID.endsWith ("_mix")
        || parameterID == "random_amount" || parameterID == "delay_feedback"
        || parameterID == "mod_feedback" || parameterID == "lfo_depth"
        || parameterID == "dist_drive" || parameterID == "dist_tone"
        || parameterID == "delay_filter" || parameterID == "rev_size"
        || parameterID == "rev_damp" || parameterID == "mod_depth")
        return juce::String (juce::roundToInt ((float) value * 100.0f)) + "%";

    return juce::String (value, 2);
}
