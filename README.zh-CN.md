# DiceFX

[English README](./README.md)

DiceFX 是一个以 macOS 为优先目标、基于 JUCE 开发的 `VST3` 音频插件。它的核心思路很直接：按下骰子按钮，随机分配未锁定参数，把原本普通的输入信号变成更有意外感和可玩性的效果链结果。

当前版本重点放在紧凑双态 UI、工厂与用户预设、参数锁、可跟随宿主速度的 LFO，以及适合声音实验的多效果器串联处理。

## 当前功能

- 面向 macOS 的 `VST3` 插件
- 基于 `JUCE + CMake` 的工程结构
- 多效果链路：`Distortion -> Modulation -> Delay -> Reverb`
- 效果类型：
  - Distortion：`Modern`、`Vintage`、`Hard`
  - Delay：`Digital`、`Tape`、`PingPong`
  - Reverb：`Room`、`Hall`、`Plate`
  - Modulation：`Chorus`、`Flanger`
- 全局控制：`Input`、`Output`、`Mix`、`Random Amount`
- 一个全局 `LFO`，支持自由速率和宿主节奏同步
- 当前 LFO 可调制目标：
  - `Dist Drive`
  - `Delay Time`
  - `Delay Feedback`
  - `Reverb Size`
  - `Reverb Mix`
  - `Mod Depth`
- 参数锁定功能，锁住后不会被 Dice 随机改动
- 来自 `Resources/Presets.json` 的工厂预设
- 支持用户预设保存、导入、导出，格式为 JSON
- 默认紧凑视图 + `advanced` 高级视图

## 构建

环境要求：

- macOS
- CMake `>= 3.22`
- Xcode Command Line Tools 或较新的 Clang 工具链
- `external/JUCE` 下存在 JUCE 源码

推荐用子模块方式克隆：

```bash
git clone --recurse-submodules <your-repo-url>
cd <repo-folder>
```

如果本地缺少 JUCE：

```bash
git submodule update --init --recursive
```

构建命令：

```bash
cmake -S . -B build
cmake --build build --config Release
```

[CMakeLists.txt](./CMakeLists.txt) 中启用了 `COPY_PLUGIN_AFTER_BUILD`，所以 Release 构建完成后会自动复制到 macOS 的 VST3 目录：

```text
~/Library/Audio/Plug-Ins/VST3/DiceFX.vst3
```

## 项目结构

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

## 预设

- 工厂预设内嵌于 [Resources/Presets.json](./Resources/Presets.json)
- 用户预设默认保存在：

```text
~/Library/Application Support/DiceFX/Presets
```

- 导入和导出使用纯 JSON，方便查看和版本管理

## 说明

- 当前仓库是 `macOS-first`
- `AU`、Windows 打包完善、更多调制路由和更多效果模型仍在后续计划中
- 目前 UI 和 DSP 仍在持续迭代，建议视为开发版而不是最终正式版

## 许可证

DiceFX 以 `GPL-3.0` 协议开源，详见 [LICENSE](./LICENSE)。

由于当前 JUCE 以 GPL 方式使用，如果后续要分发闭源衍生版本，需要切换到对应的 JUCE 商业授权路径。
