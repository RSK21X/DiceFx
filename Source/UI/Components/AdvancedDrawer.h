#pragma once

#include <JuceHeader.h>
#include "../Theme.h"
#include "../../PluginProcessor.h"

class LockMatrixContent final : public juce::Component
{
public:
    explicit LockMatrixContent (DiceFXAudioProcessor&);

    void paint (juce::Graphics&) override;
    void resized() override;
    int getContentHeight() const;

    std::function<void()> onChanged;

private:
    struct Row
    {
        juce::String parameterID;
        juce::String label;
        std::unique_ptr<juce::ToggleButton> toggle;
        std::unique_ptr<juce::Label> text;
    };

    struct Group
    {
        juce::String title;
        std::vector<Row> rows;
    };

    DiceFXAudioProcessor& processor;
    std::vector<Group> groups;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LockMatrixContent)
};

class AdvancedDrawer final : public juce::Component
{
public:
    explicit AdvancedDrawer (DiceFXAudioProcessor&);

    void paint (juce::Graphics&) override;
    void resized() override;

    std::function<void()> onClose;
    std::function<void()> onLocksChanged;

private:
    DiceFXAudioProcessor& processor;
    juce::TextButton closeButton { "Close" };
    juce::Viewport viewport;
    LockMatrixContent content;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AdvancedDrawer)
};
