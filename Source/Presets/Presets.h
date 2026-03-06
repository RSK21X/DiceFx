#pragma once

#include <JuceHeader.h>

struct Preset
{
    juce::String name;
    juce::NamedValueSet params;
};

class Presets
{
public:
    explicit Presets (juce::AudioProcessorValueTreeState& state)
        : apvts (state)
    {
        loadFactoryPresets();
        reloadUserPresets();
    }

    const std::vector<Preset>& getFactoryPresets() const { return factoryPresets; }
    const std::vector<Preset>& getUserPresets() const { return userPresets; }

    bool applyFactoryPreset (int index)
    {
        if (index < 0 || index >= static_cast<int> (factoryPresets.size()))
            return false;

        applyPreset (factoryPresets[static_cast<size_t> (index)]);
        return true;
    }

    bool applyUserPreset (int index)
    {
        if (index < 0 || index >= static_cast<int> (userPresets.size()))
            return false;

        applyPreset (userPresets[static_cast<size_t> (index)]);
        return true;
    }

    bool importPresetFromFile (const juce::File& file)
    {
        if (! file.existsAsFile())
            return false;

        const auto content = file.loadFileAsString();
        const auto varData = juce::JSON::parse (content);
        if (varData.isVoid())
            return false;

        if (varData.isArray())
        {
            const auto* array = varData.getArray();
            if (array == nullptr || array->isEmpty())
                return false;
            auto preset = presetFromVar ((*array)[0], file.getFileNameWithoutExtension());
            applyPreset (preset);
            return true;
        }

        if (varData.isObject())
        {
            auto preset = presetFromVar (varData, file.getFileNameWithoutExtension());
            applyPreset (preset);
            return true;
        }

        return false;
    }

    void reloadUserPresets()
    {
        userPresets.clear();

        auto dir = ensureUserPresetDirExists();

        juce::Array<juce::File> files;
        dir.findChildFiles (files, juce::File::findFiles, false, "*.json");
        files.sort();

        for (const auto& f : files)
        {
            const auto content = f.loadFileAsString();
            const auto varData = juce::JSON::parse (content);
            if (varData.isVoid())
                continue;

            auto preset = presetFromVar (varData, f.getFileNameWithoutExtension());
            if (preset.name.isNotEmpty())
                userPresets.push_back (std::move (preset));
        }
    }

    bool saveUserPreset (juce::String name)
    {
        name = name.trim();
        if (name.isEmpty())
            return false;

        auto dir = ensureUserPresetDirExists();

        auto safe = juce::File::createLegalFileName (name);
        if (safe.isEmpty())
            safe = "Preset";

        juce::File file = dir.getChildFile (safe).withFileExtension ("json");
        for (int attempt = 1; file.existsAsFile() && attempt < 100; ++attempt)
            file = dir.getChildFile (safe + " " + juce::String (attempt)).withFileExtension ("json");

        if (! exportPresetToFile (file))
            return false;

        reloadUserPresets();
        return true;
    }

    bool exportPresetToFile (const juce::File& file)
    {
        juce::DynamicObject::Ptr obj = new juce::DynamicObject();
        obj->setProperty ("name", file.getFileNameWithoutExtension());

        juce::DynamicObject::Ptr paramsObj = new juce::DynamicObject();
        const auto& params = apvts.processor.getParameters();
        for (int i = 0; i < params.size(); ++i)
        {
            auto* param = params.getUnchecked (i);
            if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*> (param))
            {
                const float value = ranged->convertFrom0to1 (param->getValue());
                paramsObj->setProperty (ranged->getParameterID(), value);
            }
        }

        obj->setProperty ("params", juce::var (paramsObj.get()));

        const juce::String json = juce::JSON::toString (juce::var (obj.get()), true);
        return file.replaceWithText (json);
    }

private:
    void loadFactoryPresets()
    {
        const juce::String jsonData (BinaryData::Presets_json, BinaryData::Presets_jsonSize);
        const auto varData = juce::JSON::parse (jsonData);

        if (! varData.isArray())
            return;

        const auto* array = varData.getArray();
        if (array == nullptr)
            return;

        factoryPresets.clear();
        for (const auto& entry : *array)
            factoryPresets.push_back (presetFromVar (entry, "Preset"));
    }

    Preset presetFromVar (const juce::var& data, const juce::String& nameFallback) const
    {
        Preset preset;
        if (! data.isObject())
        {
            preset.name = nameFallback;
            return preset;
        }

        const auto* obj = data.getDynamicObject();
        preset.name = obj->getProperty ("name").toString();
        if (preset.name.isEmpty())
            preset.name = nameFallback;

        const auto paramsVar = obj->getProperty ("params");
        if (paramsVar.isObject())
        {
            const auto* paramsObj = paramsVar.getDynamicObject();
            for (const auto& prop : paramsObj->getProperties())
                preset.params.set (prop.name, prop.value);
        }

        return preset;
    }

    void applyPreset (const Preset& preset)
    {
        for (const auto& prop : preset.params)
        {
            auto* param = apvts.getParameter (prop.name.toString());
            auto* ranged = dynamic_cast<juce::RangedAudioParameter*> (param);
            if (ranged == nullptr)
                continue;

            const float value = static_cast<float> (prop.value);
            const float normalized = ranged->convertTo0to1 (value);

            param->beginChangeGesture();
            param->setValueNotifyingHost (normalized);
            param->endChangeGesture();
        }
    }

    static juce::File getUserPresetDir()
    {
        auto base = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
            .getChildFile ("DiceFX")
            .getChildFile ("Presets");
        return base;
    }

    static juce::File ensureUserPresetDirExists()
    {
        auto dir = getUserPresetDir();
        auto parent = dir.getParentDirectory();
        if (! parent.exists())
            parent.createDirectory();
        if (! dir.exists())
            dir.createDirectory();
        return dir;
    }

    juce::AudioProcessorValueTreeState& apvts;
    std::vector<Preset> factoryPresets;
    std::vector<Preset> userPresets;
};
