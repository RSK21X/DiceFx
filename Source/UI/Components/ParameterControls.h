#pragma once

#include <JuceHeader.h>
#include "../Theme.h"

class ParameterToggle final : public juce::Component
{
public:
    ParameterToggle (juce::AudioProcessorValueTreeState& apvts,
                     const juce::String& paramID,
                     const juce::String& text,
                     bool pillStyle = false);

    void resized() override;
    void setDimmed (bool shouldDim);

    juce::ToggleButton& getButton() { return button; }

private:
    juce::ToggleButton button;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParameterToggle)
};

class ParameterCombo final : public juce::Component
{
public:
    ParameterCombo (juce::AudioProcessorValueTreeState& apvts,
                    const juce::String& paramID,
                    const juce::String& labelText,
                    const juce::StringArray& items,
                    bool inlineLabel = true,
                    bool chipStyle = false);

    void resized() override;
    void setDimmed (bool shouldDim);

    juce::ComboBox& getCombo() { return combo; }

private:
    juce::Label label;
    juce::ComboBox combo;

    bool useInlineLabel = true;
    bool useChipStyle = false;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParameterCombo)
};

class ModuleLockButton final : public juce::Button
{
public:
    ModuleLockButton();

    void setLockState (bool fullyLocked, bool mixed);
    bool isFullyLocked() const { return fullyLocked; }

    void paintButton (juce::Graphics&, bool shouldDrawButtonAsHighlighted,
                      bool shouldDrawButtonAsDown) override;

private:
    bool fullyLocked = false;
    bool mixed = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModuleLockButton)
};
