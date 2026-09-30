#pragma once

#include <JuceHeader.h>
#include "../Theme.h"
#include "../../PluginProcessor.h"

class HeaderBar final : public juce::Component,
                        private juce::Button::Listener,
                        private juce::ComboBox::Listener
{
public:
    explicit HeaderBar (DiceFXAudioProcessor&);

    void paint (juce::Graphics&) override;
    void resized() override;

    void rebuildPresetMenu (const juce::String& preferredName = {});
    void setHistoryAvailability (bool canPrevious, bool canNext);
    void setABState (bool showingB);
    void setAdvancedState (bool open);

    std::function<void()> onPrevious;
    std::function<void()> onNext;
    std::function<void()> onAB;
    std::function<void()> onSave;
    std::function<void()> onImport;
    std::function<void()> onExport;
    std::function<void()> onPresetChanged;
    std::function<void()> onAdvanced;

private:
    enum class PresetKind { Factory, User };
    struct PresetItem { PresetKind kind; int index = -1; juce::String name; };

    void buttonClicked (juce::Button*) override;
    void comboBoxChanged (juce::ComboBox*) override;
    void showMenu();
    void drawPixelLogo (juce::Graphics&, juce::Rectangle<int>) const;

    DiceFXAudioProcessor& processor;
    juce::ComboBox presetBox;
    juce::TextButton previousButton;
    juce::TextButton nextButton;
    juce::TextButton abButton { "A / B" };
    juce::TextButton saveButton { "Save" };
    juce::TextButton menuButton { "..." };
    juce::TextButton advancedButton { "ADV" };
    std::vector<PresetItem> presetItems;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HeaderBar)
};
