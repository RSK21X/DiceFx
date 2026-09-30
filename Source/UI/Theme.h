#pragma once

#include <JuceHeader.h>

namespace DiceTheme
{
inline const juce::Colour background { 0xff09090b };
inline const juce::Colour surface { 0xff151517 };
inline const juce::Colour elevated { 0xff202023 };
inline const juce::Colour border { 0xff303034 };
inline const juce::Colour borderStrong { 0xff4b4b50 };
inline const juce::Colour textPrimary { 0xfff2f2f3 };
inline const juce::Colour textSecondary { 0xffa9a9af };
inline const juce::Colour textDisabled { 0xff6f6f76 };
inline const juce::Colour track { 0xff343439 };
inline const juce::Colour accent { 0xffffd629 };
inline const juce::Colour accentHover { 0xffffe575 };
inline const juce::Colour white { 0xfff2f2f3 };

constexpr float outerRadius = 6.0f;
constexpr float cardRadius = 5.0f;
constexpr float controlRadius = 3.0f;
constexpr float pillRadius = 999.0f;

inline juce::Font regularFont (float size)
{
    return juce::Font (juce::FontOptions (size, juce::Font::plain));
}

inline juce::Font mediumFont (float size)
{
    return juce::Font (juce::FontOptions (size, juce::Font::bold));
}

inline juce::Colour withAlpha (juce::Colour colour, float alpha)
{
    return colour.withAlpha (juce::jlimit (0.0f, 1.0f, alpha));
}

inline void drawPanel (juce::Graphics& g, juce::Rectangle<float> bounds,
                       juce::Colour fill = surface, float radius = cardRadius,
                       bool elevatedShadow = true)
{
    juce::ignoreUnused (elevatedShadow);
    g.setColour (fill);
    g.fillRoundedRectangle (bounds, radius);
    g.setColour (border);
    g.drawRoundedRectangle (bounds.reduced (0.5f), radius, 1.0f);
}

inline void drawLabel (juce::Graphics& g, const juce::String& text,
                       juce::Rectangle<int> bounds, float size,
                       juce::Colour colour = textSecondary,
                       juce::Justification justification = juce::Justification::centredLeft,
                       bool uppercase = false)
{
    g.setColour (colour);
    g.setFont (regularFont (size));
    g.drawText (uppercase ? text.toUpperCase() : text, bounds, justification, false);
}

inline void drawSectionLabel (juce::Graphics& g, const juce::String& text,
                              juce::Rectangle<int> bounds)
{
    g.setColour (textSecondary);
    g.setFont (mediumFont (10.0f));
    g.drawText (text.toUpperCase(), bounds, juce::Justification::centredLeft, false);
}
}
