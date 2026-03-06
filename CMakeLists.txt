cmake_minimum_required(VERSION 3.22)

project(DiceFX VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Expect JUCE as a submodule in external/JUCE
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/JUCE/CMakeLists.txt")
    add_subdirectory(external/JUCE)
else()
    message(FATAL_ERROR "JUCE not found. Please add JUCE to external/JUCE (git submodule recommended).")
endif()

juce_add_plugin(DiceFX
    COMPANY_NAME "BalorClub"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE BALO
    PLUGIN_CODE DICE
    FORMATS VST3
    PRODUCT_NAME "DiceFX")

juce_add_binary_data(DiceFXResources SOURCES
    Resources/Presets.json)

juce_generate_juce_header(DiceFX)

target_sources(DiceFX PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/Dsp/Distortion.h
    Source/Dsp/Delay.h
    Source/Dsp/Reverb.h
    Source/Dsp/Modulation.h
    Source/Mod/LFO.h
    Source/Random/Randomizer.h
    Source/Presets/Presets.h)

target_compile_definitions(DiceFX PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(DiceFX PRIVATE
    DiceFXResources
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_extra)
