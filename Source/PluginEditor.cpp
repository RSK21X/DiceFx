#include "PluginEditor.h"

#include <cmath>

namespace
{
juce::StringArray changedParameters (const DiceParameterSnapshot& before,
                                     const DiceParameterSnapshot& after)
{
    juce::StringArray result;

    for (const auto& [id, value] : after.values)
        for (const auto& [beforeID, beforeValue] : before.values)
            if (id == beforeID && std::abs (value - beforeValue) > 0.00001f)
            {
                result.addIfNotAlreadyThere (id);
                break;
            }

    return result;
}
}

DiceFXAudioProcessorEditor::DiceFXAudioProcessorEditor (DiceFXAudioProcessor& p)
    : AudioProcessorEditor (&p),
      processor (p),
      history (p.getAPVTS()),
      header (p),
      hero (p),
      distortion (p, EffectCard::Kind::Distortion),
      delay (p, EffectCard::Kind::Delay),
      reverb (p, EffectCard::Kind::Reverb),
      modulation (p, EffectCard::Kind::Modulation),
      lfoStrip (p),
      utilityBar (p),
      advancedDrawer (p)
{
    setLookAndFeel (&lookAndFeel);
    setResizable (true, true);
    setResizeLimits (720, 450, 1120, 700);
    if (auto* constrainer = getConstrainer())
        constrainer->setFixedAspectRatio (800.0 / 500.0);

    addAndMakeVisible (panel);
    for (auto* component : std::initializer_list<juce::Component*> {
            &header, &hero, &distortion, &delay, &reverb, &modulation,
            &lfoStrip, &utilityBar, &advancedDrawer })
        panel.addAndMakeVisible (component);
    advancedDrawer.setVisible (false);

    hero.onRandomize = [this] { randomize(); };
    hero.onPrevious = [this] { restorePrevious(); };
    hero.onNext = [this] { restoreNext(); };

    header.onPrevious = [this] { restorePrevious(); };
    header.onNext = [this] { restoreNext(); };
    header.onAB = [this] { toggleAB(); };
    header.onAdvanced = [this] { setAdvancedMode (! advancedMode); };
    header.onSave = [this] { savePreset(); };
    header.onImport = [this] { importPreset(); };
    header.onExport = [this] { exportPreset(); };
    header.onPresetChanged = [this]
    {
        refreshParameterDisplays();
        refreshModuleLocks();
    };

    advancedDrawer.onClose = [this] { setAdvancedMode (false); };
    advancedDrawer.onLocksChanged = [this] { refreshModuleLocks(); };

    auto refreshCards = [this]
    {
        distortion.refreshLockState();
        delay.refreshLockState();
        reverb.refreshLockState();
        modulation.refreshLockState();
    };
    distortion.onLocksChanged = refreshCards;
    delay.onLocksChanged = refreshCards;
    reverb.onLocksChanged = refreshCards;
    modulation.onLocksChanged = refreshCards;

    aState = history.captureCurrent();
    updateHistoryControls();
    setSize (800, 500);
}

DiceFXAudioProcessorEditor::~DiceFXAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void DiceFXAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (DiceTheme::background.darker (0.3f));
    g.addTransform (panel.getTransform());
    auto chassis = juce::Rectangle<float> (0, 0, 800, 500).reduced (1);
    g.setColour (DiceTheme::background);
    g.fillRoundedRectangle (chassis, 7);
    g.setColour (DiceTheme::borderStrong.withAlpha (0.55f));
    g.drawRoundedRectangle (chassis, 7, 1);
    DiceTheme::drawLabel (g, "DRAG TO ADJUST  /  DOUBLE-CLICK TO RESET  /  LOCK TO KEEP",
                          { 16, 476, 768, 16 }, 10, DiceTheme::textSecondary,
                          juce::Justification::centred);
}

void DiceFXAudioProcessorEditor::resized()
{
    // A single transform preserves every control's geometry at any host size.
    const float scale = juce::jmin (getWidth() / 800.0f, getHeight() / 500.0f);
    panel.setBounds (0, 0, 800, 500);
    panel.setTransform (juce::AffineTransform::scale (scale).translated (
        (getWidth() - 800 * scale) * 0.5f, (getHeight() - 500 * scale) * 0.5f));
    header.setBounds (16, 12, 768, 36);
    hero.setBounds (16, 54, 410, 44);
    distortion.setBounds (16, 106, 380, 142);
    delay.setBounds (404, 106, 380, 142);
    reverb.setBounds (16, 256, 380, 142);
    modulation.setBounds (404, 256, 380, 142);
    lfoStrip.setBounds (16, 406, 768, 64);
    // Global controls share the randomization strip, avoiding a second oversized footer.
    utilityBar.setBounds (432, 54, 352, 44);
    advancedDrawer.setBounds (510, 106, 274, 364);
    if (advancedMode)
        advancedDrawer.toFront (false);
}

void DiceFXAudioProcessorEditor::randomize()
{
    const auto before = history.captureCurrent();
    processor.randomizeParameters();
    const auto after = history.captureCurrent();

    if (history.pushCurrent())
    {
        const auto changed = changedParameters (before, after);
        distortion.flashParameters (changed);
        delay.flashParameters (changed);
        reverb.flashParameters (changed);
        modulation.flashParameters (changed);
        hero.flashRandomize();
    }

    refreshParameterDisplays();
    updateHistoryControls();
}

void DiceFXAudioProcessorEditor::restorePrevious()
{
    if (history.previous())
    {
        refreshParameterDisplays();
        updateHistoryControls();
    }
}

void DiceFXAudioProcessorEditor::restoreNext()
{
    if (history.next())
    {
        refreshParameterDisplays();
        updateHistoryControls();
    }
}

void DiceFXAudioProcessorEditor::toggleAB()
{
    if (showingB)
    {
        bState = history.captureCurrent();
        if (aState.has_value())
            history.restore (*aState);
        showingB = false;
    }
    else
    {
        aState = history.captureCurrent();
        if (! bState.has_value())
            bState = aState;
        if (bState.has_value())
            history.restore (*bState);
        showingB = true;
    }

    refreshParameterDisplays();
    header.setABState (showingB);
    updateHistoryControls();
    repaint();
}

void DiceFXAudioProcessorEditor::setAdvancedMode (bool shouldShowAdvanced)
{
    advancedMode = shouldShowAdvanced;
    advancedDrawer.setVisible (advancedMode);
    header.setAdvancedState (advancedMode);
    if (advancedMode)
        advancedDrawer.toFront (false);
    resized();
    repaint();
}

void DiceFXAudioProcessorEditor::updateHistoryControls()
{
    header.setHistoryAvailability (history.canPrevious(), history.canNext());
    hero.setHistoryAvailability (history.canPrevious(), history.canNext());
}

void DiceFXAudioProcessorEditor::refreshParameterDisplays()
{
    distortion.refreshDisplays();
    delay.refreshDisplays();
    reverb.refreshDisplays();
    modulation.refreshDisplays();
    lfoStrip.refreshDisplays();
}

void DiceFXAudioProcessorEditor::refreshModuleLocks()
{
    distortion.refreshLockState();
    delay.refreshLockState();
    reverb.refreshLockState();
    modulation.refreshLockState();
}

void DiceFXAudioProcessorEditor::savePreset()
{
    auto window = std::make_unique<juce::AlertWindow> (
        "Save Preset", "Name this preset for your DiceFX library.", juce::AlertWindow::NoIcon);
    window->addTextEditor ("name", "New Preset", "Preset name:");
    window->addButton ("Cancel", 0);
    window->addButton ("Save", 1);

    auto* raw = window.release();
    juce::Component::SafePointer<juce::AlertWindow> safe (raw);
    raw->enterModalState (true, juce::ModalCallbackFunction::create ([this, safe] (int result)
    {
        if (result != 1 || safe == nullptr)
            return;

        const auto name = safe->getTextEditor ("name")->getText().trim();
        if (processor.saveUserPreset (name))
            header.rebuildPresetMenu (name);
    }), true);
}

void DiceFXAudioProcessorEditor::importPreset()
{
    auto chooser = std::make_shared<juce::FileChooser> (
        "Import Preset", juce::File::getSpecialLocation (juce::File::userDocumentsDirectory), "*.json");
    chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                          [this, chooser] (const juce::FileChooser& fileChooser)
    {
        const auto file = fileChooser.getResult();
        if (file.existsAsFile())
        {
            if (processor.importPreset (file))
            {
                refreshParameterDisplays();
                refreshModuleLocks();
            }
        }
    });
}

void DiceFXAudioProcessorEditor::exportPreset()
{
    auto chooser = std::make_shared<juce::FileChooser> (
        "Export Preset",
        juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("DiceFX_Preset.json"),
        "*.json");
    chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
                          [this, chooser] (const juce::FileChooser& fileChooser)
    {
        auto file = fileChooser.getResult();
        if (file == juce::File())
            return;
        if (file.getFileExtension().isEmpty())
            file = file.withFileExtension ("json");
        processor.exportPreset (file);
    });
}
