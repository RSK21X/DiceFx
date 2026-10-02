# Changelog / 更新记录

## 1.1.1 — 2026-10-02

### English

- Keep distortion and delay cutoffs below Nyquist and preserve filter history across blocks.
- Update existing filter coefficients without audio-thread allocation; reuse dry/reverb storage for variable and oversized blocks.
- Advance shared control ramps once per stereo frame and prepare reverb for the actual sample rate.
- Run LFO updates on a fixed 16-sample clock, independent of host block boundaries.
- Smooth input gain, output gain, global mix and delay-time changes.
- Prepare sample-rate-scaled delay memory for 24-second synchronized repeats at 10–1000 BPM.
- Clear all effect state on host reset and report conservative tails, including infinite regenerative Tape tails.
- Add offline DSP regression checks and macOS allocation auditing. UI and parameter IDs are unchanged.

### 中文

- 将失真和延迟的截止频率限制在安全范围，保留音频块之间的滤波历史。
- 就地更新滤波系数，复用干声和混响缓存，避免音频线程分配内存。
- 每个立体声采样帧只推进一次控制平滑值，并按实际采样率初始化混响。
- LFO 使用固定 16 采样时钟，避免依赖宿主音频块长度。
- 平滑输入、输出、总混合和延迟时间变化。
- 各采样率预分配 24 秒延迟，节拍同步支持 10–1000 BPM。
- 宿主重置时清除效果状态；尾音使用保守估计，可能持续自激的 Tape 反馈声明无限尾音。
- 增加离线 DSP 回归检查和 macOS 内存分配审计。界面和参数 ID 不变。

## 1.1.0 — 2026-09-30

### English

- Rebuilt the editor as a compact 2×2 effects panel with flat black-and-yellow styling.
- Added embedded SVG icons for randomization, save, navigation, menus and parameter locks.
- Unified knob geometry and proportionally scaled the interface to prevent overlaps.
- Kept presets, randomization history, A/B, module locks and individual parameter locks accessible.
- Removed decorative and unfinished controls that did not map to the processor.
- Updated synchronized delay and LFO value displays when sync mode changes.
- Added an optional standalone app and automated editor checks.
- Added illustrated English and Chinese documentation and a pinned JUCE 8.0.12 submodule.
- Published universal macOS VST3 and standalone archives targeting macOS 11+.
- Audio processor, DSP, LFO, randomizer and preset implementation files are unchanged from the preceding main branch.

### 中文

- 重构为紧凑的 2×2 效果器面板，采用黑黄扁平配色。
- 随机化、保存、导航、菜单和参数锁使用内嵌 SVG 图标。
- 统一旋钮几何尺寸，以等比例缩放保持控件不重叠。
- 保留预设、随机化历史、A/B、模块锁和单独参数锁。
- 移除未连接处理器的装饰和未完成功能。
- 同步模式切换时更新延迟与 LFO 的数值单位显示。
- 增加可选独立应用与自动界面检查。
- 更新带图的中英文说明，并固定 JUCE 8.0.12 子模块。
- 提供以 macOS 11+ 为构建目标的通用架构 VST3 和独立应用压缩包。
- 音频处理器、DSP、LFO、随机化器和预设实现文件与此前 main 分支保持一致。
