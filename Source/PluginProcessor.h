#pragma once

#include <JuceHeader.h>
#include "Dsp/Distortion.h"
#include "Dsp/Delay.h"
#include "Dsp/Reverb.h"
#include "Dsp/Modulation.h"
#include "Mod/LFO.h"
#include "Random/Randomizer.h"
#include "Presets/Presets.h"

class DiceFXAudioProcessor final : public juce::AudioProcessor
{
public:
    DiceFXAudioProcessor();
    ~DiceFXAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

    void randomizeParameters();

    juce::Value getLockValue (const juce::String& paramID);
    bool isLocked (const juce::String& paramID) const;
    void setLocked (const juce::String& paramID, bool locked);

    const std::vector<Preset>& getFactoryPresets() const { return presets.getFactoryPresets(); }
    const std::vector<Preset>& getUserPresets() const { return presets.getUserPresets(); }
    bool applyFactoryPreset (int index) { return presets.applyFactoryPreset(index); }
    bool applyUserPreset (int index) { return presets.applyUserPreset(index); }
    void reloadUserPresets() { presets.reloadUserPresets(); }
    bool saveUserPreset (const juce::String& name) { return presets.saveUserPreset(name); }
    bool importPreset (const juce::File& file) { return presets.importPresetFromFile(file); }
    bool exportPreset (const juce::File& file) { return presets.exportPresetToFile(file); }

    static const juce::StringArray& getParameterIDs();

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    double getHostBpm() const;
    float applyLfoToRange (float baseValue, float minValue, float maxValue, float lfoValue) const;

    juce::AudioProcessorValueTreeState apvts;
    juce::ValueTree lockState;

    Randomizer randomizer;
    Presets presets;

    LFO lfo;
    Distortion distortion;
    Modulation modulation;
    Delay delay;
    DiceReverb reverb;

    juce::AudioBuffer<float> dryBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DiceFXAudioProcessor)
};
