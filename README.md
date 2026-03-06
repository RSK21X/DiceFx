# DiceFX

[中文说明](./README.zh-CN.md)

<img width="840" height="550" alt="ScreenShot_2026-03-06_163311_059" src="https://github.com/user-attachments/assets/3f71af90-1699-4690-8894-1b226cea1793" />

DiceFX is a macOS-first JUCE VST3 plugin built around one idea: press the dice, randomize unlocked parameters, and turn a clean input into a playable accident.

The current build focuses on a compact two-state UI, factory and user presets, parameter locks, tempo-synced LFO modulation, and a modular multi-FX chain for sound design experiments.

## Current Feature Set

- `VST3` plugin target on macOS
- `JUCE + CMake` project layout
- Multi-FX chain: `Distortion -> Modulation -> Delay -> Reverb`
- Effect types:
  - Distortion: `Modern`, `Vintage`, `Hard`
  - Delay: `Digital`, `Tape`, `PingPong`
  - Reverb: `Room`, `Hall`, `Plate`
  - Modulation: `Chorus`, `Flanger`
- Global controls: `Input`, `Output`, `Mix`, `Random Amount`
- One global `LFO` with free rate / tempo sync
- Current LFO destinations:
  - `Dist Drive`
  - `Delay Time`
  - `Delay Feedback`
  - `Reverb Size`
  - `Reverb Mix`
  - `Mod Depth`
- Parameter locks that protect values from dice randomization
- Factory presets from `Resources/Presets.json`
- User preset save/import/export in JSON format
- Compact view plus `advanced` view for per-module controls

## Build

Requirements:

- macOS
- CMake `>= 3.22`
- Xcode Command Line Tools or a recent Clang toolchain
- JUCE checked out at `external/JUCE`

Clone with submodules:

```bash
git clone --recurse-submodules <your-repo-url>
cd <repo-folder>
```

If JUCE is missing:

```bash
git submodule update --init --recursive
```

Build:

```bash
cmake -S . -B build
cmake --build build --config Release
```

With `COPY_PLUGIN_AFTER_BUILD` enabled in [CMakeLists.txt](./CMakeLists.txt), the Release build is copied to the macOS VST3 location:

```text
~/Library/Audio/Plug-Ins/VST3/DiceFX.vst3
```

## Project Layout

```text
.
├── CMakeLists.txt
├── Resources/
│   └── Presets.json
├── Source/
│   ├── Dsp/
│   ├── Mod/
│   ├── Presets/
│   ├── Random/
│   ├── PluginEditor.cpp
│   ├── PluginEditor.h
│   ├── PluginProcessor.cpp
│   └── PluginProcessor.h
└── external/
    └── JUCE/
```

## Presets

- Factory presets are embedded from [Resources/Presets.json](./Resources/Presets.json).
- User presets are stored under:

```text
~/Library/Application Support/DiceFX/Presets
```

- Import/export uses plain JSON so preset files are easy to inspect and version.

## Notes

- This repository is currently `macOS-first`.
- `AU`, Windows packaging polish, extra modulation routing, and more effect models are still future work.
- The UI and DSP are under active iteration and should be treated as a development build rather than a final release.

## License

DiceFX is released under `GPL-3.0`. See [LICENSE](./LICENSE).

Because JUCE is used under the GPL workflow here, distributing derivative closed-source builds would require a different JUCE licensing path.
