#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class DiceLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    DiceLookAndFeel();

    void drawRotarySlider (juce::Graphics&,
                           int x, int y, int width, int height,
                           float sliderPosProportional,
                           float rotaryStartAngle,
                           float rotaryEndAngle,
                           juce::Slider&) override;

    void drawLinearSlider (juce::Graphics&,
                           int x, int y, int width, int height,
                           float sliderPos,
                           float minSliderPos,
                           float maxSliderPos,
                           const juce::Slider::SliderStyle,
                           juce::Slider&) override;

    int getSliderThumbRadius (juce::Slider&) override;

    void drawToggleButton (juce::Graphics&,
                           juce::ToggleButton&,
                           bool shouldDrawButtonAsHighlighted,
                           bool shouldDrawButtonAsDown) override;

    void drawButtonBackground (juce::Graphics&,
                               juce::Button&,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override;

    void drawButtonText (juce::Graphics&,
                         juce::TextButton&,
                         bool shouldDrawButtonAsHighlighted,
                         bool shouldDrawButtonAsDown) override;

    void drawComboBox (juce::Graphics&,
                       int width, int height,
                       bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       juce::ComboBox&) override;

    void drawPopupMenuItem (juce::Graphics&,
                            const juce::Rectangle<int>& area,
                            bool isSeparator,
                            bool isActive,
                            bool isHighlighted,
                            bool isTicked,
                            bool hasSubMenu,
                            const juce::String& text,
                            const juce::String& shortcutKeyText,
                            const juce::Drawable* icon,
                            const juce::Colour* textColour) override;

    void drawPopupMenuSectionHeader (juce::Graphics&,
                                     const juce::Rectangle<int>& area,
                                     const juce::String& sectionName) override;

    juce::Font getPopupMenuFont() override;
    void getIdealPopupMenuItemSize (const juce::String& text,
                                    bool isSeparator,
                                    int standardMenuItemHeight,
                                    int& idealWidth,
                                    int& idealHeight) override;

    void positionComboBoxText (juce::ComboBox&, juce::Label&) override;

    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;
};

class ParamSlider final : public juce::Component
{
public:
    ParamSlider (juce::AudioProcessorValueTreeState& apvts,
                 const juce::String& paramID,
                 const juce::String& labelText,
                 const juce::Value& lockValue,
                 juce::Slider::SliderStyle style = juce::Slider::RotaryHorizontalVerticalDrag,
                 bool showTextBox = true,
                 bool showLock = true);

    void resized() override;

private:
    juce::Label label;
    juce::Slider slider;
    juce::ToggleButton lockButton;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParamSlider)
};

class ParamToggle final : public juce::Component
{
public:
    ParamToggle (juce::AudioProcessorValueTreeState& apvts,
                 const juce::String& paramID,
                 const juce::String& labelText,
                 const juce::Value& lockValue,
                 bool showLock = true);

    void resized() override;

private:
    juce::ToggleButton toggle;
    juce::ToggleButton lockButton;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParamToggle)
};

class ParamCombo final : public juce::Component
{
public:
    ParamCombo (juce::AudioProcessorValueTreeState& apvts,
                const juce::String& paramID,
                const juce::String& labelText,
                const juce::StringArray& items,
                const juce::Value& lockValue = {},
                bool showLock = false);

    void resized() override;

private:
    juce::Label label;
    juce::ComboBox combo;
    juce::ToggleButton lockButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParamCombo)
};

class DiceFXAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                         private juce::Button::Listener,
                                         private juce::ComboBox::Listener
{
public:
    explicit DiceFXAudioProcessorEditor (DiceFXAudioProcessor&);
    ~DiceFXAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void buttonClicked (juce::Button* button) override;
    void comboBoxChanged (juce::ComboBox* comboBoxThatHasChanged) override;
    void rebuildPresetMenu (const juce::String& preferredName = {});
    void applyAdvancedVisibility();
    void setAdvancedMode (bool shouldShowAdvanced);

    DiceFXAudioProcessor& processor;
    DiceLookAndFeel lookAndFeel;

    juce::TextButton diceButton { "Roll" };

    juce::ComboBox presetBox;
    juce::TextButton saveButton { "Save" };
    juce::TextButton importButton { "Import" };
    juce::TextButton exportButton { "Export" };
    juce::TextButton advancedButton { "advanced" };

    ParamToggle distEnable;
    ParamCombo distType;
    ParamSlider distDrive;
    ParamSlider distMix;
    ParamSlider distTone;

    ParamToggle delayEnable;
    ParamCombo delayType;
    ParamToggle delaySync;
    ParamSlider delayTime;
    ParamSlider delayFeedback;
    ParamSlider delayMix;
    ParamSlider delayFilter;

    ParamToggle revEnable;
    ParamCombo revType;
    ParamSlider revSize;
    ParamSlider revDamp;
    ParamSlider revMix;

    ParamToggle modEnable;
    ParamCombo modType;
    ParamSlider modRate;
    ParamSlider modDepth;
    ParamSlider modMix;
    ParamSlider modFeedback;

    ParamToggle lfoSync;
    ParamSlider lfoRate;
    ParamSlider lfoDepth;
    ParamCombo lfoDest;

    ParamSlider inputGain;
    ParamSlider outputGain;
    ParamSlider globalMix;
    ParamSlider randomAmount;

    bool advancedMode = false;

    juce::Rectangle<int> frameBounds;
    juce::Rectangle<int> leftPaneBounds;
    juce::Rectangle<int> rightPaneBounds;
    juce::Rectangle<int> headerBounds;
    juce::Rectangle<int> diceCardBounds;
    juce::Rectangle<int> distCardBounds;
    juce::Rectangle<int> delayCardBounds;
    juce::Rectangle<int> reverbCardBounds;
    juce::Rectangle<int> modCardBounds;
    juce::Rectangle<int> lfoCardBounds;
    juce::Rectangle<int> globalCardBounds;

    enum class PresetKind { Factory, User };
    struct PresetItem { PresetKind kind; int index = -1; juce::String name; };
    std::vector<PresetItem> presetItems;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DiceFXAudioProcessorEditor)
};
