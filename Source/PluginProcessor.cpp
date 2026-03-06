#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
constexpr float kMinGainDb = -24.0f;
constexpr float kMaxGainDb = 24.0f;
constexpr float kMinDelayMs = 1.0f;
constexpr float kMaxDelayMs = 2000.0f;
const juce::StringArray kParameterIDs = {
    "input_gain",
    "output_gain",
    "mix",
    "random_amount",
    "dist_enable",
    "dist_type",
    "dist_drive",
    "dist_mix",
    "dist_tone",
    "delay_enable",
    "delay_type",
    "delay_time",
    "delay_sync",
    "delay_feedback",
    "delay_mix",
    "delay_filter",
    "rev_enable",
    "rev_type",
    "rev_size",
    "rev_damp",
    "rev_mix",
    "mod_enable",
    "mod_type",
    "mod_rate",
    "mod_depth",
    "mod_mix",
    "mod_feedback",
    "lfo_sync",
    "lfo_rate",
    "lfo_depth",
    "lfo_dest"
};

juce::StringArray makeLfoDestinations()
{
    return {
        "None",
        "Dist Drive",
        "Delay Time",
        "Delay Feedback",
        "Reverb Size",
        "Reverb Mix",
        "Mod Depth"
    };
}
}

DiceFXAudioProcessor::DiceFXAudioProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createParameterLayout()),
      lockState ("LOCKS"),
      randomizer (apvts, lockState),
      presets (apvts)
{
    apvts.state.addChild (lockState, -1, nullptr);
    for (const auto& id : kParameterIDs)
    {
        if (! lockState.hasProperty (id))
        {
            const bool defaultLocked = (id == "input_gain" || id == "output_gain");
            lockState.setProperty (id, defaultLocked, nullptr);
        }
    }
}

const juce::String DiceFXAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DiceFXAudioProcessor::acceptsMidi() const { return false; }
bool DiceFXAudioProcessor::producesMidi() const { return false; }
bool DiceFXAudioProcessor::isMidiEffect() const { return false; }
double DiceFXAudioProcessor::getTailLengthSeconds() const { return 2.0; }

int DiceFXAudioProcessor::getNumPrograms() { return 1; }
int DiceFXAudioProcessor::getCurrentProgram() { return 0; }
void DiceFXAudioProcessor::setCurrentProgram (int) {}
const juce::String DiceFXAudioProcessor::getProgramName (int) { return {}; }
void DiceFXAudioProcessor::changeProgramName (int, const juce::String&) {}

void DiceFXAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32> (samplesPerBlock), 2 };

    distortion.prepare (sampleRate);
    modulation.prepare (sampleRate, samplesPerBlock);
    delay.prepare (sampleRate, samplesPerBlock);
    reverb.prepare (spec);
    lfo.prepare (sampleRate);

    dryBuffer.setSize (getTotalNumOutputChannels(), samplesPerBlock);
}

void DiceFXAudioProcessor::releaseResources() {}

bool DiceFXAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

float DiceFXAudioProcessor::applyLfoToRange (float baseValue, float minValue, float maxValue, float lfoValue) const
{
    auto range = maxValue - minValue;
    return juce::jlimit (minValue, maxValue, baseValue + (lfoValue * range));
}

double DiceFXAudioProcessor::getHostBpm() const
{
    if (auto* playHead = getPlayHead())
    {
        if (const auto pos = playHead->getPosition())
        {
            if (const auto bpm = pos->getBpm())
                return *bpm;
        }
    }

    return 120.0;
}

void DiceFXAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    const int numSamples = buffer.getNumSamples();

    const float inputGainDb = *apvts.getRawParameterValue ("input_gain");
    const float outputGainDb = *apvts.getRawParameterValue ("output_gain");
    const float globalMix = *apvts.getRawParameterValue ("mix");

    buffer.applyGain (juce::Decibels::decibelsToGain (inputGainDb));
    dryBuffer.makeCopyOf (buffer);

    const bool distEnable = apvts.getRawParameterValue ("dist_enable")->load() > 0.5f;
    const bool delayEnable = apvts.getRawParameterValue ("delay_enable")->load() > 0.5f;
    const bool revEnable = apvts.getRawParameterValue ("rev_enable")->load() > 0.5f;
    const bool modEnable = apvts.getRawParameterValue ("mod_enable")->load() > 0.5f;

    const int distType = static_cast<int> (apvts.getRawParameterValue ("dist_type")->load());
    float distDrive = *apvts.getRawParameterValue ("dist_drive");
    float distMix = *apvts.getRawParameterValue ("dist_mix");
    float distTone = *apvts.getRawParameterValue ("dist_tone");

    const int delayType = static_cast<int> (apvts.getRawParameterValue ("delay_type")->load());
    float delayTimeMs = *apvts.getRawParameterValue ("delay_time");
    const bool delaySync = apvts.getRawParameterValue ("delay_sync")->load() > 0.5f;
    float delayFeedback = *apvts.getRawParameterValue ("delay_feedback");
    float delayMix = *apvts.getRawParameterValue ("delay_mix");
    float delayFilter = *apvts.getRawParameterValue ("delay_filter");

    const int revType = static_cast<int> (apvts.getRawParameterValue ("rev_type")->load());
    float revSize = *apvts.getRawParameterValue ("rev_size");
    float revDamp = *apvts.getRawParameterValue ("rev_damp");
    float revMix = *apvts.getRawParameterValue ("rev_mix");

    const int modType = static_cast<int> (apvts.getRawParameterValue ("mod_type")->load());
    float modRate = *apvts.getRawParameterValue ("mod_rate");
    float modDepth = *apvts.getRawParameterValue ("mod_depth");
    float modMix = *apvts.getRawParameterValue ("mod_mix");
    float modFeedback = *apvts.getRawParameterValue ("mod_feedback");

    const bool lfoSync = apvts.getRawParameterValue ("lfo_sync")->load() > 0.5f;
    const float lfoRateValue = *apvts.getRawParameterValue ("lfo_rate");
    const float lfoDepth = *apvts.getRawParameterValue ("lfo_depth");
    const int lfoDest = static_cast<int> (apvts.getRawParameterValue ("lfo_dest")->load());

    const double bpm = getHostBpm();

    lfo.setSync (lfoSync);
    lfo.setRateValue (lfoRateValue);
    lfo.setTempo (bpm);

    const float lfoValue = lfo.getNextValue (numSamples) * lfoDepth;

    if (delaySync)
        delayTimeMs = LFO::tempoDivisionToMs (LFO::getDivisionIndexFromValue (delayTimeMs, kMinDelayMs, kMaxDelayMs), bpm);

    switch (lfoDest)
    {
        case 1: distDrive = applyLfoToRange (distDrive, 0.0f, 1.0f, lfoValue); break;
        case 2: delayTimeMs = applyLfoToRange (delayTimeMs, kMinDelayMs, kMaxDelayMs, lfoValue); break;
        case 3: delayFeedback = applyLfoToRange (delayFeedback, 0.0f, 0.95f, lfoValue); break;
        case 4: revSize = applyLfoToRange (revSize, 0.0f, 1.0f, lfoValue); break;
        case 5: revMix = applyLfoToRange (revMix, 0.0f, 1.0f, lfoValue); break;
        case 6: modDepth = applyLfoToRange (modDepth, 0.0f, 1.0f, lfoValue); break;
        default: break;
    }

    distortion.setParameters (distDrive, distMix, distTone, distType, distEnable);
    distortion.process (buffer);

    modulation.setParameters (modRate, modDepth, modMix, modFeedback, modType, modEnable);
    modulation.process (buffer);

    delay.setParameters (delayTimeMs, delayFeedback, delayMix, delayFilter, delayType, delayEnable);
    delay.process (buffer);

    reverb.setParameters (revSize, revDamp, revMix, revType, revEnable);
    reverb.process (buffer);

    const float dryGain = 1.0f - globalMix;
    for (int channel = 0; channel < totalNumOutputChannels; ++channel)
    {
        auto* wet = buffer.getWritePointer (channel);
        const auto* dry = dryBuffer.getReadPointer (channel);
        for (int i = 0; i < numSamples; ++i)
            wet[i] = wet[i] * globalMix + dry[i] * dryGain;
    }

    buffer.applyGain (juce::Decibels::decibelsToGain (outputGainDb));
}

bool DiceFXAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* DiceFXAudioProcessor::createEditor()
{
    return new DiceFXAudioProcessorEditor (*this);
}

void DiceFXAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    if (auto existing = state.getChildWithName ("LOCKS"); existing.isValid())
        state.removeChild (existing, nullptr);

    state.addChild (lockState.createCopy(), -1, nullptr);

    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void DiceFXAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));

    if (xml == nullptr)
        return;

    if (xml->hasTagName (apvts.state.getType()))
    {
        auto state = juce::ValueTree::fromXml (*xml);
        apvts.replaceState (state);
        lockState = apvts.state.getChildWithName ("LOCKS");
        if (! lockState.isValid())
        {
            lockState = juce::ValueTree ("LOCKS");
            apvts.state.addChild (lockState, -1, nullptr);
        }

        for (const auto& id : kParameterIDs)
        {
            if (! lockState.hasProperty (id))
            {
                const bool defaultLocked = (id == "input_gain" || id == "output_gain");
                lockState.setProperty (id, defaultLocked, nullptr);
            }
        }
    }
}

juce::Value DiceFXAudioProcessor::getLockValue (const juce::String& paramID)
{
    return lockState.getPropertyAsValue (paramID, nullptr);
}

bool DiceFXAudioProcessor::isLocked (const juce::String& paramID) const
{
    return lockState.getProperty (paramID, false);
}

void DiceFXAudioProcessor::setLocked (const juce::String& paramID, bool locked)
{
    lockState.setProperty (paramID, locked, nullptr);
}

void DiceFXAudioProcessor::randomizeParameters()
{
    const float amount = *apvts.getRawParameterValue ("random_amount");
    randomizer.randomize (amount);
}

juce::AudioProcessorValueTreeState::ParameterLayout DiceFXAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    auto boolParam = [] (const juce::String& id, const juce::String& name, bool def)
    {
        return std::make_unique<juce::AudioParameterBool> (id, name, def);
    };

    auto floatParam = [] (const juce::String& id, const juce::String& name, float min, float max, float def, float step = 0.0f)
    {
        juce::NormalisableRange<float> range (min, max, step);
        return std::make_unique<juce::AudioParameterFloat> (id, name, range, def);
    };

    params.push_back (floatParam ("input_gain", "Input", kMinGainDb, kMaxGainDb, 0.0f, 0.01f));
    params.push_back (floatParam ("output_gain", "Output", kMinGainDb, kMaxGainDb, 0.0f, 0.01f));
    params.push_back (floatParam ("mix", "Mix", 0.0f, 1.0f, 0.5f, 0.001f));
    params.push_back (floatParam ("random_amount", "Random Amount", 0.0f, 1.0f, 0.5f, 0.001f));

    params.push_back (boolParam ("dist_enable", "Dist On", true));
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("dist_type", "Dist Type",
        juce::StringArray { "Modern", "Vintage", "Hard" }, 0));
    params.push_back (floatParam ("dist_drive", "Drive", 0.0f, 1.0f, 0.3f, 0.001f));
    params.push_back (floatParam ("dist_mix", "Dist Mix", 0.0f, 1.0f, 0.4f, 0.001f));
    params.push_back (floatParam ("dist_tone", "Tone", 0.0f, 1.0f, 0.7f, 0.001f));

    params.push_back (boolParam ("delay_enable", "Delay On", true));
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("delay_type", "Delay Type",
        juce::StringArray { "Digital", "Tape", "PingPong" }, 0));
    params.push_back (floatParam ("delay_time", "Delay Time", kMinDelayMs, kMaxDelayMs, 380.0f, 0.01f));
    params.push_back (boolParam ("delay_sync", "Delay Sync", false));
    params.push_back (floatParam ("delay_feedback", "Feedback", 0.0f, 0.95f, 0.35f, 0.001f));
    params.push_back (floatParam ("delay_mix", "Delay Mix", 0.0f, 1.0f, 0.35f, 0.001f));
    params.push_back (floatParam ("delay_filter", "Delay Tone", 0.0f, 1.0f, 0.6f, 0.001f));

    params.push_back (boolParam ("rev_enable", "Reverb On", true));
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("rev_type", "Reverb Type",
        juce::StringArray { "Room", "Hall", "Plate" }, 0));
    params.push_back (floatParam ("rev_size", "Size", 0.0f, 1.0f, 0.5f, 0.001f));
    params.push_back (floatParam ("rev_damp", "Damp", 0.0f, 1.0f, 0.4f, 0.001f));
    params.push_back (floatParam ("rev_mix", "Reverb Mix", 0.0f, 1.0f, 0.4f, 0.001f));

    params.push_back (boolParam ("mod_enable", "Mod On", false));
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("mod_type", "Mod Type",
        juce::StringArray { "Chorus", "Flanger" }, 0));
    params.push_back (floatParam ("mod_rate", "Mod Rate", 0.0f, 1.0f, 0.35f, 0.001f));
    params.push_back (floatParam ("mod_depth", "Mod Depth", 0.0f, 1.0f, 0.4f, 0.001f));
    params.push_back (floatParam ("mod_mix", "Mod Mix", 0.0f, 1.0f, 0.3f, 0.001f));
    params.push_back (floatParam ("mod_feedback", "Mod Feedback", 0.0f, 0.95f, 0.2f, 0.001f));

    params.push_back (boolParam ("lfo_sync", "LFO Sync", true));
    params.push_back (floatParam ("lfo_rate", "LFO Rate", 0.0f, 1.0f, 0.5f, 0.001f));
    params.push_back (floatParam ("lfo_depth", "LFO Depth", 0.0f, 1.0f, 0.0f, 0.001f));
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("lfo_dest", "LFO Dest", makeLfoDestinations(), 0));

    return { params.begin(), params.end() };
}

const juce::StringArray& DiceFXAudioProcessor::getParameterIDs()
{
    return kParameterIDs;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DiceFXAudioProcessor();
}
