# DiceFX

**Roll a new sound. Keep the parts you like.**

[中文](README.zh-CN.md) · [Download v1.1.1](https://github.com/RSK21X/DiceFx/releases/tag/v1.1.1) · [Changelog](CHANGELOG.md)

A compact stereo multi-effects plug-in: distortion, modulation, delay and reverb, with one-click randomization.

https://github.com/user-attachments/assets/d30079da-32c5-4a76-9c59-fde0a0ef6355



## Features

- Four effects, a tempo-syncable sine LFO, and global input / mix / output.
- Randomization with module and individual parameter locks.
- Presets, undo/redo and A/B comparison.
- Flat black-and-yellow UI, SVG icons and a resizable 800 × 500 window.

**v1.1.1** fixes DSP stability, timing, stereo consistency and control smoothing. The interface and parameter IDs are unchanged.

## Install

macOS 11+ build target, Apple Silicon + Intel. Checks passed on Apple Silicon and under Rosetta.

- [VST3 plug-in](https://github.com/RSK21X/DiceFx/releases/download/v1.1.1/DiceFX-1.1.1-macOS-universal-VST3.zip): quit your DAW, extract and copy `DiceFX.vst3` to `~/Library/Audio/Plug-Ins/VST3/`, then reopen and rescan.
- [Standalone app](https://github.com/RSK21X/DiceFx/releases/download/v1.1.1/DiceFX-1.1.1-macOS-universal-Standalone.zip): extract and open `DiceFX.app`. Input starts muted to prevent feedback.

The bundles are ad-hoc signed, not Apple-notarized. macOS may require approval to open a trusted download. AU, AAX and Windows downloads are not included.

## Quick start

Choose a preset → lock what you like → set **Amount** → **RANDOMIZE** → adjust **MIX** → save. Use the arrows to undo/redo and **A/B** to compare. Double-click a knob to reset it.

## Build

Requires CMake 3.22+, C++17 and Xcode Command Line Tools on macOS. JUCE is included as a pinned submodule.

```bash
git clone --recurse-submodules https://github.com/RSK21X/DiceFx.git
cd DiceFx
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DDICEFX_COPY_PLUGIN_AFTER_BUILD=OFF
cmake --build build --target DiceFX_VST3 -j 4
```

[Build and test details](docs/BUILD.md) · [GPL-3.0](LICENSE)
