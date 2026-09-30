#include "AdvancedDrawer.h"

namespace
{
std::vector<std::pair<juce::String, juce::String>> effectRows (const juce::String& prefix,
                                                                std::initializer_list<std::pair<const char*, const char*>> rows)
{
    std::vector<std::pair<juce::String, juce::String>> result;
    for (const auto& row : rows)
        result.emplace_back (prefix + row.first, row.second);
    return result;
}
}

LockMatrixContent::LockMatrixContent (DiceFXAudioProcessor& p)
    : processor (p)
{
    groups.push_back ({ "DISTORTION", {} });
    for (const auto& [id, label] : effectRows ("dist_", {
        { "enable", "Enable" }, { "type", "Type" }, { "drive", "Drive" },
        { "tone", "Tone" }, { "mix", "Mix" }}))
        groups.back().rows.push_back ({ id, label, std::make_unique<juce::ToggleButton>(),
                                        std::make_unique<juce::Label>() });

    groups.push_back ({ "DELAY", {} });
    for (const auto& [id, label] : effectRows ("delay_", {
        { "enable", "Enable" }, { "type", "Type" }, { "time", "Time" },
        { "sync", "Sync" }, { "feedback", "Feedback" }, { "filter", "Tone" },
        { "mix", "Mix" }}))
        groups.back().rows.push_back ({ id, label, std::make_unique<juce::ToggleButton>(),
                                        std::make_unique<juce::Label>() });

    groups.push_back ({ "REVERB", {} });
    for (const auto& [id, label] : effectRows ("rev_", {
        { "enable", "Enable" }, { "type", "Type" }, { "size", "Size" },
        { "damp", "Damp" }, { "mix", "Mix" }}))
        groups.back().rows.push_back ({ id, label, std::make_unique<juce::ToggleButton>(),
                                        std::make_unique<juce::Label>() });

    groups.push_back ({ "MODULATION", {} });
    for (const auto& [id, label] : effectRows ("mod_", {
        { "enable", "Enable" }, { "type", "Type" }, { "rate", "Rate" },
        { "depth", "Depth" }, { "mix", "Mix" }, { "feedback", "Feedback" }}))
        groups.back().rows.push_back ({ id, label, std::make_unique<juce::ToggleButton>(),
                                        std::make_unique<juce::Label>() });

    groups.push_back ({ "GLOBAL", {} });
    for (const auto& [id, label] : std::vector<std::pair<juce::String, juce::String>> {
        { "input_gain", "Input" }, { "output_gain", "Output" }, { "mix", "Dry / Wet" } })
        groups.back().rows.push_back ({ id, label, std::make_unique<juce::ToggleButton>(),
                                        std::make_unique<juce::Label>() });

    for (auto& group : groups)
        for (auto& row : group.rows)
        {
            row.toggle->setComponentID ("drawer-lock");
            row.toggle->setButtonText ("");
            row.toggle->setClickingTogglesState (true);
            row.toggle->setTooltip ("Lock " + row.label + " when randomizing.");
            row.toggle->getToggleStateValue().referTo (processor.getLockValue (row.parameterID));
            row.toggle->onClick = [this]
            {
                if (onChanged != nullptr)
                    onChanged();
            };

            row.text->setText (row.label, juce::dontSendNotification);
            row.text->setFont (DiceTheme::regularFont (11.0f));
            row.text->setColour (juce::Label::textColourId, DiceTheme::textPrimary);
            row.text->setJustificationType (juce::Justification::centredLeft);
            addAndMakeVisible (*row.toggle);
            addAndMakeVisible (*row.text);
        }
}

void LockMatrixContent::paint (juce::Graphics& g)
{
    int y = 0;
    for (const auto& group : groups)
    {
        g.setColour (DiceTheme::textSecondary);
        g.setFont (DiceTheme::mediumFont (9.0f));
        g.drawText (group.title, 0, y, getWidth(), 18, juce::Justification::centredLeft, false);
        y += 20;

        for (const auto& row : group.rows)
        {
            g.setColour (DiceTheme::withAlpha (DiceTheme::border, 0.75f));
            g.fillRect (0, y + 22, getWidth(), 1);
            y += 23;
        }
        y += 10;
    }
}

void LockMatrixContent::resized()
{
    int y = 20;
    for (auto& group : groups)
    {
        for (auto& row : group.rows)
        {
            row.toggle->setBounds (0, y + 3, 22, 16);
            row.text->setBounds (29, y, getWidth() - 29, 22);
            y += 23;
        }
        y += 30;
    }
}

int LockMatrixContent::getContentHeight() const
{
    int height = 0;
    for (const auto& group : groups)
        height += 20 + (int) group.rows.size() * 23 + 30;
    return height;
}

AdvancedDrawer::AdvancedDrawer (DiceFXAudioProcessor& p)
    : processor (p), content (p)
{
    addAndMakeVisible (closeButton);
    addAndMakeVisible (viewport);
    viewport.setViewedComponent (&content, false);
    viewport.setScrollBarsShown (true, false);
    closeButton.setComponentID ("drawer-close");
    closeButton.setButtonText ("Close");
    closeButton.setTooltip ("Close parameter locks.");
    closeButton.onClick = [this]
    {
        if (onClose != nullptr)
            onClose();
    };
    content.onChanged = [this]
    {
        if (onLocksChanged != nullptr)
            onLocksChanged();
    };
}

void AdvancedDrawer::paint (juce::Graphics& g)
{
    DiceTheme::drawPanel (g, getLocalBounds().toFloat(), DiceTheme::elevated,
                          DiceTheme::outerRadius);
    auto header = getLocalBounds().reduced (16, 12).removeFromTop (26);
    DiceTheme::drawSectionLabel (g, "PARAMETER LOCKS", header);
}

void AdvancedDrawer::resized()
{
    auto area = getLocalBounds().reduced (16, 12);
    auto header = area.removeFromTop (28);
    closeButton.setBounds (header.removeFromRight (26));
    area.removeFromTop (8);
    viewport.setBounds (area);
    content.setSize (juce::jmax (0, area.getWidth() - 10), content.getContentHeight());
}
