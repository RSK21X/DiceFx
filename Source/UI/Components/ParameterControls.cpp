#include "ParameterControls.h"
#include "../Icons.h"

namespace
{
void drawLockGlyph (juce::Graphics& g, juce::Rectangle<float> area,
                    juce::Colour colour, bool open, bool mixed)
{
    DiceIcons::draw (g, mixed ? DiceIcons::Icon::PartialLock
                             : open ? DiceIcons::Icon::Unlock : DiceIcons::Icon::Lock,
                     area, colour);
}
}

ParameterToggle::ParameterToggle (juce::AudioProcessorValueTreeState& apvts,
                                  const juce::String& paramID,
                                  const juce::String& text,
                                  bool pillStyle)
{
    addAndMakeVisible (button);
    button.setButtonText (text);
    button.setClickingTogglesState (true);
    button.setComponentID (pillStyle ? "sync-pill" : "parameter-toggle");
    button.setTooltip (paramID);
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, paramID, button);
}

void ParameterToggle::resized()
{
    button.setBounds (getLocalBounds());
}

void ParameterToggle::setDimmed (bool shouldDim)
{
    button.setAlpha (shouldDim ? 0.62f : 1.0f);
}

ParameterCombo::ParameterCombo (juce::AudioProcessorValueTreeState& apvts,
                                const juce::String& paramID,
                                const juce::String& labelText,
                                const juce::StringArray& items,
                                bool inlineLabel,
                                bool chipStyle)
    : useInlineLabel (inlineLabel), useChipStyle (chipStyle)
{
    addAndMakeVisible (label);
    addAndMakeVisible (combo);

    label.setText (labelText, juce::dontSendNotification);
    label.setFont (DiceTheme::regularFont (11.0f));
    label.setColour (juce::Label::textColourId, DiceTheme::textSecondary);
    label.setJustificationType (juce::Justification::centredLeft);
    label.setMinimumHorizontalScale (0.55f);

    combo.addItemList (items, 1);
    combo.setComponentID (chipStyle ? "destination-chip" : (inlineLabel ? "combo-inline" : "combo"));
    combo.setJustificationType (juce::Justification::centredLeft);
    combo.setTextWhenNothingSelected ("Select");
    combo.setTextWhenNoChoicesAvailable ("No choices");
    combo.setTooltip (paramID);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, paramID, combo);
}

void ParameterCombo::resized()
{
    auto area = getLocalBounds();
    if (useChipStyle)
    {
        label.setBounds (0, 0, 0, 0);
        combo.setBounds (area.reduced (0, 2));
        return;
    }

    if (useInlineLabel)
    {
        label.setBounds (area.removeFromLeft (36));
        combo.setBounds (area.reduced (0, 2));
    }
    else
    {
        label.setBounds (area.removeFromTop (17));
        combo.setBounds (area.reduced (0, 1));
    }
}

void ParameterCombo::setDimmed (bool shouldDim)
{
    const auto alpha = shouldDim ? 0.75f : 1.0f;
    label.setAlpha (alpha);
    combo.setAlpha (alpha);
}

ModuleLockButton::ModuleLockButton()
    : juce::Button ("Module Lock")
{
    setTooltip ("Lock or unlock every parameter in this effect.");
    setWantsKeyboardFocus (true);
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
}

void ModuleLockButton::setLockState (bool locked, bool isMixed)
{
    fullyLocked = locked;
    mixed = isMixed;
    repaint();
}

void ModuleLockButton::paintButton (juce::Graphics& g,
                                    bool shouldDrawButtonAsHighlighted,
                                    bool shouldDrawButtonAsDown)
{
    auto bounds = getLocalBounds().toFloat().reduced (1.0f);
    auto fill = fullyLocked ? DiceTheme::withAlpha (DiceTheme::accent, 0.14f) : DiceTheme::elevated;
    if (shouldDrawButtonAsDown)
        fill = fill.darker (0.10f);
    else if (shouldDrawButtonAsHighlighted)
        fill = fill.brighter (0.06f);

    g.setColour (fill);
    g.fillRoundedRectangle (bounds, DiceTheme::controlRadius);
    g.setColour (fullyLocked || mixed ? DiceTheme::accent : DiceTheme::borderStrong);
    g.drawRoundedRectangle (bounds, DiceTheme::controlRadius, 1.0f);

    drawLockGlyph (g, bounds.withSizeKeepingCentre (20.0f, 20.0f),
                   fullyLocked || mixed ? DiceTheme::accent : DiceTheme::textSecondary,
                   ! fullyLocked && ! mixed, mixed);
}
