#include "PluginProcessor.h"
#include <iostream>
#include <stdexcept>
#include <chrono>
#include <cstdlib>
#include <new>

namespace { thread_local bool trackNew = false; thread_local std::uint64_t newCount = 0; }
void* operator new (std::size_t size)
{
    if (trackNew) ++newCount;
    if (auto* p = std::malloc (size == 0 ? 1 : size)) return p;
    throw std::bad_alloc();
}
void* operator new[] (std::size_t n) { return ::operator new (n); }
void operator delete (void* p) noexcept { std::free (p); }
void operator delete[] (void* p) noexcept { std::free (p); }
void operator delete (void* p, std::size_t) noexcept { std::free (p); }
void operator delete[] (void* p, std::size_t) noexcept { std::free (p); }
#if JUCE_MAC
extern "C" void diceAuditBegin();
extern "C" std::uint64_t diceAuditEnd();
#endif

namespace
{
void require (bool ok, const char* message) { if (! ok) throw std::runtime_error (message); }
void fill (juce::AudioBuffer<float>& b, float v)
{
    for (int c = 0; c < 2; ++c) for (int i = 0; i < b.getNumSamples(); ++i) b.setSample (c, i, v);
}
void finite (const juce::AudioBuffer<float>& b)
{
    for (int c = 0; c < 2; ++c) for (int i = 0; i < b.getNumSamples(); ++i)
        require (std::isfinite (b.getSample (c, i)) && std::abs (b.getSample (c, i)) < 100.0f, "Unstable audio output");
}
void stereo (const juce::AudioBuffer<float>& b)
{
    for (int i = 0; i < b.getNumSamples(); ++i)
        require (std::abs (b.getSample (0, i) - b.getSample (1, i)) < 1.0e-6f, "Stereo control ramps differ");
}
void set (DiceFXAudioProcessor& p, const char* id, float v)
{
    auto* q = p.getAPVTS().getParameter (id);
    q->setValueNotifyingHost (q->convertTo0to1 (v));
}
void isolated (DiceFXAudioProcessor& p)
{
    for (auto id : { "dist_enable", "delay_enable", "rev_enable", "mod_enable" }) set (p, id, 0);
    set (p, "mix", 1); set (p, "lfo_depth", 0); set (p, "lfo_dest", 0);
}
void stabilityAndHistory()
{
    for (double sr : { 22050.0, 32000.0, 44100.0, 48000.0, 96000.0, 192000.0 })
        for (int type = 0; type < 3; ++type)
        {
            Distortion d; Delay e;
            d.prepare (sr); e.prepare (sr, 128);
            juce::AudioBuffer<float> x (2, 128), y (2, 128);
            for (int k = 0; k < 200; ++k)
            {
                d.setParameters (0.7f, 1, 1, type, true);
                e.setParameters (1, 0.95f, 1, 1, type, true);
                fill (x, 0.1f); fill (y, 0.1f); d.process (x); e.process (y);
                finite (x); finite (y); stereo (x); stereo (y);
            }
        }
    Distortion d; Delay e; d.prepare (48000); e.prepare (48000, 512);
    juce::AudioBuffer<float> x (2, 512), y (2, 512);
    for (int k = 0; k < 40; ++k)
    {
        d.setParameters (0.5f, 1, 0, 0, true); e.setParameters (1, 0, 1, 0, 0, true);
        fill (x, 0.1f); fill (y, 0.1f); d.process (x); e.process (y);
    }
    const float dx = x.getSample (0, 511), ey = y.getSample (0, 511);
    d.setParameters (0.5f, 1, 0, 0, true); e.setParameters (1, 0, 1, 0, 0, true);
    fill (x, 0.1f); fill (y, 0.1f); d.process (x); e.process (y);
    require (std::abs (x.getSample (0, 0) - dx) < 1.0e-5f, "Distortion loses filter history");
    require (std::abs (y.getSample (0, 0) - ey) < 1.0e-5f, "Delay loses filter history");
    for (int type = 0; type < 3; ++type)
    {
        d.setParameters (0.3f, 0.2f, 0.7f, type, true); fill (x, 0.1f); d.process (x); stereo (x);
        e.setParameters (20, 0.5f, 0.3f, 0.7f, type, true); fill (y, 0.1f); e.process (y); stereo (y);
    }
    DiceReverb r; r.prepare ({ 48000, 512, 2 }); r.setParameters (0.5f, 0.5f, 1, 0, true);
    fill (x, 0.1f); r.process (x); stereo (x); // Before any reflections: only the shared mix ramp is audible.
    std::cout << "PASS: stable filters at six sample rates, preserved history, aligned stereo ramps\n";
}
void timing()
{
    for (double sr : { 44100.0, 48000.0, 96000.0, 192000.0 })
    {
        DiceReverb r; r.prepare ({ sr, 64, 2 }); r.setParameters (0.5f, 0.5f, 1, 0, true);
        juce::AudioBuffer<float> b (2, 64);
        for (int k = 0; k < 200; ++k) { b.clear(); r.process (b); }
        int first = -1;
        for (int k = 0; k < 100; ++k)
        {
            b.clear(); if (k == 0) { b.setSample (0, 0, 1); b.setSample (1, 0, 1); }
            r.process (b);
            for (int i = 0; i < 64; ++i) if (first < 0 && std::abs (b.getSample (0, i)) > 1.0e-7f) first = k * 64 + i;
        }
        require (std::abs (first - static_cast<int> (sr * 1116 / 44100)) <= 1, "Reverb timing ignores sample rate");
    }
    for (double sr : { 48000.0, 96000.0, 192000.0 })
    {
        Delay d; d.prepare (sr, 256); d.setParameters (static_cast<float> (LFO::tempoDivisionToMs (0, 40)), 0, 1, 1, 0, true);
        juce::AudioBuffer<float> b (2, 256);
        for (int k = 0; k < 80; ++k) { b.clear(); d.process (b); }
        int first = -1;
        for (int k = 0; k < 5000 && first < 0; ++k)
        {
            b.clear(); if (k == 0) b.setSample (0, 0, 1); d.process (b);
            for (int i = 0; i < 256; ++i) if (first < 0 && std::abs (b.getSample (0, i)) > 1.0e-6f) first = k * 256 + i;
        }
        require (first == static_cast<int> (sr * 6), "Tempo-synced delay is truncated");
    }
    require (LFO::tempoDivisionToMs (0, 10) == Delay::maximumDelaySeconds * 1000, "Sync range exceeds prepared storage");
    class PlayHead final : public juce::AudioPlayHead
    {
        juce::Optional<PositionInfo> getPosition() const override
        {
            PositionInfo position; position.setBpm (40); return position;
        }
    } playHead;
    DiceFXAudioProcessor p; isolated (p); p.setPlayHead (&playHead);
    set (p, "delay_enable", 1); set (p, "delay_sync", 1); set (p, "delay_time", 1);
    set (p, "delay_mix", 1); set (p, "delay_feedback", 0); set (p, "lfo_dest", 2);
    p.prepareToPlay (96000, 256); juce::MidiBuffer midi; juce::AudioBuffer<float> b (2, 256);
    for (int k = 0; k < 80; ++k) { b.clear(); p.processBlock (b, midi); }
    int first = -1;
    for (int k = 0; k < 2500 && first < 0; ++k)
    {
        b.clear(); if (k == 0) b.setSample (0, 0, 1); p.processBlock (b, midi);
        for (int i = 0; i < 256; ++i) if (first < 0 && std::abs (b.getSample (0, i)) > 1.0e-6f) first = k * 256 + i;
    }
    require (first == 576000, "Processor truncates synced delay when Delay Time is the zero-depth LFO destination");
    std::cout << "PASS: sample-rate-correct reverb and six-second synced delays through 192 kHz\n";
}
std::vector<float> render (int destination, const std::vector<int>& sizes, bool lfo = true)
{
    DiceFXAudioProcessor p;
    set (p, "mod_enable", 1); set (p, "delay_time", 80); set (p, "delay_sync", 0);
    set (p, "lfo_sync", 0); set (p, "lfo_rate", 1); set (p, "lfo_depth", lfo ? 0.2f : 0.0f); set (p, "lfo_dest", static_cast<float> (destination));
    p.prepareToPlay (48000, 64); juce::MidiBuffer midi;
    std::vector<float> result; result.reserve (60000);
    int position = 0, block = 0;
    while (position < 30000)
    {
        const int count = juce::jmin (sizes[static_cast<size_t> (block++) % sizes.size()], 30000 - position);
        juce::AudioBuffer<float> b (2, count);
        for (int c = 0; c < 2; ++c) for (int i = 0; i < count; ++i)
            b.setSample (c, i, 0.1f * std::sin ((position + i) * juce::MathConstants<double>::twoPi * 187 / 48000));
        p.processBlock (b, midi); finite (b);
        for (int i = 0; i < count; ++i) for (int c = 0; c < 2; ++c) result.push_back (b.getSample (c, i));
        position += count;
    }
    return result;
}
void modulationAndBlocks()
{
    LFO l; l.prepare (48000); l.setSync (false); l.setRateValue (1);
    float peak = 0; for (int i = 0; i < 4800; ++i) peak = juce::jmax (peak, std::abs (l.getNextValue()));
    require (peak > 0.999f, "20 Hz LFO does not move");
    for (int dest = 1; dest <= 6; ++dest)
    {
        const auto reference = render (dest, { 64 });
        for (const auto& sizes : { std::vector<int> { 512 }, { 2400 }, { 1, 17, 333, 65, 2049 } })
        {
            const auto actual = render (dest, sizes);
            for (size_t i = 0; i < reference.size(); ++i)
                require (std::abs (reference[i] - actual[i]) < 1.0e-6f, "Audio depends on host block size");
        }
    }
    const auto moving = render (1, { 2400 }), still = render (1, { 2400 }, false);
    float difference = 0;
    for (size_t i = 0; i < moving.size(); ++i) difference = juce::jmax (difference, std::abs (moving[i] - still[i]));
    require (difference > 0.001f, "Modulation freezes with 2400-sample host blocks");
    std::cout << "PASS: all six LFO destinations match across small, large, and irregular blocks\n";
}
void smoothing()
{
    for (auto id : { "input_gain", "output_gain", "mix" })
    {
        DiceFXAudioProcessor p; isolated (p);
        const bool isMix = juce::String (id) == "mix";
        if (isMix) { set (p, "dist_enable", 1); set (p, "dist_mix", 1); set (p, "mix", 0); }
        p.prepareToPlay (48000, 512); juce::AudioBuffer<float> b (2, 512); juce::MidiBuffer midi;
        for (int k = 0; k < 40; ++k) { fill (b, 0.1f); p.processBlock (b, midi); }
        const float previous = b.getSample (0, 511);
        set (p, id, isMix ? 1.0f : 24.0f);
        fill (b, 0.1f); p.processBlock (b, midi); stereo (b);
        require (std::abs (b.getSample (0, 0) - previous) < 0.005f, "Master control jumps abruptly");
        for (int k = 0; k < 3; ++k) { fill (b, 0.1f); p.processBlock (b, midi); }
        if (! isMix) require (std::abs (b.getSample (0, 511) - 0.1f * juce::Decibels::decibelsToGain (24.0f)) < 1.0e-4f, "Gain ramp does not reach its target");
    }
    Delay a, b; a.prepare (48000, 480); b.prepare (48000, 480);
    a.setParameters (250, 0, 1, 1, 0, true); b.setParameters (250, 0, 1, 1, 0, true);
    juce::AudioBuffer<float> x (2, 480), y (2, 480);
    for (int k = 0; k <= 100; ++k)
    {
        if (k == 100) a.setParameters (260, 0, 1, 1, 0, true);
        for (int c = 0; c < 2; ++c) for (int i = 0; i < 480; ++i)
            x.setSample (c, i, 0.1f * std::sin ((k * 480 + i) * juce::MathConstants<double>::twoPi * 440 / 48000));
        y.makeCopyOf (x); a.process (x); b.process (y);
    }
    require (std::abs (x.getSample (0, 0) - y.getSample (0, 0)) < 0.001f, "Delay-time change jumps to a new read position");
    std::cout << "PASS: input/output/mix and delay-time changes are smoothed\n";
}
void resetsAndTails()
{
    DiceFXAudioProcessor p; isolated (p); set (p, "delay_enable", 1); set (p, "delay_time", 100);
    set (p, "delay_feedback", 0); set (p, "delay_mix", 1); p.prepareToPlay (48000, 64);
    juce::AudioBuffer<float> b (2, 64); juce::MidiBuffer midi;
    for (int k = 0; k < 200; ++k) { b.clear(); p.processBlock (b, midi); }
    b.clear(); b.setSample (0, 0, 1); p.processBlock (b, midi); p.reset();
    for (int k = 0; k < 2000; ++k) { b.clear(); p.processBlock (b, midi); require (b.getMagnitude (0, 0, 64) == 0, "Host reset leaves old echoes"); }
    set (p, "delay_time", 2000); set (p, "delay_feedback", 0.95f);
    require (p.getTailLengthSeconds() > 6.0, "Tail declaration truncates repeated echoes");
    set (p, "delay_type", Delay::Tape); require (std::isinf (p.getTailLengthSeconds()), "Regenerative Tape feedback needs an infinite tail");
    set (p, "delay_type", Delay::Digital); set (p, "delay_feedback", 0.1f); set (p, "delay_time", 100);
    p.prepareToPlay (48000, 64);
    for (int k = 0; k < 200; ++k) { b.clear(); p.processBlock (b, midi); }
    const double tail = p.getTailLengthSeconds(); float afterTail = 0;
    for (int k = 0; k < 2000; ++k)
    {
        b.clear(); if (k == 0) b.setSample (0, 0, 1); p.processBlock (b, midi);
        if (k * 64 > tail * 48000) afterTail = juce::jmax (afterTail, b.getMagnitude (0, 0, 64));
    }
    require (afterTail < 1.0e-6f, "Audio outlasts the declared delay tail");
    DiceReverb r; r.prepare ({ 48000, 64, 2 }); r.setParameters (1, 0, 1, 1, true);
    b.clear(); b.setSample (0, 0, 1); r.process (b); r.reset();
    for (int k = 0; k < 200; ++k) { b.clear(); r.process (b); require (b.getMagnitude (0, 0, 64) == 0, "Reverb reset leaves old reflections"); }
    DiceFXAudioProcessor all;
    set (all, "mod_enable", 1); set (all, "mod_type", Modulation::Flanger);
    set (all, "delay_type", Delay::Tape); set (all, "delay_time", 20); set (all, "delay_feedback", 0.95f);
    all.prepareToPlay (48000, 64);
    for (int k = 0; k < 200; ++k) { fill (b, 0.1f); all.processBlock (b, midi); }
    all.reset();
    for (int k = 0; k < 2000; ++k)
    {
        b.clear(); all.processBlock (b, midi);
        require (b.getMagnitude (0, 0, 64) == 0 && b.getMagnitude (1, 0, 64) == 0, "Host reset leaves combined effect state");
    }
    std::cout << "PASS: resets clear audio, decay bounds cover echoes, regenerative tails are reported honestly\n";
}
void allocationAndPerformance()
{
#if JUCE_MAC
    diceAuditBegin();
    auto* calibration = std::malloc (127);
    static_cast<volatile char*> (calibration)[0] = 1;
    const auto calibrated = diceAuditEnd(); std::free (calibration);
    require (calibrated > 0, "Allocation instrumentation is not active");
#endif
    DiceFXAudioProcessor p; set (p, "mod_enable", 1);
    juce::MidiBuffer midi;
    for (double sr : { 22050.0, 48000.0, 192000.0 })
    {
        p.prepareToPlay (sr, 64);
        for (int n : { 0, 1, 17, 64, 127, 512, 2048, 2400 })
        {
            juce::AudioBuffer<float> b (2, n); fill (b, 0.1f);
            set (p, "dist_tone", n % 2 ? 0.1f : 0.9f); set (p, "delay_filter", n % 2 ? 0.9f : 0.1f);
            set (p, "dist_type", static_cast<float> (n % 3)); set (p, "delay_type", static_cast<float> (n % 3));
            newCount = 0; trackNew = true;
#if JUCE_MAC
            diceAuditBegin();
#endif
            p.processBlock (b, midi);
#if JUCE_MAC
            const auto cCount = diceAuditEnd();
#endif
            trackNew = false;
            require (newCount == 0, "Audio processing allocates C++ objects");
#if JUCE_MAC
            require (cCount == 0, "Audio processing calls malloc/calloc/realloc");
#endif
        }
    }
    p.prepareToPlay (48000, 64);
    juce::AudioBuffer<float> b (2, 512);
    const auto start = std::chrono::steady_clock::now();
    for (int k = 0; k < 1000; ++k) { fill (b, 0.1f); p.processBlock (b, midi); }
    const double elapsed = std::chrono::duration<double> (std::chrono::steady_clock::now() - start).count();
    std::cout << "PASS: allocation-free processing including first/zero/oversized blocks; 10.67 s of audio processed in " << elapsed << " s\n";
}
}
int main()
{
    juce::ScopedJuceInitialiser_GUI init;
    try
    {
        stabilityAndHistory(); timing(); modulationAndBlocks(); smoothing(); resetsAndTails(); allocationAndPerformance();
        std::cout << "PASS: all DSP regression checks\n"; return 0;
    }
    catch (const std::exception& e) { trackNew = false; std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
}
