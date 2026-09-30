#include "EffectCard.h"

namespace
{
juce::StringArray typeChoices (EffectCard::Kind kind)
{
    switch (kind)
    {
        case EffectCard::Kind::Distortion: return { "Modern", "Vintage", "Hard" };
        case EffectCard::Kind::Delay: return { "Digital", "Tape", "PingPong" };
        case EffectCard::Kind::Reverb: return { "Room", "Hall", "Plate" };
        case EffectCard::Kind::Modulation: return { "Chorus", "Flanger" };
    }

    return {};
}
}

EffectCard::EffectCard (DiceFXAudioProcessor& p, Kind effectKind)
    : processor (p),
      kind (effectKind),
      title (effectKind == Kind::Distortion ? "Distortion"
             : effectKind == Kind::Delay ? "Delay"
             : effectKind == Kind::Reverb ? "Reverb" : "Modulation"),
      enableID (effectKind == Kind::Distortion ? "dist_enable"
                 : effectKind == Kind::Delay ? "delay_enable"
                 : effectKind == Kind::Reverb ? "rev_enable" : "mod_enable"),
      enable (p.getAPVTS(), enableID, title),
      type (p.getAPVTS(), effectKind == Kind::Distortion ? "dist_type"
                              : effectKind == Kind::Delay ? "delay_type"
                              : effectKind == Kind::Reverb ? "rev_type" : "mod_type",
             "Type", typeChoices (effectKind)),
      mainKnob (p.getAPVTS(), effectKind == Kind::Distortion ? "dist_drive"
                                : effectKind == Kind::Delay ? "delay_time"
                                : effectKind == Kind::Reverb ? "rev_size" : "mod_depth",
                effectKind == Kind::Distortion ? "Drive"
                : effectKind == Kind::Delay ? "Time"
                : effectKind == Kind::Reverb ? "Size" : "Depth",
                ParameterKnob::Size::Small)
{
    addAndMakeVisible (enable);
    addAndMakeVisible (type);
    addAndMakeVisible (mainKnob);
    addAndMakeVisible (moduleLock);

    switch (kind)
    {
        case Kind::Distortion:
            lockIDs = { "dist_enable", "dist_type", "dist_drive", "dist_tone", "dist_mix" };
            secondaryKnobs.emplace_back (std::make_unique<ParameterKnob> (p.getAPVTS(), "dist_tone", "Tone", ParameterKnob::Size::Small));
            secondaryKnobs.emplace_back (std::make_unique<ParameterKnob> (p.getAPVTS(), "dist_mix", "Mix", ParameterKnob::Size::Small));
            break;

        case Kind::Delay:
            lockIDs = { "delay_enable", "delay_type", "delay_time", "delay_sync",
                        "delay_feedback", "delay_filter", "delay_mix" };
            sync = std::make_unique<ParameterToggle> (p.getAPVTS(), "delay_sync", "Sync", true);
            addAndMakeVisible (*sync);
            secondaryKnobs.emplace_back (std::make_unique<ParameterKnob> (p.getAPVTS(), "delay_feedback", "Feedback", ParameterKnob::Size::Small));
            secondaryKnobs.emplace_back (std::make_unique<ParameterKnob> (p.getAPVTS(), "delay_filter", "Tone", ParameterKnob::Size::Small));
            secondaryKnobs.emplace_back (std::make_unique<ParameterKnob> (p.getAPVTS(), "delay_mix", "Mix", ParameterKnob::Size::Small));
            break;

        case Kind::Reverb:
            lockIDs = { "rev_enable", "rev_type", "rev_size", "rev_damp", "rev_mix" };
            secondaryKnobs.emplace_back (std::make_unique<ParameterKnob> (p.getAPVTS(), "rev_damp", "Damp", ParameterKnob::Size::Small));
            secondaryKnobs.emplace_back (std::make_unique<ParameterKnob> (p.getAPVTS(), "rev_mix", "Mix", ParameterKnob::Size::Small));
            break;

        case Kind::Modulation:
            lockIDs = { "mod_enable", "mod_type", "mod_rate", "mod_depth", "mod_mix", "mod_feedback" };
            secondaryKnobs.emplace_back (std::make_unique<ParameterKnob> (p.getAPVTS(), "mod_rate", "Rate", ParameterKnob::Size::Small));
            secondaryKnobs.emplace_back (std::make_unique<ParameterKnob> (p.getAPVTS(), "mod_feedback", "Feedback", ParameterKnob::Size::Small));
            secondaryKnobs.emplace_back (std::make_unique<ParameterKnob> (p.getAPVTS(), "mod_mix", "Mix", ParameterKnob::Size::Small));
            break;
    }

    for (auto& knob : secondaryKnobs)
        addAndMakeVisible (*knob);

    moduleLock.onClick = [this] { toggleModuleLock(); };
    enable.getButton().onStateChange = [this]
    {
        updateBypassAppearance();
        repaint();
    };

    if (sync != nullptr)
        sync->getButton().onStateChange = [this]
        {
            mainKnob.refreshDisplay();
            repaint();
        };

    refreshLockState();
    updateBypassAppearance();
}

void EffectCard::paint (juce::Graphics& g)
{
    const bool active = enable.getButton().getToggleState();
    const auto fill = active ? DiceTheme::surface : DiceTheme::surface.darker (0.18f);
    DiceTheme::drawPanel (g, getLocalBounds().toFloat(), fill, DiceTheme::cardRadius);

    g.setColour (DiceTheme::border.withAlpha (0.6f));
    g.drawHorizontalLine (43, 10, (float) getWidth() - 10);
}

void EffectCard::resized()
{
    auto inner = getLocalBounds().reduced (10, 8);
    auto header = inner.removeFromTop (28);
    moduleLock.setBounds (header.removeFromRight (26));
    header.removeFromRight (6);
    type.setBounds (header.removeFromRight (144));
    header.removeFromRight (6);
    if (sync != nullptr)
    {
        sync->setBounds (header.removeFromRight (44));
        header.removeFromRight (6);
    }
    enable.setBounds (header);

    inner.removeFromTop (12);
    const int count = 1 + (int) secondaryKnobs.size();
    const int width = inner.getWidth() / count;
    mainKnob.setBounds (inner.removeFromLeft (width));
    for (auto& knob : secondaryKnobs)
        knob->setBounds (inner.removeFromLeft (width));
}

void EffectCard::refreshLockState()
{
    int locked = 0;
    for (const auto& id : lockIDs)
        locked += processor.isLocked (id) ? 1 : 0;

    moduleLock.setLockState (locked == (int) lockIDs.size(), locked > 0 && locked < (int) lockIDs.size());
}

void EffectCard::refreshDisplays()
{
    mainKnob.refreshDisplay();
    for (auto& knob : secondaryKnobs)
        knob->refreshDisplay();
}

void EffectCard::flashParameters (const juce::StringArray& changedIDs)
{
    auto shouldFlash = [&changedIDs] (const juce::String& id)
    {
        return changedIDs.contains (id);
    };

    if (shouldFlash (mainKnob.getParameterID()))
        mainKnob.flash();
    for (auto& knob : secondaryKnobs)
        if (shouldFlash (knob->getParameterID()))
            knob->flash();
}

void EffectCard::toggleModuleLock()
{
    const bool shouldLock = ! moduleLock.isFullyLocked();
    for (const auto& id : lockIDs)
        processor.setLocked (id, shouldLock);

    refreshLockState();
    if (onLocksChanged != nullptr)
        onLocksChanged();
}

void EffectCard::updateBypassAppearance()
{
    const bool dimmed = ! enable.getButton().getToggleState();
    type.setDimmed (dimmed);
    mainKnob.setDimmed (dimmed);
    for (auto& knob : secondaryKnobs)
        knob->setDimmed (dimmed);
    if (sync != nullptr)
        sync->setDimmed (dimmed);
}
