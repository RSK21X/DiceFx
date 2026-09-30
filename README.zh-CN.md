# DiceFX

**掷出新声音，保留喜欢的部分。**

[English](README.md) · [下载插件](https://github.com/RSK21X/DiceFx/releases/latest) · [更新记录](CHANGELOG.md)

DiceFX 是一个用于声音实验的立体声 VST3 多效果器插件。将失真、调制、延迟和混响组合起来，随机改变未锁定的参数，在保留关键设置的同时探索新的声音。

![DiceFX 1.1.0：紧凑黑黄扁平界面](docs/images/dicefx-main.png)

## 1.1.0 有什么新变化

界面更小、更清楚：黑黄扁平配色、可缩放 SVG 图标、整洁的 2×2 效果器面板。默认窗口为 **800 × 500**，支持在 **720 × 450** 到 **1120 × 700** 之间缩放，旋钮保持正圆，控件间距不随缩放错乱。

本次更新修改界面与文档，**不修改音频处理代码**。

## 下载与安装

前往 [GitHub Releases](https://github.com/RSK21X/DiceFx/releases/latest) 下载最新版本。

| 下载文件 | 用途 |
| --- | --- |
| `DiceFX-1.1.0-macOS-universal-VST3.zip` | 在 DAW 中使用的 VST3 插件 |
| `DiceFX-1.1.0-macOS-universal-Standalone.zip` | 无需 DAW 即可体验界面和效果的独立应用 |
| `SHA256SUMS.txt` | 发布压缩包的校验值 |

发布包包含 Apple Silicon（`arm64`）和 Intel（`x86_64`）两种架构，构建目标为 macOS 11 及以上版本。插件需要支持 VST3 的宿主，本次不提供 AU、AAX 或 Windows 安装包。

发布检查已在 macOS 27 上通过 Apple Silicon 原生运行及 Rosetta 下的 Intel 架构运行。尚未对较旧的 macOS 版本及实体 Intel Mac 进行运行验证。

1. 退出 DAW，解压 VST3 压缩包。
2. 将 `DiceFX.vst3` 放入 `~/Library/Audio/Plug-Ins/VST3/`。
3. 重新打开 DAW，必要时重新扫描插件，在立体声音轨上加载 DiceFX。

独立应用无需 DAW。为避免反馈，它启动时默认将音频输入静音；开启输入前，请先检查音频设置。没有宿主速度信息时，同步控制使用处理器内置的 120 BPM 作为参考。

发布包采用 ad-hoc 签名，**未经过 Apple 公证**。macOS 可能要求你批准打开下载的应用。仅在信任下载来源时使用针对该应用的批准流程，无需关闭系统整体安全保护。

## 快速上手

1. 选择 **Init Clean** 或其他预设，点击各模块的状态灯启用需要的效果。
2. 调整 **Amount**，决定随机变化的幅度。
3. 点击模块的**锁**保护整个模块，或打开 **Locks** 单独锁定参数。
4. 点击 **RANDOMIZE**。通过前后箭头撤销、重做随机结果，或用 **A / B** 比较两个声音。
5. 调整全局 **MIX**，将喜欢的结果保存为预设。

拖动旋钮调整参数，双击恢复默认值。点击状态灯开启或关闭模块。参数锁只阻止随机化修改，不影响手动编辑。

## 效果器与调制

音频处理顺序为 **失真 → 调制 → 延迟 → 混响**，另有全局输入增益、干湿混合和输出增益。

| 模块 | 类型 | 控制 |
| --- | --- | --- |
| Distortion / 失真 | Modern · Vintage · Hard | Drive · Tone · Mix |
| Delay / 延迟 | Digital · Tape · PingPong | Time · Sync · Feedback · Tone · Mix |
| Reverb / 混响 | Room · Hall · Plate | Size · Damp · Mix |
| Modulation / 调制 | Chorus · Flanger | Depth · Rate · Feedback · Mix |

全局正弦 LFO 支持自由速率、节奏同步和深度调整，**每次只能选择一个调制目标**：None、Dist Drive、Delay Time、Delay Feedback、Reverb Size、Reverb Mix 或 Mod Depth。当前处理器不支持多个同时生效的 LFO 目标，也不支持切换波形。

### 参数锁

![DiceFX 单独参数锁面板](docs/images/dicefx-locks.png)

模块锁可以保护整个效果器。**Locks** 抽屉可分别锁定效果参数及全局输入、输出、混合参数；模块只锁定部分参数时会显示单独的状态图标。

### 预设

内置四个工厂预设：**Init Clean**、**Echo Mist**、**Grain Smash**、**Wide Wash**。

通过 **Save** 保存用户预设，**…** 菜单提供保存、导入和导出功能。预设使用 JSON 格式。本版本在 macOS 中的用户预设目录为：

```text
~/Library/DiceFX/Presets/
```

本次界面更新保留既有参数 ID 和音频处理实现。

## 从源码构建

需要 CMake 3.22 及以上版本、支持 C++17 的编译器；macOS 上还需要 Xcode Command Line Tools。JUCE 8.0.12 已作为 Git 子模块固定版本。

```bash
git clone --recurse-submodules https://github.com/RSK21X/DiceFx.git
cd DiceFx
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target DiceFX_VST3 -j 4
```

如果克隆时没有获取 JUCE，执行 `git submodule update --init --recursive`。

默认构建完成后会将 VST3 复制到当前用户的插件目录。添加 `-DDICEFX_COPY_PLUGIN_AFTER_BUILD=OFF` 可以只构建、不安装。

### macOS 通用架构发布包

```bash
cmake -S . -B build-release \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 \
  -DDICEFX_BUILD_PREVIEW=ON \
  -DDICEFX_COPY_PLUGIN_AFTER_BUILD=OFF
cmake --build build-release --config Release \
  --target DiceFX_VST3 DiceFX_Standalone -j 4
```

生成文件位于 `build-release/DiceFX_artefacts/Release/VST3/` 和 `build-release/DiceFX_artefacts/Release/Standalone/`。

### 界面与交互检查

```bash
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DDICEFX_BUILD_UI_CHECKS=ON \
  -DDICEFX_COPY_PLUGIN_AFTER_BUILD=OFF
cmake --build build --config Release --target DiceFXUIChecks -j 4
./build/DiceFXUIChecks_artefacts/Release/DiceFXUIChecks ./build/ui-screenshots
```

检查程序会在四种尺寸下渲染真实插件界面，并测试 SVG 图标、旋钮圆形、控件无重叠、参数绑定、工厂预设、参数锁、随机化、撤销与重做、A/B 和同步数值显示。还会在 44.1/48/96 kHz、64/512 采样块下检查所有工厂预设的音频输出是否为有限值。

## 项目结构

- `Source/UI/`：主题、控件、界面布局和随机化历史。
- `Source/Dsp/`、`Source/Mod/`、`Source/Random/`：音频效果、LFO 和随机化器。
- `Source/Presets/` 与 `Resources/Presets.json`：预设管理和工厂预设。
- `Resources/Icons/`：内嵌 SVG 图标。
- `Tests/UIChecks.cpp`：界面检查与截图。
- `external/JUCE/`：固定版本的框架子模块。

## 许可证

项目代码使用 [GPL-3.0](LICENSE)。JUCE 有其独立的授权条款，请查看 JUCE 子模块内的许可证。
