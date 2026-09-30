#pragma once

#include <JuceHeader.h>
#include <optional>

#include "PluginProcessor.h"
#include "UI/DiceLookAndFeel.h"
#include "UI/Components/AdvancedDrawer.h"
#include "UI/Components/EffectCard.h"
#include "UI/Components/HeaderBar.h"
#include "UI/Components/LfoRoutingStrip.h"
#include "UI/Components/RandomizeHero.h"
#include "UI/Components/UtilityBar.h"
#include "UI/State/RandomHistory.h"

class DiceFXAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit DiceFXAudioProcessorEditor (DiceFXAudioProcessor&);
    ~DiceFXAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void randomize();
    void restorePrevious();
    void restoreNext();
    void toggleAB();
    void setAdvancedMode (bool shouldShowAdvanced);
    void updateHistoryControls();
    void refreshParameterDisplays();
    void refreshModuleLocks();
    void savePreset();
    void importPreset();
    void exportPreset();

    DiceFXAudioProcessor& processor;
    DiceLookAndFeel lookAndFeel;
    RandomHistory history;
    juce::Component panel;

    HeaderBar header;
    RandomizeHero hero;
    EffectCard distortion;
    EffectCard delay;
    EffectCard reverb;
    EffectCard modulation;
    LfoRoutingStrip lfoStrip;
    UtilityBar utilityBar;
    AdvancedDrawer advancedDrawer;

    std::optional<DiceParameterSnapshot> aState;
    std::optional<DiceParameterSnapshot> bState;
    bool showingB = false;
    bool advancedMode = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DiceFXAudioProcessorEditor)
};
