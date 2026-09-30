#include "HeaderBar.h"

#include <array>
#include <cmath>

HeaderBar::HeaderBar (DiceFXAudioProcessor& p)
    : processor (p), previousButton ("Previous"), nextButton ("Next")
{
    addAndMakeVisible (presetBox);
    addAndMakeVisible (previousButton);
    addAndMakeVisible (nextButton);
    addAndMakeVisible (abButton);
    addAndMakeVisible (saveButton);
    addAndMakeVisible (menuButton);
    addAndMakeVisible (advancedButton);

    previousButton.setComponentID ("header-prev");
    nextButton.setComponentID ("header-next");
    abButton.setComponentID ("header-ab");
    saveButton.setComponentID ("header-save");
    menuButton.setComponentID ("header-menu");
    advancedButton.setComponentID ("header-advanced");
    advancedButton.setButtonText ("Locks");

    for (auto* button : { &previousButton, &nextButton, &abButton, &saveButton,
                          &menuButton, &advancedButton })
    {
        button->addListener (this);
        button->setWantsKeyboardFocus (true);
    }

    previousButton.setTooltip ("Undo randomization.");
    nextButton.setTooltip ("Redo randomization.");
    advancedButton.setTooltip ("Choose which parameters stay unchanged when randomizing.");
    abButton.setClickingTogglesState (false);
    advancedButton.setClickingTogglesState (false);
    presetBox.addListener (this);
    presetBox.setJustificationType (juce::Justification::centredLeft);
    presetBox.setTextWhenNothingSelected ("Init Clean");
    presetBox.setTooltip ("Choose a DiceFX preset.");

    rebuildPresetMenu();
}

void HeaderBar::paint (juce::Graphics& g)
{
    drawPixelLogo (g, { 0, 5, 84, 24 });
    DiceTheme::drawLabel (g, "MULTI-FX", { 610, 0, 88, getHeight() }, 10);
}

void HeaderBar::resized()
{
    auto area = getLocalBounds().withTrimmedLeft (98).reduced (0, 4);
    presetBox.setBounds (area.removeFromLeft (226));
    area.removeFromLeft (8);
    previousButton.setBounds (area.removeFromLeft (28));
    area.removeFromLeft (4);
    nextButton.setBounds (area.removeFromLeft (28));
    area.removeFromLeft (8);
    abButton.setBounds (area.removeFromLeft (46));
    area.removeFromLeft (6);
    saveButton.setBounds (area.removeFromLeft (64));
    area.removeFromLeft (6);
    menuButton.setBounds (area.removeFromLeft (28));
    advancedButton.setBounds (area.removeFromRight (64));
}

void HeaderBar::rebuildPresetMenu (const juce::String& preferredName)
{
    processor.reloadUserPresets();
    const auto keep = preferredName.isNotEmpty() ? preferredName : presetBox.getText();

    presetBox.clear (juce::dontSendNotification);
    presetItems.clear();
    int itemID = 1;

    for (int i = 0; i < (int) processor.getFactoryPresets().size(); ++i)
    {
        const auto& preset = processor.getFactoryPresets()[(size_t) i];
        presetBox.addItem (preset.name, itemID++);
        presetItems.push_back ({ PresetKind::Factory, i, preset.name });
    }

    if (! processor.getUserPresets().empty())
        presetBox.addSeparator();

    for (int i = 0; i < (int) processor.getUserPresets().size(); ++i)
    {
        const auto& preset = processor.getUserPresets()[(size_t) i];
        presetBox.addItem (preset.name, itemID++);
        presetItems.push_back ({ PresetKind::User, i, preset.name });
    }

    int selectedID = presetItems.empty() ? 0 : 1;
    for (int i = 0; i < (int) presetItems.size(); ++i)
        if (presetItems[(size_t) i].name == keep)
            selectedID = i + 1;

    if (selectedID != 0)
        presetBox.setSelectedId (selectedID, juce::dontSendNotification);
}

void HeaderBar::setHistoryAvailability (bool canPrevious, bool canNext)
{
    previousButton.setEnabled (canPrevious);
    nextButton.setEnabled (canNext);
}

void HeaderBar::setABState (bool showingB)
{
    abButton.setToggleState (showingB, juce::dontSendNotification);
    abButton.setTooltip (showingB ? "Showing B. Click to return to A." : "Compare the current sound with A/B.");
    abButton.repaint();
}

void HeaderBar::setAdvancedState (bool open)
{
    advancedButton.setToggleState (open, juce::dontSendNotification);
    advancedButton.setButtonText (open ? "Close" : "Locks");
    advancedButton.repaint();
}

void HeaderBar::buttonClicked (juce::Button* button)
{
    if (button == &previousButton && onPrevious != nullptr)
        onPrevious();
    else if (button == &nextButton && onNext != nullptr)
        onNext();
    else if (button == &abButton && onAB != nullptr)
        onAB();
    else if (button == &saveButton && onSave != nullptr)
        onSave();
    else if (button == &menuButton)
        showMenu();
    else if (button == &advancedButton && onAdvanced != nullptr)
        onAdvanced();
}

void HeaderBar::comboBoxChanged (juce::ComboBox* changed)
{
    if (changed != &presetBox)
        return;

    const int index = presetBox.getSelectedId() - 1;
    if (index < 0 || index >= (int) presetItems.size())
        return;

    const auto item = presetItems[(size_t) index];
    if (item.kind == PresetKind::Factory)
        processor.applyFactoryPreset (item.index);
    else
        processor.applyUserPreset (item.index);

    if (onPresetChanged != nullptr)
        onPresetChanged();
}

void HeaderBar::showMenu()
{
    juce::PopupMenu menu;
    menu.addItem (1, "Save Preset");
    menu.addSeparator();
    menu.addItem (2, "Import Preset...");
    menu.addItem (3, "Export Current Preset...");

    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&menuButton),
                        [this] (int result)
    {
        if (result == 1 && onSave != nullptr)
            onSave();
        else if (result == 2 && onImport != nullptr)
            onImport();
        else if (result == 3 && onExport != nullptr)
            onExport();
    });
}

void HeaderBar::drawPixelLogo (juce::Graphics& g, juce::Rectangle<int> area) const
{
    static const std::array<std::array<const char*, 7>, 6> glyphs {{
        {{ "11110", "10001", "10001", "10001", "10001", "10001", "11110" }},
        {{ "11111", "00100", "00100", "00100", "00100", "00100", "11111" }},
        {{ "01111", "10000", "10000", "10000", "10000", "10000", "01111" }},
        {{ "11111", "10000", "10000", "11110", "10000", "10000", "11111" }},
        {{ "11111", "10000", "10000", "11110", "10000", "10000", "10000" }},
        {{ "10001", "10001", "01010", "00100", "01010", "10001", "10001" }}
    }};

    constexpr int rows = 7;
    constexpr int cols = 5;
    constexpr int spacing = 1;
    constexpr int totalCols = cols * 6 + spacing * 5;
    const int pixel = juce::jmax (2, juce::jmin (area.getWidth() / totalCols, area.getHeight() / rows));
    const int startX = area.getX() + (area.getWidth() - totalCols * pixel) / 2;
    const int startY = area.getY() + (area.getHeight() - rows * pixel) / 2;

    for (int gi = 0; gi < 6; ++gi)
        for (int row = 0; row < rows; ++row)
            for (int col = 0; col < cols; ++col)
                if (glyphs[(size_t) gi][(size_t) row][col] == '1')
                {
                    const auto x = startX + (gi * (cols + spacing) + col) * pixel;
                    const auto y = startY + row * pixel;
                    g.setColour (row == 0 ? DiceTheme::accent : DiceTheme::textPrimary);
                    g.fillRect (x, y, pixel, pixel);
                }
}
