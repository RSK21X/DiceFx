#include "PluginEditor.h"
#include "UI/Icons.h"
#include <iostream>
#include <stdexcept>

namespace
{
void require (bool condition, const juce::String& message)
{
    if (! condition) throw std::runtime_error (message.toStdString());
}
void pump()
{
    juce::MessageManager::getInstance()->runDispatchLoopUntil (100);
}
bool same (const DiceParameterSnapshot& a, const DiceParameterSnapshot& b)
{
    if (a.values.size() != b.values.size()) return false;
    for (size_t i = 0; i < a.values.size(); ++i)
        if (a.values[i].first != b.values[i].first || std::abs (a.values[i].second - b.values[i].second) > 0.00001f)
            return false;
    return true;
}
void visit (juce::Component& root, const std::function<void(juce::Component&)>& fn)
{
    fn (root);
    for (auto* child : root.getChildren()) visit (*child, fn);
}
juce::Button& button (juce::Component& root, const juce::String& id)
{
    juce::Button* found = nullptr;
    visit (root, [&] (juce::Component& c)
    {
        if (c.getComponentID() == id) found = dynamic_cast<juce::Button*> (&c);
    });
    require (found != nullptr, "Missing button: " + id);
    return *found;
}
void click (juce::Component& root, const juce::String& id)
{
    auto& target = button (root, id);
    require (target.isEnabled(), "Disabled button: " + id);
    target.triggerClick();
    pump();
}
float raw (DiceFXAudioProcessor& p, const juce::String& id)
{
    return p.getAPVTS().getRawParameterValue (id)->load();
}
void set (DiceFXAudioProcessor& p, const juce::String& id, float value)
{
    auto* parameter = p.getAPVTS().getParameter (id);
    parameter->setValueNotifyingHost (parameter->convertTo0to1 (value));
    pump();
}
void snapshot (juce::Component& editor, const juce::File& dir, const juce::String& name)
{
    auto image = editor.createComponentSnapshot (editor.getLocalBounds(), true, 2);
    auto file = dir.getChildFile (name + ".png");
    auto stream = file.createOutputStream();
    require (stream != nullptr, "Cannot write screenshot");
    require (stream->setPosition (0) && stream->truncate().wasOk(), "Cannot replace screenshot");
    juce::PNGImageFormat png;
    require (png.writeImageToStream (image, *stream), "Cannot encode screenshot");
}
void geometry (juce::Component& root)
{
    int sliders = 0;
    visit (root, [&] (juce::Component& c)
    {
        if (! c.isVisible()) return;
        if (auto* slider = dynamic_cast<juce::Slider*> (&c))
        {
            ++sliders;
            require (slider->getWidth() > 0 && slider->getHeight() > 0, "Empty slider");
            if (slider->getSliderStyle() == juce::Slider::RotaryHorizontalVerticalDrag)
                require (slider->getWidth() == slider->getHeight(), "Non-circular knob");
        }
        if (dynamic_cast<EffectCard*> (&c) || dynamic_cast<ParameterKnob*> (&c)
            || dynamic_cast<HeaderBar*> (&c) || dynamic_cast<LfoRoutingStrip*> (&c)
            || dynamic_cast<UtilityBar*> (&c) || dynamic_cast<RandomizeHero*> (&c)
            || dynamic_cast<ParameterCombo*> (&c))
        {
            const auto& children = c.getChildren();
            for (int i = 0; i < children.size(); ++i)
            {
                auto* a = children[i];
                if (! a->isVisible() || a->getBounds().isEmpty()) continue;
                require (c.getLocalBounds().contains (a->getBounds()), "Control outside section");
                for (int j = i + 1; j < children.size(); ++j)
                    if (children[j]->isVisible() && ! children[j]->getBounds().isEmpty())
                        require (! a->getBounds().intersects (children[j]->getBounds()), "Overlapping controls");
            }
        }
    });
    require (sliders == 20, "Unexpected number of parameter sliders");
}
}

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI init;
    try
    {
        require (argc == 2, "Pass screenshot output directory");
        const juce::File output (argv[1]);
        require (output.createDirectory().wasOk(), "Cannot create output directory");
        for (int i = 0; i < 11; ++i)
        {
            juce::Image icon (juce::Image::ARGB, 32, 32, true);
            juce::Graphics g (icon);
            DiceIcons::draw (g, (DiceIcons::Icon) i, { 0, 0, 32, 32 }, DiceTheme::accent);
            int pixels = 0;
            for (int y = 0; y < 32; ++y)
                for (int x = 0; x < 32; ++x)
                    if (icon.getPixelAt (x, y).getAlpha() > 0) ++pixels;
            require (pixels > 20, "SVG icon did not render: " + juce::String (i));
        }
        DiceFXAudioProcessor processor;
        DiceFXAudioProcessorEditor editor (processor);
        editor.setVisible (true);
        pump();
        for (const auto size : { juce::Point<int> (720, 450), { 800, 500 }, { 1120, 700 }, { 900, 600 } })
        {
            editor.setSize (size.x, size.y);
            pump();
            geometry (editor);
            snapshot (editor, output, "dicefx-" + juce::String (size.x) + "x" + juce::String (size.y));
        }
        editor.setSize (800, 500);
        // Knob -> parameter and host parameter -> knob work in both directions.
        int knobs = 0;
        visit (editor, [&] (juce::Component& c)
        {
            if (auto* knob = dynamic_cast<ParameterKnob*> (&c))
            {
                ++knobs;
                auto& slider = knob->getSlider();
                auto* parameter = processor.getAPVTS().getParameter (knob->getParameterID());
                const double value = parameter->convertFrom0to1 (0.37f);
                slider.setValue (value, juce::sendNotificationSync);
                require (std::abs (raw (processor, knob->getParameterID()) - slider.getValue()) < 0.01,
                         "Knob is not attached to parameter");
                parameter->setValueNotifyingHost (0.61f);
                pump();
                require (std::abs (raw (processor, knob->getParameterID()) - slider.getValue()) < 0.01,
                         "Host update did not reach knob");
            }
        });
        require (knobs == 16, "Unexpected knob count");
        int combos = 0, toggles = 0;
        visit (editor, [&] (juce::Component& c)
        {
            if (auto* combo = dynamic_cast<ParameterCombo*> (&c))
            {
                ++combos;
                auto& box = combo->getCombo();
                const int next = box.getSelectedId() % box.getNumItems() + 1;
                box.setSelectedId (next, juce::sendNotificationSync);
                pump();
                require ((int) raw (processor, box.getTooltip()) == next - 1, "Choice control is not attached");
            }
            if (auto* toggle = dynamic_cast<ParameterToggle*> (&c))
            {
                ++toggles;
                auto& b = toggle->getButton();
                b.triggerClick(); pump();
                require ((raw (processor, b.getTooltip()) > 0.5f) == b.getToggleState(), "Toggle is not attached");
            }
        });
        require (combos == 5 && toggles == 6, "Unexpected choice or toggle count");
        juce::ComboBox* presets = nullptr;
        visit (editor, [&] (juce::Component& c)
        {
            if (auto* box = dynamic_cast<juce::ComboBox*> (&c))
                if (box->getTooltip() == "Choose a DiceFX preset.") presets = box;
        });
        require (presets != nullptr && presets->getNumItems() >= 2, "Missing factory presets");
        presets->setSelectedId (2, juce::sendNotificationSync); pump();
        const auto& preset = processor.getFactoryPresets()[1];
        for (int i = 0; i < preset.params.size(); ++i)
            require (std::abs (raw (processor, preset.params.getName (i).toString())
                              - (float) preset.params.getValueAt (i)) < 0.01f, "Preset did not update parameter");
        presets->setSelectedId (1, juce::sendNotificationSync); pump();
        // Start a fresh editor so undo has a known initial snapshot.
        DiceFXAudioProcessorEditor interactions (processor);
        interactions.setVisible (true);
        RandomHistory history (processor.getAPVTS());
        const auto initial = history.captureCurrent();
        ModuleLockButton* moduleLock = nullptr;
        visit (interactions, [&] (juce::Component& c)
        {
            if (auto* effect = dynamic_cast<EffectCard*> (&c))
                if (effect->getKind() == EffectCard::Kind::Distortion)
                    moduleLock = &effect->getModuleLockButton();
        });
        require (moduleLock != nullptr, "Missing module lock");
        moduleLock->triggerClick(); pump();
        require (processor.isLocked ("dist_drive"), "Module lock failed");
        const float lockedDrive = raw (processor, "dist_drive");
        click (interactions, "hero-randomize");
        require (raw (processor, "dist_drive") == lockedDrive, "Randomization changed locked control");
        const auto randomized = history.captureCurrent();
        require (! same (initial, randomized), "Randomize made no changes");
        click (interactions, "header-prev");
        require (same (initial, history.captureCurrent()), "Undo did not restore sound");
        click (interactions, "header-next");
        require (same (randomized, history.captureCurrent()), "Redo did not restore sound");
        click (interactions, "header-ab");
        set (processor, "rev_mix", 0.19f);
        click (interactions, "header-ab");
        require (same (randomized, history.captureCurrent()), "A/B did not restore A");
        click (interactions, "header-ab");
        require (std::abs (raw (processor, "rev_mix") - 0.19f) < 0.001f, "A/B did not restore B");
        click (interactions, "header-advanced");
        snapshot (interactions, output, "dicefx-locks");
        click (interactions, "drawer-close");
        // Sync mode changes display units even when the rate itself is unchanged.
        set (processor, "delay_sync", 1);
        set (processor, "lfo_sync", 1);
        pump();
        for (const auto& id : { juce::String ("delay_time"), juce::String ("lfo_rate") })
            visit (interactions, [&] (juce::Component& c)
            {
                if (auto* knob = dynamic_cast<ParameterKnob*> (&c))
                    if (knob->getParameterID() == id)
                    {
                        auto* value = dynamic_cast<juce::Label*> (knob->getChildComponent (2));
                        require (value != nullptr && value->getText().contains ("/"), "Sync display did not update");
                    }
            });
        snapshot (interactions, output, "dicefx-randomized");
        // Offline smoke check: no audio-device access or real-time output.
        for (const double sampleRate : { 44100.0, 48000.0, 96000.0 })
            for (const int blockSize : { 64, 512 })
            {
                processor.prepareToPlay (sampleRate, blockSize);
                for (int presetIndex = 0; presetIndex < (int) processor.getFactoryPresets().size(); ++presetIndex)
                {
                    processor.applyFactoryPreset (presetIndex);
                    juce::AudioBuffer<float> audio (2, blockSize);
                    juce::MidiBuffer midi;
                    for (int block = 0; block < 32; ++block)
                    {
                        for (int channel = 0; channel < 2; ++channel)
                            for (int sample = 0; sample < blockSize; ++sample)
                                audio.setSample (channel, sample, 0.1f * std::sin (
                                    (block * blockSize + sample) * juce::MathConstants<double>::twoPi * 440 / sampleRate));
                        processor.processBlock (audio, midi);
                        for (int channel = 0; channel < 2; ++channel)
                            for (int sample = 0; sample < blockSize; ++sample)
                                require (std::isfinite (audio.getSample (channel, sample)), "Non-finite audio output");
                    }
                }
                processor.releaseResources();
            }
        std::cout << "PASS: 11 SVG icons, four sizes, circular knobs, no overlapping controls, 16 knob bindings, five choices, six toggles, factory preset, module lock, randomize, undo/redo, A/B, lock drawer, sync displays, and finite audio for all factory presets at three sample rates and two block sizes.\n";
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
}
