#pragma once

#include <JuceHeader.h>
#include "ParameterControls.h"
#include "ParameterKnob.h"
#include "../../PluginProcessor.h"

class EffectCard final : public juce::Component
{
public:
    enum class Kind { Distortion, Delay, Reverb, Modulation };

    EffectCard (DiceFXAudioProcessor&, Kind);

    void paint (juce::Graphics&) override;
    void resized() override;

    void refreshLockState();
    void refreshDisplays();
    void flashParameters (const juce::StringArray& changedIDs);

    Kind getKind() const { return kind; }
    const std::vector<juce::String>& getLockIDs() const { return lockIDs; }
    ModuleLockButton& getModuleLockButton() { return moduleLock; }

    std::function<void()> onLocksChanged;

private:
    void toggleModuleLock();
    void updateBypassAppearance();

    DiceFXAudioProcessor& processor;
    Kind kind;
    juce::String title;
    juce::String enableID;

    ParameterToggle enable;
    ParameterCombo type;
    std::unique_ptr<ParameterToggle> sync;
    ParameterKnob mainKnob;
    std::vector<std::unique_ptr<ParameterKnob>> secondaryKnobs;
    ModuleLockButton moduleLock;
    std::vector<juce::String> lockIDs;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EffectCard)
};
