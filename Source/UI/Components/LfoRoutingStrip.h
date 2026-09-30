#pragma once

#include <JuceHeader.h>
#include "../Theme.h"
#include "ParameterControls.h"
#include "ParameterKnob.h"
#include "../../PluginProcessor.h"

class LfoRoutingStrip final : public juce::Component
{
public:
    explicit LfoRoutingStrip (DiceFXAudioProcessor&);

    void paint (juce::Graphics&) override;
    void resized() override;

    void refreshDisplays();

private:
    ParameterToggle sync;
    ParameterKnob rate;
    ParameterKnob depth;
    ParameterCombo destination;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LfoRoutingStrip)
};
