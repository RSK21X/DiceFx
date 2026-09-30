#include "LfoRoutingStrip.h"

LfoRoutingStrip::LfoRoutingStrip (DiceFXAudioProcessor& processor)
    : sync (processor.getAPVTS(), "lfo_sync", "Sync", true),
      rate (processor.getAPVTS(), "lfo_rate", "Rate", ParameterKnob::Size::Mini),
      depth (processor.getAPVTS(), "lfo_depth", "Depth", ParameterKnob::Size::Mini),
      destination (processor.getAPVTS(), "lfo_dest", "Target",
                   { "None", "Dist Drive", "Delay Time", "Delay Feedback", "Reverb Size", "Reverb Mix", "Mod Depth" },
                   true, false)
{
    addAndMakeVisible (sync);
    addAndMakeVisible (rate);
    addAndMakeVisible (depth);
    addAndMakeVisible (destination);
    sync.getButton().onStateChange = [this] { rate.refreshDisplay(); };
}

void LfoRoutingStrip::refreshDisplays() { rate.refreshDisplay(); depth.refreshDisplay(); }

void LfoRoutingStrip::paint (juce::Graphics& g)
{
    DiceTheme::drawPanel (g, getLocalBounds().toFloat());
    DiceTheme::drawLabel (g, "LFO", { 14, 10, 44, 18 }, 12, DiceTheme::textPrimary);
    DiceTheme::drawLabel (g, "SINE", { 14, 31, 44, 18 }, 10);
    g.setColour (DiceTheme::border);
    g.drawVerticalLine (315, 12, 52);
    DiceTheme::drawLabel (g, "Modulation", { 330, 5, 160, 16 }, 10);
}

void LfoRoutingStrip::resized()
{
    sync.setBounds (68, 20, 48, 24);
    rate.setBounds (128, 2, 100, 60);
    depth.setBounds (236, 2, 70, 60);
    destination.setBounds (330, 22, getWidth() - 348, 30);
}
