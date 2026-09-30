#include "DiceLookAndFeel.h"
#include "Theme.h"
#include "Icons.h"
#include <cmath>

DiceLookAndFeel::DiceLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, DiceTheme::background);
    setColour (juce::Label::textColourId, DiceTheme::textPrimary);
    setColour (juce::ComboBox::textColourId, DiceTheme::textPrimary);
    setColour (juce::PopupMenu::backgroundColourId, DiceTheme::elevated);
    setColour (juce::PopupMenu::textColourId, DiceTheme::textPrimary);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, DiceTheme::accent.withAlpha (0.2f));
    setColour (juce::ScrollBar::thumbColourId, DiceTheme::borderStrong);
    setColour (juce::ScrollBar::trackColourId, DiceTheme::background);
}

juce::Font DiceLookAndFeel::getTextButtonFont (juce::TextButton&, int)
{
    return DiceTheme::mediumFont (11);
}

void DiceLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                            const juce::Colour&, bool highlighted, bool down)
{
    const auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);
    const bool random = button.getComponentID() == "hero-randomize";
    auto fill = random ? DiceTheme::accent : DiceTheme::elevated;
    if (button.getToggleState()) fill = DiceTheme::accent.withAlpha (0.24f);
    if (highlighted) fill = fill.brighter (0.08f);
    if (down) fill = fill.darker (0.14f);
    g.setColour (fill);
    g.fillRoundedRectangle (bounds, 3);
    g.setColour (button.hasKeyboardFocus (true) ? DiceTheme::accentHover : DiceTheme::borderStrong);
    g.drawRoundedRectangle (bounds, 3, 1);

}

void DiceLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& button, bool, bool)
{
    using Icon = DiceIcons::Icon;
    const auto id = button.getComponentID();
    const auto colour = ! button.isEnabled() ? DiceTheme::textDisabled
                        : id == "hero-randomize" ? DiceTheme::background : DiceTheme::textPrimary;
    const auto bounds = button.getLocalBounds().toFloat();
    g.setColour (colour);
    g.setFont (getTextButtonFont (button, button.getHeight()));
    if (id == "drawer-close" || id == "header-menu" || id == "header-prev" || id == "header-next")
    {
        const auto icon = id == "drawer-close" ? Icon::Close : id == "header-menu" ? Icon::Menu
                        : id == "header-prev" ? Icon::Back : Icon::Forward;
        DiceIcons::draw (g, icon, bounds.withSizeKeepingCentre (16, 16), colour);
    }
    else if (id == "hero-randomize" || id == "header-save" || id == "header-advanced")
    {
        const auto icon = id == "hero-randomize" ? Icon::Dice : id == "header-save" ? Icon::Save : Icon::Lock;
        DiceIcons::draw (g, icon, { 7, bounds.getCentreY() - 8, 16, 16 }, colour);
        g.drawText (button.getButtonText(), button.getLocalBounds().withTrimmedLeft (27).withTrimmedRight (5),
                    juce::Justification::centred, false);
    }
    else
        g.drawText (button.getButtonText(), button.getLocalBounds().reduced (4),
                    juce::Justification::centred, false);
}

void DiceLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                        float position, float start, float end, juce::Slider&)
{
    const float diameter = (float) juce::jmin (width, height) - 4;
    const auto centre = juce::Point<float> (x + width * 0.5f, y + height * 0.5f);
    const float radius = diameter * 0.5f, arcRadius = radius - 1;
    const float angle = start + position * (end - start);
    auto polar = [centre] (float r, float a)
    {
        return centre + juce::Point<float> (std::sin (a) * r, -std::cos (a) * r);
    };
    for (int i = 0; i <= 10; ++i)
    {
        const float a = start + (end - start) * i / 10.0f;
        g.setColour (DiceTheme::borderStrong);
        g.drawLine ({ polar (radius - 1, a), polar (radius - 3, a) }, 0.8f);
    }
    juce::Path track, value;
    track.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0, start, end, true);
    value.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0, start, angle, true);
    g.setColour (DiceTheme::background.darker (0.35f));
    g.strokePath (track, juce::PathStrokeType (2.4f));
    g.setColour (DiceTheme::accent);
    g.strokePath (value, juce::PathStrokeType (2.4f));
    const float bodyRadius = radius - 5;
    const auto body = juce::Rectangle<float> (bodyRadius * 2, bodyRadius * 2).withCentre (centre);
    g.setColour (DiceTheme::elevated);
    g.fillEllipse (body);
    g.setColour (DiceTheme::borderStrong);
    g.drawEllipse (body, 1);
    g.setColour (DiceTheme::white);
    g.drawLine ({ polar (bodyRadius * 0.48f, angle), polar (bodyRadius * 0.88f, angle) }, 2);
}

int DiceLookAndFeel::getSliderThumbRadius (juce::Slider&) { return 5; }

void DiceLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                        float position, float minimum, float maximum,
                                        juce::Slider::SliderStyle style, juce::Slider& slider)
{
    if (style != juce::Slider::LinearHorizontal)
    {
        LookAndFeel_V4::drawLinearSlider (g, x, y, width, height, position, minimum, maximum, style, slider);
        return;
    }
    const auto track = juce::Rectangle<float> ((float) x, y + height * 0.5f - 2, (float) width, 4);
    g.setColour (DiceTheme::track);
    g.fillRoundedRectangle (track, 2);
    g.setColour (DiceTheme::accent);
    g.fillRoundedRectangle (track.withWidth (juce::jlimit (0.0f, (float) width, position - x)), 2);
    const auto thumb = juce::Rectangle<float> (8, 14).withCentre ({ position, track.getCentreY() });
    g.setColour (DiceTheme::textPrimary);
    g.fillRoundedRectangle (thumb, 2);
    g.setColour (DiceTheme::background);
    g.drawVerticalLine ((int) position, thumb.getY() + 3, thumb.getBottom() - 3);
}

void DiceLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button, bool hover, bool)
{
    const auto bounds = button.getLocalBounds().toFloat();
    const bool on = button.getToggleState();
    const auto id = button.getComponentID();
    if (id == "drawer-lock")
    {
        const auto box = bounds.withSizeKeepingCentre (14, 14);
        g.setColour (on ? DiceTheme::accent : DiceTheme::background);
        g.fillRoundedRectangle (box, 2);
        g.setColour (DiceTheme::borderStrong);
        g.drawRoundedRectangle (box, 2, 1);
        if (on)
        {
            DiceIcons::draw (g, DiceIcons::Icon::Check, box.reduced (1), DiceTheme::background);
        }
        return;
    }
    if (id == "sync-pill")
    {
        const auto box = bounds.reduced (0.5f);
        g.setColour (on ? DiceTheme::accent.withAlpha (0.15f) : DiceTheme::background);
        g.fillRoundedRectangle (box, 3);
        g.setColour (on || hover || button.hasKeyboardFocus (true) ? DiceTheme::accent : DiceTheme::borderStrong);
        g.drawRoundedRectangle (box, 3, 1);
        g.setColour (on ? DiceTheme::accent : DiceTheme::textSecondary);
        g.setFont (DiceTheme::mediumFont (10));
        g.drawText ("SYNC", box.toNearestInt(), juce::Justification::centred, false);
        return;
    }
    const auto lamp = juce::Rectangle<float> (10, 10).withCentre ({ 8, bounds.getCentreY() });
    g.setColour (on ? DiceTheme::accent : DiceTheme::track);
    g.fillEllipse (lamp);
    g.setColour (on ? DiceTheme::textPrimary : DiceTheme::textSecondary);
    g.setFont (DiceTheme::mediumFont (12));
    g.drawText (button.getButtonText(), button.getLocalBounds().withTrimmedLeft (23),
                juce::Justification::centredLeft, false);
    if (hover || button.hasKeyboardFocus (true))
    {
        g.setColour (DiceTheme::accent.withAlpha (0.7f));
        g.drawRoundedRectangle (bounds.reduced (0.5f), 3, 1);
    }
}

void DiceLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool,
                                    int, int, int, int, juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<float> (0.5f, 0.5f, (float) width - 1, (float) height - 1);
    g.setColour (DiceTheme::background.darker (0.2f));
    g.fillRoundedRectangle (bounds, 3);
    g.setColour (box.hasKeyboardFocus (true) ? DiceTheme::accent : DiceTheme::border);
    g.drawRoundedRectangle (bounds, 3, 1);
    DiceIcons::draw (g, DiceIcons::Icon::Down,
                     { (float) width - 22, height * 0.5f - 7, 14, 14 }, DiceTheme::textSecondary);
}

void DiceLookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    label.setBounds (box.getLocalBounds().reduced (8, 1).withTrimmedRight (18));
    label.setFont (DiceTheme::regularFont (12));
    label.setColour (juce::Label::textColourId, DiceTheme::textPrimary);
}

void DiceLookAndFeel::drawPopupMenuItem (juce::Graphics& g, const juce::Rectangle<int>& area,
                                         bool separator, bool active, bool highlighted,
                                         bool ticked, bool, const juce::String& text,
                                         const juce::String& shortcut, const juce::Drawable*,
                                         const juce::Colour* textColour)
{
    if (separator)
    {
        g.setColour (DiceTheme::border);
        g.fillRect (area.reduced (8, 0).withHeight (1).withCentre (area.getCentre()));
        return;
    }

    g.setColour (highlighted ? DiceTheme::withAlpha (DiceTheme::accent, 0.22f) : DiceTheme::elevated);
    g.fillRect (area);
    auto colour = textColour != nullptr ? *textColour : DiceTheme::textPrimary;
    if (! active)
        colour = DiceTheme::textDisabled;
    g.setColour (colour);
    g.setFont (getPopupMenuFont());
    g.drawText (text, area.reduced (12, 0), juce::Justification::centredLeft, false);
    if (shortcut.isNotEmpty())
        g.drawText (shortcut, area.reduced (12, 0), juce::Justification::centredRight, false);
    if (ticked)
    {
        g.setColour (DiceTheme::accent);
        g.fillEllipse (juce::Rectangle<float> (5.0f, 5.0f).withCentre ({ (float) area.getX() + 6.0f,
                                                                         (float) area.getCentreY() }));
    }
}

void DiceLookAndFeel::drawPopupMenuSectionHeader (juce::Graphics& g,
                                                  const juce::Rectangle<int>& area,
                                                  const juce::String& text)
{
    g.setColour (DiceTheme::textSecondary);
    g.setFont (DiceTheme::mediumFont (10.0f));
    g.drawText (text.toUpperCase(), area.reduced (10, 0), juce::Justification::centredLeft, false);
}

juce::Font DiceLookAndFeel::getPopupMenuFont()
{
    return DiceTheme::regularFont (12.0f);
}

void DiceLookAndFeel::getIdealPopupMenuItemSize (const juce::String& text, bool separator,
                                                 int, int& width, int& height)
{
    if (separator)
    {
        width = 60;
        height = 8;
        return;
    }

    height = 28;
    width = juce::GlyphArrangement::getStringWidthInt (getPopupMenuFont(), text) + 32;
}
