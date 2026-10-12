#pragma once
#include <JuceHeader.h>
#include <array>

// DreamVox factory presets v0.2.
// Only uses parameters actually implemented in the current DSP.
// Reverb, echo and stereo doubling are intentionally excluded until implemented.
namespace DreamVoxPresets
{
struct Preset
{
    const char* name;
    const char* category;
    float low, body, air, threshold, ratio, deess, warmth, output;
};

inline constexpr std::array<Preset, 18> factory {{
    { "Natural Clean", "Essentials", 0.0f, -0.5f, 0.5f, -14.0f, 1.8f, 22.0f, 5.0f, 0.0f },
    { "Smooth R&B", "Essentials", -1.0f, 1.0f, 1.5f, -20.0f, 2.7f, 32.0f, 18.0f, -1.0f },
    { "Intimate Lead", "Essentials", -2.0f, 1.0f, -0.5f, -22.0f, 2.2f, 26.0f, 12.0f, -1.0f },
    { "Modern Pop Lead", "Essentials", -1.0f, -0.5f, 2.0f, -22.0f, 3.2f, 42.0f, 9.0f, -1.5f },
    { "Soft Ballad", "Essentials", 0.0f, 0.5f, 0.0f, -17.0f, 2.0f, 25.0f, 9.0f, -0.5f },
    { "Bright Vocal", "Tone", -1.0f, -1.0f, 3.0f, -18.0f, 2.3f, 48.0f, 6.0f, -1.0f },
    { "Warm Velvet", "Tone", 0.5f, 1.5f, -1.5f, -19.0f, 2.5f, 28.0f, 30.0f, -1.5f },
    { "Airy R&B", "Tone", -1.5f, -0.5f, 2.5f, -19.0f, 2.3f, 48.0f, 10.0f, -1.5f },
    { "Dark Midnight", "Tone", 0.5f, 0.5f, -2.0f, -20.0f, 2.2f, 20.0f, 24.0f, -1.0f },
    { "Neo-Soul Warmth", "Tone", 0.5f, 1.0f, -1.0f, -17.0f, 2.2f, 27.0f, 32.0f, -1.0f },
    { "Tight Doubles", "Backing", -2.0f, -1.0f, 0.5f, -23.0f, 3.5f, 40.0f, 8.0f, -2.0f },
    { "Soft Harmonies", "Backing", -1.0f, -0.5f, -1.0f, -18.0f, 2.2f, 35.0f, 13.0f, -2.0f },
    { "BGV Presence", "Backing", -2.0f, 0.5f, 1.5f, -20.0f, 2.7f, 40.0f, 9.0f, -2.0f },
    { "Harsh Vocal Tamer", "Repair", -1.0f, -2.0f, -2.0f, -18.0f, 2.4f, 65.0f, 7.0f, -1.0f },
    { "Muddy Vocal Cleanup", "Repair", -4.0f, -1.0f, 1.5f, -19.0f, 2.5f, 37.0f, 7.0f, -1.0f },
    { "Sibilance Control", "Repair", -0.5f, 0.0f, -1.0f, -16.0f, 2.0f, 80.0f, 5.0f, -1.0f },
    { "Dynamic Vocal", "Repair", 0.0f, 0.0f, 0.0f, -23.0f, 3.5f, 30.0f, 10.0f, -1.0f },
    { "Gentle Levelling", "Repair", 0.0f, 0.0f, 0.0f, -24.0f, 2.0f, 28.0f, 5.0f, -1.0f }
}};

// Call from the UI/message thread, never from processBlock.
// APVTS parameter IDs match the current DreamVox processor.
inline void apply (juce::AudioProcessorValueTreeState& state, const Preset& preset)
{
    const std::pair<const char*, float> values[] = {
        {"low", preset.low}, {"body", preset.body}, {"air", preset.air},
        {"threshold", preset.threshold}, {"ratio", preset.ratio},
        {"deess", preset.deess}, {"warmth", preset.warmth},
        {"output", preset.output}
    };
    for (const auto& item : values)
        if (auto* param = state.getParameter (item.first))
            param->setValueNotifyingHost (param->convertTo0to1 (item.second));
    if (auto* bypass = state.getParameter ("bypass"))
        bypass->setValueNotifyingHost (0.0f);
}
} // namespace DreamVoxPresets
