#pragma once
#include <JuceHeader.h>
#include <BinaryData.h>
#include <array>

namespace DiceIcons
{
enum class Icon { Back, Forward, Down, Menu, Close, Lock, Unlock, PartialLock, Check, Dice, Save };

inline void draw (juce::Graphics& g, Icon icon, juce::Rectangle<float> bounds, juce::Colour colour)
{
    struct Cache
    {
        std::array<std::unique_ptr<juce::Drawable>, 11> images;
        Cache()
        {
            const char* names[] = { "icon_back_svg", "icon_forward_svg", "icon_down_svg", "icon_menu_svg",
                "icon_close_svg", "icon_lock_svg", "icon_unlock_svg", "icon_partial_lock_svg",
                "icon_check_svg", "icon_dice_svg", "icon_save_svg" };
            for (size_t i = 0; i < images.size(); ++i)
            {
                int size = 0;
                if (auto* resource = BinaryData::getNamedResource (names[i], size))
                    if (auto xml = juce::XmlDocument::parse (juce::String::fromUTF8 (resource, size)))
                        images[i] = juce::Drawable::createFromSVG (*xml);
                jassert (images[i] != nullptr);
            }
        }
    };
    static Cache cache;
    if (auto* source = cache.images[(size_t) icon].get())
    {
        // Copy before tinting so disabled/hover states never alter the cached SVG.
        auto tinted = source->createCopy();
        tinted->replaceColour (juce::Colours::white, colour);
        tinted->drawWithin (g, bounds, juce::RectanglePlacement::centred, 1.0f);
    }
}
}
