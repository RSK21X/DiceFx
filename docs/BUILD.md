# Build and test / 构建与测试

## Requirements

CMake 3.22+, C++17 and Xcode Command Line Tools on macOS. JUCE 8.0.12 is pinned in `external/JUCE`.

Clone with `--recurse-submodules`, or run `git submodule update --init --recursive` in an existing clone. The default build installs the VST3 into the local plug-in folder; the commands below disable that copy.

## Universal macOS build

```bash
cmake -S . -B build-release \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 \
  -DDICEFX_BUILD_PREVIEW=ON \
  -DDICEFX_BUILD_UI_CHECKS=ON \
  -DDICEFX_BUILD_DSP_CHECKS=ON \
  -DDICEFX_COPY_PLUGIN_AFTER_BUILD=OFF
cmake --build build-release --config Release \
  --target DiceFX_VST3 DiceFX_Standalone DiceFXDSPChecks DiceFXUIChecks -j 4
```

Bundles are in `build-release/DiceFX_artefacts/Release/VST3/` and `build-release/DiceFX_artefacts/Release/Standalone/`.

After the VST3 manifest has been generated, sign the finished bundles before packaging them:

```bash
codesign --force --deep --sign - build-release/DiceFX_artefacts/Release/VST3/DiceFX.vst3
codesign --force --deep --sign - build-release/DiceFX_artefacts/Release/Standalone/DiceFX.app
codesign --verify --deep --strict build-release/DiceFX_artefacts/Release/VST3/DiceFX.vst3
codesign --verify --deep --strict build-release/DiceFX_artefacts/Release/Standalone/DiceFX.app
```

This is ad-hoc signing, not Apple notarization. It does not bypass macOS download approval.

## Checks

```bash
ctest --test-dir build-release -C Release --output-on-failure
./build-release/DiceFXUIChecks_artefacts/Release/DiceFXUIChecks ./build-release/ui-screenshots
arch -x86_64 ./build-release/DiceFXDSPChecks_artefacts/Release/DiceFXDSPChecks
arch -x86_64 ./build-release/DiceFXUIChecks_artefacts/Release/DiceFXUIChecks ./build-release/ui-screenshots-intel
```

The Intel commands require Rosetta on Apple Silicon. DSP checks cover 22.05–192 kHz stability, filter history, stereo ramps, reverb and synced-delay timing, all six LFO destinations with different block sizes, smoothing, resets and tails. On macOS they also audit `malloc`, `calloc`, `realloc` and C++ allocation during processing; other platforms check C++ allocation only. The audit library is test-only and is not shipped with the plug-in.

UI checks cover four window sizes, SVG icons, circular knobs, control spacing, parameter bindings, presets, locks, randomization, history, A/B and sync displays.

Release checks run on macOS 27, natively on Apple Silicon and under Rosetta. Older macOS versions, a physical Intel Mac, Windows and Linux have not been runtime-tested.

## Behaviour notes

- The signal chain is Distortion → Modulation → Delay → Reverb. The global sine LFO targets one parameter at a time.
- Tempo sync supports 10–1000 BPM and up to 24 seconds of delay at each sample rate. Positive out-of-range tempos are limited to that range; missing or invalid tempo falls back to 120 BPM.
- Tail lengths are conservative. Long synchronized repeats can extend exports; regenerative Tape feedback reports an infinite tail. Use an explicit export range for intentionally sustained feedback.
- User presets are JSON files stored in `~/Library/DiceFX/Presets/` on macOS. Save, import and export are available from the editor.
- Parameter IDs are preserved, but the DSP fixes can change the sound of existing projects where the old behaviour was incorrect.

## 中文摘要

上面的通用架构命令同时生成 VST3、独立应用和检查程序，且不会自动安装插件。JUCE 子模块需完整下载；macOS 需要 Xcode Command Line Tools。

打包前，在 VST3 清单生成后对最终包重新签名并验证。该签名不是 Apple 公证，下载后的系统批准流程仍然适用。内存分配审计库仅用于测试，不会随插件发布。

检查覆盖 22.05–192 kHz 的音频稳定性、滤波连续性、左右声道变化、同步与混响时间、不同音频块长度下的 LFO、一致的控制平滑、重置、尾音，以及界面和交互。Intel 架构通过 Rosetta 检查；尚未对较旧的 macOS、实体 Intel Mac、Windows 或 Linux 进行运行验证。

同步范围为 10–1000 BPM，最长延迟 24 秒。同步长尾音可能延长导出；持续自激的 Tape 反馈声明无限尾音，建议手动指定导出区间。参数 ID 不变，但修复旧错误后，原工程中的声音可能有所变化。
