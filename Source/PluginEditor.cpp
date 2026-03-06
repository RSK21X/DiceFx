#include "PluginEditor.h"
#include <array>
#include <cmath>

namespace
{
const juce::Colour kBg        { 0xff2d2d2d };
const juce::Colour kCard      { 0xff3a3a3a };
const juce::Colour kBorder    { 0xff565656 };
const juce::Colour kText      { 0xffebebeb };
const juce::Colour kSubtext   { 0xffbbbbbb };
const juce::Colour kAccent    { 0xffff9f1a };
const juce::Colour kTrack     { 0xff646464 };
const juce::Colour kDisabled  { 0xff727272 };
constexpr int kCollapsedWidth = 980;
constexpr int kExpandedWidth = 1420;
constexpr int kEditorHeight = 560;

juce::Colour withAlpha (juce::Colour c, float a) { return c.withAlpha (juce::jlimit (0.0f, 1.0f, a)); }

void drawCard (juce::Graphics& g, juce::Rectangle<int> bounds)
{
    auto b = bounds.toFloat();
    g.setColour (withAlpha (juce::Colours::black, 0.08f));
    g.fillRoundedRectangle (b.translated (0.0f, 1.0f), 16.0f);

    g.setColour (kCard);
    g.fillRoundedRectangle (b, 16.0f);

    g.setColour (kBorder);
    g.drawRoundedRectangle (b, 16.0f, 2.0f);
}

void drawPixelLogo (juce::Graphics& g, juce::Rectangle<int> area)
{
    static const std::array<std::array<const char*, 7>, 6> glyphs {{
        {{ "11110", "10001", "10001", "10001", "10001", "10001", "11110" }}, // D
        {{ "11111", "00100", "00100", "00100", "00100", "00100", "11111" }}, // I
        {{ "01111", "10000", "10000", "10000", "10000", "10000", "01111" }}, // C
        {{ "11111", "10000", "10000", "11110", "10000", "10000", "11111" }}, // E
        {{ "11111", "10000", "10000", "11110", "10000", "10000", "10000" }}, // F
        {{ "10001", "10001", "01010", "00100", "01010", "10001", "10001" }}  // X
    }};

    constexpr int rows = 7;
    constexpr int colsPerGlyph = 5;
    constexpr int spacing = 1;
    constexpr int totalCols = 6 * colsPerGlyph + 5 * spacing;

    const int pixel = juce::jmax (2, juce::jmin (area.getWidth() / totalCols, area.getHeight() / rows));
    const int logoW = totalCols * pixel;
    const int logoH = rows * pixel;
    const int startX = area.getX() + (area.getWidth() - logoW) / 2;
    const int startY = area.getY() + (area.getHeight() - logoH) / 2;

    g.setColour (withAlpha (juce::Colours::black, 0.28f));
    for (int gi = 0; gi < 6; ++gi)
        for (int r = 0; r < rows; ++r)
            for (int c = 0; c < colsPerGlyph; ++c)
                if (glyphs[(size_t) gi][(size_t) r][c] == '1')
                    g.fillRect (startX + (gi * (colsPerGlyph + spacing) + c) * pixel + 1,
                                startY + r * pixel + 1, pixel, pixel);

    g.setColour (kText);
    for (int gi = 0; gi < 6; ++gi)
        for (int r = 0; r < rows; ++r)
            for (int c = 0; c < colsPerGlyph; ++c)
                if (glyphs[(size_t) gi][(size_t) r][c] == '1')
                    g.fillRect (startX + (gi * (colsPerGlyph + spacing) + c) * pixel,
                                startY + r * pixel, pixel, pixel);

    g.setColour (withAlpha (kAccent, 0.9f));
    for (int gi = 0; gi < 6; ++gi)
        for (int c = 0; c < colsPerGlyph; ++c)
            if (glyphs[(size_t) gi][0][c] == '1')
                g.fillRect (startX + (gi * (colsPerGlyph + spacing) + c) * pixel,
                            startY, pixel, juce::jmax (1, pixel / 2));
}

juce::Font fontRegular (float px) { return juce::Font (juce::FontOptions (px, juce::Font::plain)); }
juce::Font fontSemibold (float px) { return juce::Font (juce::FontOptions (px, juce::Font::bold)); }

juce::String tooltipForParam (const juce::String& paramID)
{
    if (paramID == "dist_enable") return "Enable/disable the distortion module.";
    if (paramID == "dist_type") return "Distortion character: Modern, Vintage, or Hard.";
    if (paramID == "dist_drive") return "Distortion drive amount.";
    if (paramID == "dist_mix") return "Distortion wet/dry mix.";
    if (paramID == "dist_tone") return "Distortion tone (brighter to the right).";

    if (paramID == "delay_enable") return "Enable/disable the delay module.";
    if (paramID == "delay_type") return "Delay character: Digital, Tape, or PingPong.";
    if (paramID == "delay_sync") return "Sync delay time to host tempo divisions.";
    if (paramID == "delay_time") return "Delay time (ms) or tempo division when Sync is on.";
    if (paramID == "delay_feedback") return "Delay feedback amount.";
    if (paramID == "delay_mix") return "Delay wet/dry mix.";
    if (paramID == "delay_filter") return "Delay tone (brighter to the right).";

    if (paramID == "rev_enable") return "Enable/disable the reverb module.";
    if (paramID == "rev_type") return "Reverb space: Room, Hall, or Plate.";
    if (paramID == "rev_size") return "Reverb room size.";
    if (paramID == "rev_damp") return "Reverb damping (darker to the right).";
    if (paramID == "rev_mix") return "Reverb wet/dry mix.";

    if (paramID == "mod_enable") return "Enable/disable the modulation module.";
    if (paramID == "mod_type") return "Modulation type: Chorus or Flanger.";
    if (paramID == "mod_rate") return "Modulation rate (Hz).";
    if (paramID == "mod_depth") return "Modulation depth.";
    if (paramID == "mod_mix") return "Modulation wet/dry mix.";
    if (paramID == "mod_feedback") return "Modulation feedback (flanger intensity).";

    if (paramID == "lfo_sync") return "Sync LFO rate to host tempo divisions.";
    if (paramID == "lfo_rate") return "LFO rate (Hz) or tempo division when Sync is on.";
    if (paramID == "lfo_depth") return "LFO modulation depth.";
    if (paramID == "lfo_dest") return "LFO target: Dist Drive, Delay Time, Delay Feedback, Reverb Size, Reverb Mix, Mod Depth.";

    if (paramID == "input_gain") return "Input gain (pre FX).";
    if (paramID == "output_gain") return "Output gain (post FX).";
    if (paramID == "mix") return "Global wet/dry mix for the whole chain.";
    if (paramID == "random_amount") return "Randomization strength when pressing ROLL.";

    return {};
}

juce::String tooltipForLock() { return "Lock: Dice will not randomize this parameter."; }
}

DiceLookAndFeel::DiceLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, kBg);
    setColour (juce::Label::textColourId, kText);
    setColour (juce::TextButton::buttonColourId, kCard);
    setColour (juce::TextButton::textColourOffId, kText);
    setColour (juce::TextButton::textColourOnId, kText);
    setColour (juce::ComboBox::backgroundColourId, kCard);
    setColour (juce::ComboBox::outlineColourId, kBorder);
    setColour (juce::ComboBox::textColourId, kText);
    setColour (juce::PopupMenu::backgroundColourId, kCard);
    setColour (juce::PopupMenu::textColourId, kText);
    setColour (juce::PopupMenu::highlightedTextColourId, kText);
    setColour (juce::PopupMenu::headerTextColourId, kSubtext);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, withAlpha (kAccent, 0.25f));
}

juce::Font DiceLookAndFeel::getTextButtonFont (juce::TextButton& b, int buttonHeight)
{
    if (b.getComponentID() == "dice")
        return fontSemibold (juce::jlimit (16, 32, buttonHeight / 3));
    if (b.getComponentID() == "advanced")
        return fontSemibold (17.0f);

    return fontSemibold (juce::jlimit (12, 16, buttonHeight / 2));
}

void DiceLookAndFeel::drawButtonBackground (juce::Graphics& g,
                                            juce::Button& button,
                                            const juce::Colour& backgroundColour,
                                            bool shouldDrawButtonAsHighlighted,
                                            bool shouldDrawButtonAsDown)
{
    const auto bounds = button.getLocalBounds().toFloat().reduced (1.0f);
    float corner = 10.0f;
    if (button.getComponentID() == "dice")
        corner = 18.0f;
    else if (button.getComponentID() == "advanced")
        corner = 14.0f;

    auto base = kCard;

    if (! button.isEnabled())
        base = withAlpha (kDisabled, 0.5f);

    if (shouldDrawButtonAsDown)
        base = base.darker (0.08f);
    else if (shouldDrawButtonAsHighlighted)
        base = base.brighter (0.03f);

    // subtle shadow
    g.setColour (withAlpha (juce::Colours::black, 0.20f));
    g.fillRoundedRectangle (bounds.translated (0.0f, 1.0f), corner);

    g.setColour (base);
    g.fillRoundedRectangle (bounds, corner);

    g.setColour (kBorder);
    g.drawRoundedRectangle (bounds, corner, 1.0f);
}

void DiceLookAndFeel::drawButtonText (juce::Graphics& g,
                                      juce::TextButton& button,
                                      bool,
                                      bool)
{
    if (button.getComponentID() == "dice")
    {
        auto area = button.getLocalBounds().toFloat().reduced (14.0f);
        auto icon = area.withSizeKeepingCentre (56.0f, 56.0f);

        g.setColour (withAlpha (juce::Colours::white, 0.08f));
        g.fillRoundedRectangle (icon, 12.0f);
        g.setColour (withAlpha (juce::Colours::white, 0.18f));
        g.drawRoundedRectangle (icon, 12.0f, 1.0f);

        auto pip = [] (juce::Graphics& gg, juce::Point<float> c, float r)
        {
            gg.fillEllipse (juce::Rectangle<float> (r * 2.0f, r * 2.0f).withCentre (c));
        };

        g.setColour (withAlpha (kText, 0.85f));
        const float r = 4.2f;
        const auto left = icon.getX() + icon.getWidth() * 0.30f;
        const auto right = icon.getX() + icon.getWidth() * 0.70f;
        const auto top = icon.getY() + icon.getHeight() * 0.30f;
        const auto bottom = icon.getY() + icon.getHeight() * 0.70f;
        const auto midX = icon.getCentreX();
        const auto midY = icon.getCentreY();

        pip (g, { left, top }, r);
        pip (g, { right, bottom }, r);
        pip (g, { midX, midY }, r);

        return;
    }

    if (button.getComponentID() == "advanced")
    {
        juce::Graphics::ScopedSaveState scoped (g);
        auto area = button.getLocalBounds().toFloat();
        auto transform = juce::AffineTransform::rotation (-juce::MathConstants<float>::halfPi, area.getCentreX(), area.getCentreY());
        g.addTransform (transform);
        g.setColour (kText);
        g.setFont (getTextButtonFont (button, button.getHeight()));
        g.drawFittedText (button.getButtonText(),
                          area.toNearestInt().reduced (6),
                          juce::Justification::centred,
                          1);
        return;
    }

    auto colour = findColour (juce::TextButton::textColourOffId);
    g.setColour (colour.withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.6f));
    g.setFont (getTextButtonFont (button, button.getHeight()));
    g.drawFittedText (button.getButtonText(), button.getLocalBounds().reduced (10), juce::Justification::centred, 1);
}

void DiceLookAndFeel::drawRotarySlider (juce::Graphics& g,
                                        int x, int y, int width, int height,
                                        float sliderPosProportional,
                                        float rotaryStartAngle,
                                        float rotaryEndAngle,
                                        juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height).reduced (10.0f);
    const auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const auto lineW = juce::jmax (2.0f, radius * 0.12f);
    const auto arcRadius = radius - lineW * 1.2f;

    const auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    juce::Path backgroundArc;
    backgroundArc.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);

    juce::Path valueArc;
    valueArc.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f, rotaryStartAngle, angle, true);

    const auto isEnabled = slider.isEnabled();
    const auto track = isEnabled ? kTrack : withAlpha (kDisabled, 0.7f);
    const auto accent = isEnabled ? kAccent : withAlpha (kDisabled, 0.9f);

    g.setColour (track);
    g.strokePath (backgroundArc, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    g.setColour (accent);
    g.strokePath (valueArc, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // knob dot
    const auto dotRadius = lineW * 0.65f;
    juce::Point<float> dot (centre.x + arcRadius * std::cos (angle),
                            centre.y + arcRadius * std::sin (angle));
    g.setColour (accent);
    g.fillEllipse (juce::Rectangle<float> (dotRadius * 2.0f, dotRadius * 2.0f).withCentre (dot));
}

int DiceLookAndFeel::getSliderThumbRadius (juce::Slider& s)
{
    if (s.getSliderStyle() == juce::Slider::LinearHorizontal)
        return 10;
    return juce::LookAndFeel_V4::getSliderThumbRadius (s);
}

void DiceLookAndFeel::drawLinearSlider (juce::Graphics& g,
                                        int x, int y, int width, int height,
                                        float sliderPos,
                                        float minSliderPos,
                                        float maxSliderPos,
                                        const juce::Slider::SliderStyle style,
                                        juce::Slider& slider)
{
    if (style != juce::Slider::LinearHorizontal)
    {
        juce::LookAndFeel_V4::drawLinearSlider (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        return;
    }

    auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height).reduced (4.0f, 8.0f);
    const auto trackH = 6.0f;
    auto track = bounds.withHeight (trackH).withCentre ({ bounds.getCentreX(), bounds.getCentreY() });
    const auto enabled = slider.isEnabled();

    const auto trackColour = enabled ? kTrack : withAlpha (kDisabled, 0.7f);
    const auto fillColour = enabled ? kAccent : withAlpha (kDisabled, 0.9f);

    g.setColour (trackColour);
    g.fillRoundedRectangle (track, trackH * 0.5f);

    auto filled = track;
    filled.setWidth (juce::jlimit (0.0f, track.getWidth(), sliderPos - track.getX()));
    g.setColour (withAlpha (fillColour, 0.95f));
    g.fillRoundedRectangle (filled, trackH * 0.5f);

    const auto r = (float) getSliderThumbRadius (slider);
    juce::Rectangle<float> thumb (r * 2.0f, r * 2.0f);
    thumb.setCentre ({ sliderPos, track.getCentreY() });

    g.setColour (withAlpha (juce::Colours::white, 0.9f));
    g.fillEllipse (thumb);
    g.setColour (withAlpha (juce::Colours::black, 0.25f));
    g.drawEllipse (thumb, 1.0f);
}

void DiceLookAndFeel::drawToggleButton (juce::Graphics& g,
                                        juce::ToggleButton& button,
                                        bool shouldDrawButtonAsHighlighted,
                                        bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat();
    const auto enabled = button.isEnabled();
    const auto toggled = button.getToggleState();

    // small square lock buttons keep the default vibe but readable
    if (button.getComponentID() == "lock")
    {
        auto box = bounds.reduced (1.0f).withSizeKeepingCentre (18.0f, 18.0f);
        g.setColour (kCard);
        g.fillRoundedRectangle (box, 5.0f);
        g.setColour (toggled ? kAccent : kBorder);
        g.drawRoundedRectangle (box, 5.0f, 1.0f);

        // Simple lock glyph
        const auto stroke = 1.4f;
        const auto icon = box.reduced (5.0f);
        juce::Path p;
        auto body = icon.reduced (0.0f, icon.getHeight() * 0.25f);
        body.setY (icon.getY() + icon.getHeight() * 0.45f);
        p.addRoundedRectangle (body, 2.0f);

        juce::Path shackle;
        auto sh = icon;
        sh.setHeight (icon.getHeight() * 0.55f);
        sh.setY (icon.getY());
        sh.reduce (icon.getWidth() * 0.18f, icon.getHeight() * 0.10f);
        shackle.addArc (sh.getX(), sh.getY(), sh.getWidth(), sh.getHeight(), juce::MathConstants<float>::pi, juce::MathConstants<float>::twoPi, true);

        g.setColour (toggled ? kAccent : kSubtext);
        g.strokePath (shackle, juce::PathStrokeType (stroke, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.strokePath (p, juce::PathStrokeType (stroke, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        return;
    }

    const auto switchH = 18.0f;
    const auto switchW = 34.0f;
    auto sw = bounds.removeFromLeft (switchW + 8.0f);
    auto track = sw.withSizeKeepingCentre (switchW, switchH);

    auto trackColour = toggled ? kAccent : withAlpha (kBorder, 0.9f);
    if (! enabled)
        trackColour = withAlpha (kDisabled, 0.8f);
    if (shouldDrawButtonAsDown)
        trackColour = trackColour.darker (0.05f);
    else if (shouldDrawButtonAsHighlighted)
        trackColour = trackColour.brighter (0.02f);

    g.setColour (trackColour);
    g.fillRoundedRectangle (track, switchH * 0.5f);

    auto knob = juce::Rectangle<float> (switchH - 4.0f, switchH - 4.0f);
    knob.setCentre (juce::Point<float> (toggled ? (track.getRight() - switchH * 0.5f) : (track.getX() + switchH * 0.5f),
                                        track.getCentreY()));

    g.setColour (withAlpha (juce::Colours::white, 0.9f));
    g.fillEllipse (knob);
    g.setColour (withAlpha (juce::Colours::black, 0.25f));
    g.drawEllipse (knob, 1.0f);

    auto textArea = bounds.toNearestInt();
    g.setColour (enabled ? kText : kSubtext);
    g.setFont (fontSemibold (12.0f));
    g.drawFittedText (button.getButtonText(), textArea, juce::Justification::centredLeft, 1);
}

void DiceLookAndFeel::drawComboBox (juce::Graphics& g,
                                    int width, int height,
                                    bool,
                                    int, int, int, int,
                                    juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<float> (0, 0, (float) width, (float) height).reduced (1.0f);
    g.setColour (kCard);
    g.fillRoundedRectangle (bounds, 6.0f);
    g.setColour (kBorder);
    g.drawRoundedRectangle (bounds, 6.0f, 1.0f);

    // chevron
    auto arrowArea = bounds.removeFromRight (30.0f).reduced (10.0f, 8.0f);
    juce::Path p;
    p.startNewSubPath (arrowArea.getX(), arrowArea.getCentreY() - 2.0f);
    p.lineTo (arrowArea.getCentreX(), arrowArea.getCentreY() + 3.0f);
    p.lineTo (arrowArea.getRight(), arrowArea.getCentreY() - 2.0f);
    g.setColour (kSubtext);
    g.strokePath (p, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void DiceLookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    label.setBounds (box.getLocalBounds().reduced (8, 1).withTrimmedRight (22));
    label.setFont (fontSemibold (13.0f));
    label.setColour (juce::Label::textColourId, kText);
}

juce::Font DiceLookAndFeel::getPopupMenuFont()
{
    return fontRegular (12.0f);
}

void DiceLookAndFeel::getIdealPopupMenuItemSize (const juce::String& text,
                                                 bool isSeparator,
                                                 int,
                                                 int& idealWidth,
                                                 int& idealHeight)
{
    if (isSeparator)
    {
        idealWidth = 50;
        idealHeight = 6;
        return;
    }

    auto font = getPopupMenuFont();
    idealHeight = 24;
    idealWidth = juce::GlyphArrangement::getStringWidthInt (font, text) + idealHeight * 2;
}

void DiceLookAndFeel::drawPopupMenuSectionHeader (juce::Graphics& g,
                                                  const juce::Rectangle<int>& area,
                                                  const juce::String& sectionName)
{
    g.setColour (kSubtext);
    g.setFont (fontSemibold (11.0f));
    g.drawText (sectionName, area.reduced (6, 2), juce::Justification::centredLeft);
}

void DiceLookAndFeel::drawPopupMenuItem (juce::Graphics& g,
                                         const juce::Rectangle<int>& area,
                                         bool isSeparator,
                                         bool isActive,
                                         bool isHighlighted,
                                         bool isTicked,
                                         bool,
                                         const juce::String& text,
                                         const juce::String& shortcutKeyText,
                                         const juce::Drawable*,
                                         const juce::Colour* textColour)
{
    if (isSeparator)
    {
        g.setColour (withAlpha (kBorder, 0.7f));
        g.fillRect (area.reduced (6, 0).withHeight (1).withCentre (area.getCentre()));
        return;
    }

    auto bg = isHighlighted ? withAlpha (kAccent, 0.28f) : kCard;
    g.setColour (bg);
    g.fillRect (area);

    auto c = textColour != nullptr ? *textColour : kText;
    if (! isActive)
        c = withAlpha (kSubtext, 0.6f);

    g.setColour (c);
    g.setFont (getPopupMenuFont());
    g.drawText (text, area.reduced (8, 0), juce::Justification::centredLeft);

    if (shortcutKeyText.isNotEmpty())
        g.drawText (shortcutKeyText, area.reduced (8, 0), juce::Justification::centredRight);

    if (isTicked)
    {
        auto tickArea = juce::Rectangle<int> (area.getX(), area.getY(), 18, area.getHeight()).reduced (4);
        g.setColour (kAccent);
        g.fillEllipse (tickArea.toFloat());
    }
}

ParamSlider::ParamSlider (juce::AudioProcessorValueTreeState& apvts,
                          const juce::String& paramID,
                          const juce::String& labelText,
                          const juce::Value& lockValue,
                          juce::Slider::SliderStyle style,
                          bool showTextBox,
                          bool showLock)
{
    addAndMakeVisible (label);
    addAndMakeVisible (slider);
    addAndMakeVisible (lockButton);

    label.setText (labelText, juce::dontSendNotification);
    label.setJustificationType (style == juce::Slider::LinearHorizontal ? juce::Justification::centredLeft
                                                                        : juce::Justification::centred);
    label.setFont (fontSemibold (11.5f));
    label.setColour (juce::Label::textColourId, kSubtext);
    label.setMinimumHorizontalScale (0.65f);

    slider.setSliderStyle (style);
    slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    slider.setColour (juce::Slider::textBoxTextColourId, kText);
    slider.setColour (juce::Slider::textBoxBackgroundColourId, kCard);
    slider.setColour (juce::Slider::textBoxHighlightColourId, withAlpha (kAccent, 0.20f));

    if (showTextBox)
    {
        const auto boxPos = (style == juce::Slider::LinearHorizontal) ? juce::Slider::TextBoxRight : juce::Slider::TextBoxBelow;
        slider.setTextBoxStyle (boxPos, false, (style == juce::Slider::LinearHorizontal) ? 180 : 72, 20);
    }
    else
    {
        slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    }

    // Display formatting.
    if (paramID == "input_gain" || paramID == "output_gain")
    {
        slider.textFromValueFunction = [] (double v)
        {
            auto s = juce::String (v, 1);
            if (v > 0.0)
                s = "+" + s;
            return s + " dB";
        };
    }
    else if (paramID == "delay_time")
    {
        auto* sync = apvts.getRawParameterValue ("delay_sync");
        slider.textFromValueFunction = [sync] (double v)
        {
            if (sync != nullptr && sync->load() > 0.5f)
            {
                const auto idx = LFO::getDivisionIndexFromValue ((float) v, 1.0f, 2000.0f);
                return juce::String (LFO::getDivisions()[(size_t) idx].label);
            }

            return juce::String (std::round (v)) + " ms";
        };
    }
    else if (paramID.contains ("mix") || paramID.contains ("Mix") || paramID.endsWith ("_mix") || paramID == "mix" || paramID == "random_amount")
    {
        slider.textFromValueFunction = [] (double v) { return juce::String ((int) std::round (v * 100.0)) + "%"; };
    }
    else if (paramID == "delay_feedback")
    {
        slider.textFromValueFunction = [] (double v) { return juce::String ((int) std::round (v * 100.0)) + "%"; };
    }
    else if (paramID == "lfo_rate")
    {
        auto* sync = apvts.getRawParameterValue ("lfo_sync");
        slider.textFromValueFunction = [sync] (double v)
        {
            if (sync != nullptr && sync->load() > 0.5f)
            {
                const auto idx = LFO::getDivisionIndexFromValue ((float) v, 0.0f, 1.0f);
                return juce::String (LFO::getDivisions()[(size_t) idx].label);
            }

            const auto hz = juce::jmap (v, 0.05, 20.0);
            return juce::String (hz, 2) + " Hz";
        };
    }
    else if (paramID == "mod_rate")
    {
        slider.textFromValueFunction = [] (double v)
        {
            const auto hz = juce::jmap (v, 0.05, 6.0);
            return juce::String (hz, 2) + " Hz";
        };
    }
    else if (paramID == "lfo_depth" || paramID == "dist_drive" || paramID == "dist_tone" || paramID == "delay_filter" || paramID == "rev_size" || paramID == "rev_damp")
    {
        slider.textFromValueFunction = [] (double v) { return juce::String ((int) std::round (v * 100.0)) + "%"; };
    }
    else if (paramID == "mod_depth" || paramID == "mod_feedback")
    {
        slider.textFromValueFunction = [] (double v) { return juce::String ((int) std::round (v * 100.0)) + "%"; };
    }

    lockButton.setButtonText ("");
    lockButton.setComponentID ("lock");
    lockButton.setClickingTogglesState (true);
    lockButton.setToggleState (false, juce::dontSendNotification);
    if (showLock)
        lockButton.getToggleStateValue().referTo (lockValue);
    lockButton.setVisible (showLock);
    lockButton.setTooltip (tooltipForLock());

    const auto tt = tooltipForParam (paramID);
    if (tt.isNotEmpty())
    {
        slider.setTooltip (tt);
        label.setTooltip (tt);
    }

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, paramID, slider);
}

void ParamSlider::resized()
{
    auto area = getLocalBounds();
    auto header = area.removeFromTop (20);
    lockButton.setBounds (header.removeFromRight (20));
    label.setBounds (header.reduced (2, 0));

    slider.setBounds (area.reduced (2));
}

ParamToggle::ParamToggle (juce::AudioProcessorValueTreeState& apvts,
                          const juce::String& paramID,
                          const juce::String& labelText,
                          const juce::Value& lockValue,
                          bool showLock)
{
    addAndMakeVisible (toggle);
    addAndMakeVisible (lockButton);

    toggle.setButtonText (labelText);
    toggle.setClickingTogglesState (true);
    {
        const auto tt = tooltipForParam (paramID);
        if (tt.isNotEmpty())
            toggle.setTooltip (tt);
    }

    lockButton.setButtonText ("");
    lockButton.setComponentID ("lock");
    lockButton.setClickingTogglesState (true);
    if (showLock)
        lockButton.getToggleStateValue().referTo (lockValue);
    lockButton.setVisible (showLock);
    lockButton.setTooltip (tooltipForLock());

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, paramID, toggle);
}

void ParamToggle::resized()
{
    auto area = getLocalBounds();
    lockButton.setBounds (area.removeFromRight (18).reduced (0, 2));
    toggle.setBounds (area.reduced (2));
}

ParamCombo::ParamCombo (juce::AudioProcessorValueTreeState& apvts,
                        const juce::String& paramID,
                        const juce::String& labelText,
                        const juce::StringArray& items,
                        const juce::Value& lockValue,
                        bool showLock)
{
    addAndMakeVisible (label);
    addAndMakeVisible (combo);
    addAndMakeVisible (lockButton);

    label.setText (labelText, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centredLeft);
    label.setFont (fontSemibold (12.0f));
    label.setColour (juce::Label::textColourId, kSubtext);
    label.setMinimumHorizontalScale (0.65f);

    combo.clear();
    combo.addItemList (items, 1);
    if (combo.getNumItems() == 0)
    {
        if (auto* choice = dynamic_cast<juce::AudioParameterChoice*> (apvts.getParameter (paramID)))
        {
            for (int i = 0; i < choice->choices.size(); ++i)
                combo.addItem (choice->choices[i], i + 1);
        }
    }
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, paramID, combo);
    combo.setJustificationType (juce::Justification::centredLeft);

    if (paramID == "lfo_dest")
        combo.setTextWhenNothingSelected ("None");
    else
        combo.setTextWhenNothingSelected ("Select");
    combo.setTextWhenNoChoicesAvailable ("No choices");

    if (combo.getNumItems() > 0 && combo.getSelectedId() == 0)
        combo.setSelectedId (1, juce::dontSendNotification);

    lockButton.setButtonText ("");
    lockButton.setComponentID ("lock");
    lockButton.setClickingTogglesState (true);
    if (showLock)
        lockButton.getToggleStateValue().referTo (lockValue);
    lockButton.setVisible (showLock);
    lockButton.setTooltip (tooltipForLock());

    const auto tt = tooltipForParam (paramID);
    if (tt.isNotEmpty())
    {
        combo.setTooltip (tt);
        label.setTooltip (tt);
    }
}

void ParamCombo::resized()
{
    auto area = getLocalBounds();
    auto header = area.removeFromTop (20);
    lockButton.setBounds (header.removeFromRight (20));
    label.setBounds (header.reduced (2, 0));
    combo.setBounds (area.reduced (2));
}

DiceFXAudioProcessorEditor::DiceFXAudioProcessorEditor (DiceFXAudioProcessor& p)
    : AudioProcessorEditor (&p),
      processor (p),
      distEnable (p.getAPVTS(), "dist_enable", "Dist", p.getLockValue ("dist_enable")),
      distType (p.getAPVTS(), "dist_type", "Type", juce::StringArray { "Modern", "Vintage", "Hard" }, p.getLockValue ("dist_type"), true),
      distDrive (p.getAPVTS(), "dist_drive", "Drive", p.getLockValue ("dist_drive"), juce::Slider::RotaryHorizontalVerticalDrag, false),
      distMix (p.getAPVTS(), "dist_mix", "Mix", p.getLockValue ("dist_mix"), juce::Slider::RotaryHorizontalVerticalDrag, false),
      distTone (p.getAPVTS(), "dist_tone", "Tone", p.getLockValue ("dist_tone"), juce::Slider::RotaryHorizontalVerticalDrag, false),
      delayEnable (p.getAPVTS(), "delay_enable", "Delay", p.getLockValue ("delay_enable")),
      delayType (p.getAPVTS(), "delay_type", "Type", juce::StringArray { "Digital", "Tape", "PingPong" }, p.getLockValue ("delay_type"), true),
      delaySync (p.getAPVTS(), "delay_sync", "Sync", p.getLockValue ("delay_sync")),
      delayTime (p.getAPVTS(), "delay_time", "Time", p.getLockValue ("delay_time"), juce::Slider::RotaryHorizontalVerticalDrag, false),
      delayFeedback (p.getAPVTS(), "delay_feedback", "Feedback", p.getLockValue ("delay_feedback"), juce::Slider::RotaryHorizontalVerticalDrag, false),
      delayMix (p.getAPVTS(), "delay_mix", "Mix", p.getLockValue ("delay_mix"), juce::Slider::RotaryHorizontalVerticalDrag, false),
      delayFilter (p.getAPVTS(), "delay_filter", "Tone", p.getLockValue ("delay_filter"), juce::Slider::RotaryHorizontalVerticalDrag, false),
      revEnable (p.getAPVTS(), "rev_enable", "Reverb", p.getLockValue ("rev_enable")),
      revType (p.getAPVTS(), "rev_type", "Type", juce::StringArray { "Room", "Hall", "Plate" }, p.getLockValue ("rev_type"), true),
      revSize (p.getAPVTS(), "rev_size", "Size", p.getLockValue ("rev_size"), juce::Slider::RotaryHorizontalVerticalDrag, false),
      revDamp (p.getAPVTS(), "rev_damp", "Damp", p.getLockValue ("rev_damp"), juce::Slider::RotaryHorizontalVerticalDrag, false),
      revMix (p.getAPVTS(), "rev_mix", "Mix", p.getLockValue ("rev_mix"), juce::Slider::RotaryHorizontalVerticalDrag, false),
      modEnable (p.getAPVTS(), "mod_enable", "Mod", p.getLockValue ("mod_enable")),
      modType (p.getAPVTS(), "mod_type", "Type", juce::StringArray { "Chorus", "Flanger" }, p.getLockValue ("mod_type"), true),
      modRate (p.getAPVTS(), "mod_rate", "Rate", p.getLockValue ("mod_rate"), juce::Slider::RotaryHorizontalVerticalDrag, false),
      modDepth (p.getAPVTS(), "mod_depth", "Depth", p.getLockValue ("mod_depth"), juce::Slider::RotaryHorizontalVerticalDrag, false),
      modMix (p.getAPVTS(), "mod_mix", "Mix", p.getLockValue ("mod_mix"), juce::Slider::RotaryHorizontalVerticalDrag, false),
      modFeedback (p.getAPVTS(), "mod_feedback", "Feedback", p.getLockValue ("mod_feedback"), juce::Slider::RotaryHorizontalVerticalDrag, false),
      lfoSync (p.getAPVTS(), "lfo_sync", "Sync", p.getLockValue ("lfo_sync")),
      lfoRate (p.getAPVTS(), "lfo_rate", "Rate", p.getLockValue ("lfo_rate"), juce::Slider::RotaryHorizontalVerticalDrag, true),
      lfoDepth (p.getAPVTS(), "lfo_depth", "Depth", p.getLockValue ("lfo_depth"), juce::Slider::RotaryHorizontalVerticalDrag, true),
      lfoDest (p.getAPVTS(), "lfo_dest", "Target", juce::StringArray { "None", "Dist Drive", "Delay Time", "Delay Feedback", "Reverb Size", "Reverb Mix", "Mod Depth" }),
      inputGain (p.getAPVTS(), "input_gain", "Input", p.getLockValue ("input_gain"), juce::Slider::RotaryHorizontalVerticalDrag, true, true),
      outputGain (p.getAPVTS(), "output_gain", "Output", p.getLockValue ("output_gain"), juce::Slider::RotaryHorizontalVerticalDrag, true, true),
      globalMix (p.getAPVTS(), "mix", "Mix", p.getLockValue ("mix"), juce::Slider::RotaryHorizontalVerticalDrag, true),
      randomAmount (p.getAPVTS(), "random_amount", "Random Amount", juce::Value(), juce::Slider::RotaryHorizontalVerticalDrag, true, false)
{
    setLookAndFeel (&lookAndFeel);

    diceButton.addListener (this);
    diceButton.setComponentID ("dice");
    diceButton.setButtonText ("");
    diceButton.setTooltip ("Randomize unlocked parameters");

    saveButton.addListener (this);
    importButton.addListener (this);
    exportButton.addListener (this);
    advancedButton.addListener (this);
    advancedButton.setComponentID ("advanced");
    advancedButton.setClickingTogglesState (false);

    presetBox.addListener (this);
    presetBox.setJustificationType (juce::Justification::centredLeft);
    presetBox.setColour (juce::ComboBox::textColourId, kText);
    presetBox.setColour (juce::ComboBox::backgroundColourId, kCard);
    presetBox.setColour (juce::ComboBox::outlineColourId, kBorder);

    rebuildPresetMenu();

    addAndMakeVisible (diceButton);

    addAndMakeVisible (presetBox);
    addAndMakeVisible (saveButton);
    addAndMakeVisible (importButton);
    addAndMakeVisible (exportButton);
    addAndMakeVisible (advancedButton);

    addAndMakeVisible (distEnable);
    addAndMakeVisible (distType);
    addAndMakeVisible (distDrive);
    addAndMakeVisible (distMix);
    addAndMakeVisible (distTone);

    addAndMakeVisible (delayEnable);
    addAndMakeVisible (delayType);
    addAndMakeVisible (delaySync);
    addAndMakeVisible (delayTime);
    addAndMakeVisible (delayFeedback);
    addAndMakeVisible (delayMix);
    addAndMakeVisible (delayFilter);

    addAndMakeVisible (revEnable);
    addAndMakeVisible (revType);
    addAndMakeVisible (revSize);
    addAndMakeVisible (revDamp);
    addAndMakeVisible (revMix);

    addAndMakeVisible (modEnable);
    addAndMakeVisible (modType);
    addAndMakeVisible (modRate);
    addAndMakeVisible (modDepth);
    addAndMakeVisible (modMix);
    addAndMakeVisible (modFeedback);

    addAndMakeVisible (lfoSync);
    addAndMakeVisible (lfoRate);
    addAndMakeVisible (lfoDepth);
    addAndMakeVisible (lfoDest);

    addAndMakeVisible (inputGain);
    addAndMakeVisible (outputGain);
    addAndMakeVisible (globalMix);
    addAndMakeVisible (randomAmount);

    setSize (kCollapsedWidth, kEditorHeight);
    setAdvancedMode (false);
}

void DiceFXAudioProcessorEditor::rebuildPresetMenu (const juce::String& preferredName)
{
    processor.reloadUserPresets();

    juce::String keep = preferredName;
    if (keep.isEmpty())
        keep = presetBox.getText();

    presetBox.clear (juce::dontSendNotification);
    presetItems.clear();

    int id = 1;
    for (int i = 0; i < (int) processor.getFactoryPresets().size(); ++i)
    {
        const auto& p = processor.getFactoryPresets()[(size_t) i];
        presetBox.addItem (p.name, id);
        presetItems.push_back ({ PresetKind::Factory, i, p.name });
        ++id;
    }

    presetBox.addSeparator();

    for (int i = 0; i < (int) processor.getUserPresets().size(); ++i)
    {
        const auto& p = processor.getUserPresets()[(size_t) i];
        presetBox.addItem (p.name, id);
        presetItems.push_back ({ PresetKind::User, i, p.name });
        ++id;
    }

    int selectId = 1;
    if (keep.isNotEmpty())
    {
        for (int itemId = 1; itemId <= presetBox.getNumItems(); ++itemId)
        {
            if (presetBox.getItemText (itemId - 1) == keep)
            {
                selectId = itemId;
                break;
            }
        }
    }

    presetBox.setSelectedId (selectId, juce::dontSendNotification);
}

DiceFXAudioProcessorEditor::~DiceFXAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void DiceFXAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (kBg);

    g.setColour (kBorder);
    g.drawRoundedRectangle (frameBounds.toFloat(), 28.0f, 2.5f);

    drawCard (g, headerBounds);
    drawCard (g, diceCardBounds);
    if (advancedMode)
    {
        drawCard (g, distCardBounds);
        drawCard (g, delayCardBounds);
        drawCard (g, reverbCardBounds);
        drawCard (g, modCardBounds);
    }
    drawCard (g, lfoCardBounds);
    drawCard (g, globalCardBounds);

    auto header = headerBounds.reduced (12, 10);
    const int controlsW = juce::jmin (350, header.getWidth() / 2);
    auto titleArea = header.withTrimmedLeft (controlsW + 8).withTrimmedRight (10).withTrimmedTop (8).withTrimmedBottom (8);
    drawPixelLogo (g, titleArea);

    if (advancedMode)
    {
        auto drawModuleTitle = [&g] (juce::Rectangle<int> b, const juce::String& t)
        {
            auto top = b.reduced (12).removeFromTop (22);
            g.setColour (kSubtext);
            g.setFont (fontSemibold (14.0f));
            g.drawText (t, top, juce::Justification::centredRight, true);
        };
        drawModuleTitle (distCardBounds, "Distortion");
        drawModuleTitle (delayCardBounds, "Delay");
        drawModuleTitle (reverbCardBounds, "Reverb");
        drawModuleTitle (modCardBounds, "Modulation");
    }
    {
        g.setColour (kText);
        g.setFont (fontSemibold (18.0f));
        g.drawText ("LFO", lfoCardBounds.reduced (18).removeFromTop (28), juce::Justification::centredLeft, true);
    }
    {
        g.setColour (kText);
        g.setFont (fontSemibold (18.0f));
        g.drawText ("Global", globalCardBounds.reduced (18).removeFromTop (28), juce::Justification::centredLeft, true);
    }
}

void DiceFXAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (16);
    frameBounds = area;
    area = area.reduced (12);
    const int gap = 10;

    auto advancedStrip = area.removeFromRight (66);
    advancedButton.setBounds (advancedStrip.reduced (5, 8));

    if (advancedMode)
    {
        leftPaneBounds = area.removeFromLeft ((area.getWidth() * 56) / 100);
        area.removeFromLeft (gap);
        rightPaneBounds = area;
    }
    else
    {
        leftPaneBounds = area;
        rightPaneBounds = {};
    }

    headerBounds = leftPaneBounds.removeFromTop (104);
    auto headerInner = headerBounds.reduced (12, 10);

    auto presetArea = headerInner.removeFromLeft (juce::jmin (330, headerInner.getWidth() / 2));
    presetBox.setBounds (presetArea.removeFromTop (34));
    presetArea.removeFromTop (4);
    auto mini = presetArea.removeFromTop (34);
    saveButton.setBounds (mini.removeFromLeft (114).reduced (2, 2));
    mini.removeFromLeft (10);
    importButton.setBounds (mini.removeFromLeft (114).reduced (2, 2));
    mini.removeFromLeft (10);
    exportButton.setBounds (mini.removeFromLeft (114).reduced (2, 2));

    leftPaneBounds.removeFromTop (gap);
    auto middle = leftPaneBounds.removeFromTop (184);
    diceCardBounds = middle.removeFromLeft (210);
    middle.removeFromLeft (gap);
    lfoCardBounds = middle;

    leftPaneBounds.removeFromTop (gap);
    globalCardBounds = leftPaneBounds;

    auto diceInner = diceCardBounds.reduced (18);
    auto diceTop = diceInner.removeFromTop (juce::jmax (84, diceInner.getHeight() - 52));
    diceButton.setBounds (diceTop);
    diceInner.removeFromTop (4);
    randomAmount.setBounds (diceInner);

    if (advancedMode)
    {
        auto r = rightPaneBounds;
        const int cardGap = 10;
        auto top = r.removeFromTop ((r.getHeight() - cardGap) / 2);
        r.removeFromTop (cardGap);
        auto bottom = r;

        auto leftTop = top.removeFromLeft ((top.getWidth() - cardGap) / 2);
        top.removeFromLeft (cardGap);
        auto rightTop = top;

        auto leftBottom = bottom.removeFromLeft ((bottom.getWidth() - cardGap) / 2);
        bottom.removeFromLeft (cardGap);
        auto rightBottom = bottom;

        distCardBounds = leftTop;
        delayCardBounds = rightTop;
        reverbCardBounds = leftBottom;
        modCardBounds = rightBottom;
    }
    else
    {
        distCardBounds = {};
        delayCardBounds = {};
        reverbCardBounds = {};
        modCardBounds = {};
    }

    // LFO card layout
    {
        auto b = lfoCardBounds.reduced (10);
        b.removeFromTop (24);
        auto topRow = b.removeFromTop (24);
        lfoSync.setBounds (topRow.removeFromLeft (130));
        b.removeFromTop (2);
        auto targetArea = b.removeFromBottom (40);
        b.removeFromBottom (2);
        auto knobs = b.removeFromTop (juce::jlimit (72, 96, b.getHeight()));
        auto w = knobs.getWidth() / 2;
        lfoRate.setBounds (knobs.removeFromLeft (w).reduced (2));
        lfoDepth.setBounds (knobs.reduced (2));
        lfoDest.setBounds (targetArea);
    }

    // Global card layout
    {
        auto b = globalCardBounds.reduced (10);
        b.removeFromTop (26);
        auto row = b.removeFromTop (130);
        auto w = row.getWidth() / 3;
        inputGain.setBounds (row.removeFromLeft (w).reduced (2));
        outputGain.setBounds (row.removeFromLeft (w).reduced (2));
        globalMix.setBounds (row.reduced (2));
    }

    if (advancedMode)
    {
        auto layoutThree = [] (juce::Rectangle<int> bounds,
                               ParamToggle& enable,
                               ParamCombo& type,
                               ParamSlider& a,
                               ParamSlider& b,
                               ParamSlider& c)
        {
            auto inner = bounds.reduced (10);
            inner.removeFromTop (20);
            enable.setBounds (inner.removeFromTop (24));
            type.setBounds (inner.removeFromTop (52));
            inner.removeFromTop (2);
            auto row = inner;
            auto w = row.getWidth() / 3;
            a.setBounds (row.removeFromLeft (w).reduced (2));
            b.setBounds (row.removeFromLeft (w).reduced (2));
            c.setBounds (row.reduced (2));
        };

        auto layoutFour = [] (juce::Rectangle<int> bounds,
                              ParamToggle& enable,
                              ParamToggle* secondaryToggle,
                              ParamCombo& type,
                              ParamSlider& a,
                              ParamSlider& b,
                              ParamSlider& c,
                              ParamSlider& d)
        {
            auto inner = bounds.reduced (10);
            inner.removeFromTop (20);
            auto toggleRow = inner.removeFromTop (24);
            enable.setBounds (toggleRow.removeFromLeft (110));
            if (secondaryToggle != nullptr)
                secondaryToggle->setBounds (toggleRow.removeFromLeft (100));
            type.setBounds (inner.removeFromTop (52));
            inner.removeFromTop (2);
            auto row = inner;
            auto w = row.getWidth() / 4;
            a.setBounds (row.removeFromLeft (w).reduced (2));
            b.setBounds (row.removeFromLeft (w).reduced (2));
            c.setBounds (row.removeFromLeft (w).reduced (2));
            d.setBounds (row.reduced (2));
        };

        layoutThree (distCardBounds, distEnable, distType, distDrive, distMix, distTone);
        layoutFour (delayCardBounds, delayEnable, &delaySync, delayType, delayTime, delayFeedback, delayMix, delayFilter);
        layoutThree (reverbCardBounds, revEnable, revType, revSize, revDamp, revMix);
        layoutFour (modCardBounds, modEnable, nullptr, modType, modRate, modDepth, modMix, modFeedback);
    }
    else
    {
        distEnable.setBounds (0, 0, 0, 0);
        distType.setBounds (0, 0, 0, 0);
        distDrive.setBounds (0, 0, 0, 0);
        distMix.setBounds (0, 0, 0, 0);
        distTone.setBounds (0, 0, 0, 0);
        delayEnable.setBounds (0, 0, 0, 0);
        delayType.setBounds (0, 0, 0, 0);
        delaySync.setBounds (0, 0, 0, 0);
        delayTime.setBounds (0, 0, 0, 0);
        delayFeedback.setBounds (0, 0, 0, 0);
        delayMix.setBounds (0, 0, 0, 0);
        delayFilter.setBounds (0, 0, 0, 0);
        revEnable.setBounds (0, 0, 0, 0);
        revType.setBounds (0, 0, 0, 0);
        revSize.setBounds (0, 0, 0, 0);
        revDamp.setBounds (0, 0, 0, 0);
        revMix.setBounds (0, 0, 0, 0);
        modEnable.setBounds (0, 0, 0, 0);
        modType.setBounds (0, 0, 0, 0);
        modRate.setBounds (0, 0, 0, 0);
        modDepth.setBounds (0, 0, 0, 0);
        modMix.setBounds (0, 0, 0, 0);
        modFeedback.setBounds (0, 0, 0, 0);
    }
}

void DiceFXAudioProcessorEditor::buttonClicked (juce::Button* button)
{
    if (button == &diceButton)
    {
        processor.randomizeParameters();
        return;
    }
    if (button == &advancedButton)
    {
        setAdvancedMode (! advancedMode);
        return;
    }

    if (button == &saveButton)
    {
        auto w = std::make_unique<juce::AlertWindow> (
            "Save Preset",
            "Name this preset. It will be stored in your user preset library.",
            juce::AlertWindow::NoIcon);

        w->addTextEditor ("name", "New Preset", "Preset name:");
        w->addButton ("Cancel", 0);
        w->addButton ("Save", 1);

        auto* raw = w.release();
        juce::Component::SafePointer<juce::AlertWindow> safe (raw);

        raw->enterModalState (true,
            juce::ModalCallbackFunction::create ([this, safe] (int result)
            {
                if (result != 1 || safe == nullptr)
                    return;

                auto name = safe->getTextEditor ("name")->getText();
                if (processor.saveUserPreset (name))
                    rebuildPresetMenu (name.trim());
            }),
            true);

        return;
    }

    if (button == &importButton)
    {
        auto chooser = std::make_shared<juce::FileChooser> (
            "Import Preset",
            juce::File::getSpecialLocation (juce::File::userDocumentsDirectory),
            "*.json");

        chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this, chooser] (const juce::FileChooser& fc)
            {
                const auto file = fc.getResult();
                if (file.existsAsFile())
                    processor.importPreset (file);
            });
        return;
    }

    if (button == &exportButton)
    {
        auto chooser = std::make_shared<juce::FileChooser> (
            "Export Preset",
            juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
                .getChildFile ("DiceFX_Preset.json"),
            "*.json");

        chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
            [this, chooser] (const juce::FileChooser& fc)
            {
                auto file = fc.getResult();
                if (file == juce::File())
                    return;
                if (file.getFileExtension().isEmpty())
                    file = file.withFileExtension ("json");
                processor.exportPreset (file);
            });
        return;
    }
}

void DiceFXAudioProcessorEditor::applyAdvancedVisibility()
{
    distEnable.setVisible (advancedMode);
    distType.setVisible (advancedMode);
    distDrive.setVisible (advancedMode);
    distMix.setVisible (advancedMode);
    distTone.setVisible (advancedMode);
    delayEnable.setVisible (advancedMode);
    delayType.setVisible (advancedMode);
    delaySync.setVisible (advancedMode);
    delayTime.setVisible (advancedMode);
    delayFeedback.setVisible (advancedMode);
    delayMix.setVisible (advancedMode);
    delayFilter.setVisible (advancedMode);
    revEnable.setVisible (advancedMode);
    revType.setVisible (advancedMode);
    revSize.setVisible (advancedMode);
    revDamp.setVisible (advancedMode);
    revMix.setVisible (advancedMode);
    modEnable.setVisible (advancedMode);
    modType.setVisible (advancedMode);
    modRate.setVisible (advancedMode);
    modDepth.setVisible (advancedMode);
    modMix.setVisible (advancedMode);
    modFeedback.setVisible (advancedMode);
    randomAmount.setVisible (true);
    lfoSync.setVisible (true);
    lfoDest.setVisible (true);
    lfoRate.setVisible (true);
    lfoDepth.setVisible (true);
    inputGain.setVisible (true);
    outputGain.setVisible (true);
    globalMix.setVisible (true);

    advancedButton.setButtonText ("advanced");
}

void DiceFXAudioProcessorEditor::setAdvancedMode (bool shouldShowAdvanced)
{
    advancedMode = shouldShowAdvanced;
    applyAdvancedVisibility();
    setSize (advancedMode ? kExpandedWidth : kCollapsedWidth, kEditorHeight);
    resized();
    repaint();
}

void DiceFXAudioProcessorEditor::comboBoxChanged (juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &presetBox)
    {
        const int selectedIndex = presetBox.getSelectedItemIndex();
        if (selectedIndex < 0)
            return;

        // presetItems contains both factory and user presets, in display order.
        // ComboBox separators are not counted as selectable items.
        if (selectedIndex >= (int) presetItems.size())
            return;

        const auto item = presetItems[(size_t) selectedIndex];
        if (item.kind == PresetKind::Factory)
            processor.applyFactoryPreset (item.index);
        else
            processor.applyUserPreset (item.index);
    }
}
