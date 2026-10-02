cmake_minimum_required(VERSION 3.22)

project(DiceFX VERSION 1.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Expect JUCE as a submodule in external/JUCE
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/JUCE/CMakeLists.txt")
    add_subdirectory(external/JUCE)
else()
    message(FATAL_ERROR "JUCE not found. Please add JUCE to external/JUCE (git submodule recommended).")
endif()

option(DICEFX_BUILD_PREVIEW "Build a standalone app for UI checks" OFF)
option(DICEFX_COPY_PLUGIN_AFTER_BUILD "Copy the built plug-in to the local plug-in folder" ON)
set(DICEFX_FORMATS VST3)
if (DICEFX_BUILD_PREVIEW)
    list(APPEND DICEFX_FORMATS Standalone)
endif()

juce_add_plugin(DiceFX
    COMPANY_NAME "BalorClub"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD ${DICEFX_COPY_PLUGIN_AFTER_BUILD}
    PLUGIN_MANUFACTURER_CODE BALO
    PLUGIN_CODE DICE
    FORMATS ${DICEFX_FORMATS}
    PRODUCT_NAME "DiceFX")

juce_add_binary_data(DiceFXResources SOURCES
    Resources/Presets.json
    Resources/Icons/icon_back.svg
    Resources/Icons/icon_forward.svg
    Resources/Icons/icon_down.svg
    Resources/Icons/icon_menu.svg
    Resources/Icons/icon_close.svg
    Resources/Icons/icon_lock.svg
    Resources/Icons/icon_unlock.svg
    Resources/Icons/icon_partial_lock.svg
    Resources/Icons/icon_check.svg
    Resources/Icons/icon_dice.svg
    Resources/Icons/icon_save.svg)

juce_generate_juce_header(DiceFX)

target_sources(DiceFX PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/UI/Theme.h
    Source/UI/Icons.h
    Source/UI/DiceLookAndFeel.cpp
    Source/UI/DiceLookAndFeel.h
    Source/UI/Components/AdvancedDrawer.cpp
    Source/UI/Components/AdvancedDrawer.h
    Source/UI/Components/EffectCard.cpp
    Source/UI/Components/EffectCard.h
    Source/UI/Components/HeaderBar.cpp
    Source/UI/Components/HeaderBar.h
    Source/UI/Components/LfoRoutingStrip.cpp
    Source/UI/Components/LfoRoutingStrip.h
    Source/UI/Components/ParameterControls.cpp
    Source/UI/Components/ParameterControls.h
    Source/UI/Components/ParameterKnob.cpp
    Source/UI/Components/ParameterKnob.h
    Source/UI/Components/RandomizeHero.cpp
    Source/UI/Components/RandomizeHero.h
    Source/UI/Components/UtilityBar.cpp
    Source/UI/Components/UtilityBar.h
    Source/UI/State/RandomHistory.cpp
    Source/UI/State/RandomHistory.h
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

option(DICEFX_BUILD_UI_CHECKS "Build UI geometry and interaction checks" OFF)
if (DICEFX_BUILD_UI_CHECKS)
    juce_add_console_app(DiceFXUIChecks PRODUCT_NAME "DiceFX UI Checks")
    target_sources(DiceFXUIChecks PRIVATE Tests/UIChecks.cpp)
    target_include_directories(DiceFXUIChecks PRIVATE Source "${CMAKE_CURRENT_BINARY_DIR}/DiceFX_artefacts/JuceLibraryCode")
    target_compile_definitions(DiceFXUIChecks PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_MODAL_LOOPS_PERMITTED=1)
    # Link the shared-code archive without inheriting plug-in-only wrapper macros.
    add_dependencies(DiceFXUIChecks DiceFX)
    target_link_libraries(DiceFXUIChecks PRIVATE "$<TARGET_FILE:DiceFX>" DiceFXResources juce::juce_audio_utils juce::juce_dsp juce::juce_gui_extra)
endif()

option(DICEFX_BUILD_DSP_CHECKS "Build offline DSP regression checks" OFF)
if (DICEFX_BUILD_DSP_CHECKS)
    juce_add_console_app(DiceFXDSPChecks PRODUCT_NAME "DiceFX DSP Checks")
    target_sources(DiceFXDSPChecks PRIVATE Tests/DSPChecks.cpp)
    target_include_directories(DiceFXDSPChecks PRIVATE Source "${CMAKE_CURRENT_BINARY_DIR}/DiceFX_artefacts/JuceLibraryCode")
    target_compile_definitions(DiceFXDSPChecks PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
    add_dependencies(DiceFXDSPChecks DiceFX)
    target_link_libraries(DiceFXDSPChecks PRIVATE "$<TARGET_FILE:DiceFX>" DiceFXResources juce::juce_audio_utils juce::juce_dsp juce::juce_gui_extra)
    if (APPLE)
        add_library(DiceFXAllocationAudit SHARED Tests/AllocationAudit.cpp)
        target_link_libraries(DiceFXDSPChecks PRIVATE DiceFXAllocationAudit)
    endif()
    enable_testing()
    add_test(NAME DiceFXDSPChecks COMMAND DiceFXDSPChecks)
endif()
