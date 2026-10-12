#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <utility>
#include <array>

// DreamVox factory presets v0.2.
// Uses the current DSP controls, including spatial effects.
namespace DreamVoxPresets
{
struct Preset
{
    const char* name;
    const char* category;
    float low, body, air, threshold, ratio, deess, warmth, output;
    float space = 0, room = 45, echo = 0, echoTime = 330, feedback = 24, doubler = 0;
};

inline constexpr std::array<Preset, 24> factory {{
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
    { "Gentle Levelling", "Repair", 0.0f, 0.0f, 0.0f, -24.0f, 2.0f, 28.0f, 5.0f, -1.0f },
    { "Midnight Dream", "Space", -1.0f, 0.5f, 0.5f, -20.0f, 2.3f, 32.0f, 15.0f, -1.5f, 28, 72, 10, 390, 23, 8 },
    { "Hypnotic Haze", "Space", -1.0f, 0.0f, -1.5f, -18.0f, 2.0f, 25.0f, 22.0f, -2.0f, 34, 80, 21, 440, 37, 14 },
    { "Ethereal Air", "Space", -1.5f, -0.5f, 2.0f, -19.0f, 2.2f, 45.0f, 8.0f, -2.0f, 39, 83, 13, 510, 28, 12 },
    { "Y2K Velvet", "Space", -0.5f, 1.0f, 0.5f, -20.0f, 2.8f, 32.0f, 22.0f, -1.5f, 12, 48, 6, 270, 18, 8 },
    { "Floating Bridge", "Creative", -1.0f, 0.0f, 1.0f, -19.0f, 2.1f, 38.0f, 12.0f, -3.0f, 48, 90, 27, 490, 44, 16 },
    { "Dream Echo", "Creative", -1.5f, 0.0f, 0.5f, -19.0f, 2.4f, 30.0f, 12.0f, -2.5f, 18, 55, 35, 350, 43, 9 }
}};

// Call from the UI/message thread, never from processBlock.
// APVTS parameter IDs match the current DreamVox processor.
inline void apply (juce::AudioProcessorValueTreeState& state, const Preset& preset)
{
    const std::pair<const char*, float> values[] = {
        {"low", preset.low}, {"body", preset.body}, {"air", preset.air},
        {"threshold", preset.threshold}, {"ratio", preset.ratio},
        {"deess", preset.deess}, {"warmth", preset.warmth},
        {"output", preset.output}, {"space", preset.space}, {"room", preset.room},
        {"echo", preset.echo}, {"echoTime", preset.echoTime},
        {"feedback", preset.feedback}, {"doubler", preset.doubler},
        // A full-chain preset must reset newer module parameters too.
        {"highpass", 25.0f}, {"gateThreshold", -85.0f},
        {"compAttack", 12.0f}, {"compRelease", 120.0f},
        {"deessFreq", 6200.0f}, {"doublerSpread", 18.0f}
    };
    for (const auto& item : values)
        if (auto* param = state.getParameter (item.first))
            param->setValueNotifyingHost (param->convertTo0to1 (item.second));
    if (auto* bypass = state.getParameter ("bypass"))
        bypass->setValueNotifyingHost (0.0f);
}

// Effect-specific presets. These affect ONLY the listed parameters.
// Other modules, chain settings, and bypass are preserved.
struct EffectPreset
{
    const char* name;
    std::array<float, 4> values;
};
struct EffectBank
{
    const char* name;
    const char* description;
    std::array<const char*, 4> parameterIds;
    int parameterCount;
    std::array<EffectPreset, 4> presets;
};
inline constexpr std::array<EffectBank, 8> effectBanks {{
    { "CLEAN", "Low-cut and gentle noise-floor control", {"highpass", "gateThreshold", nullptr, nullptr}, 2,
      {{ {"Natural", {25.0f, -85.0f, 0, 0}}, {"Lead Cleanup", {85.0f, -80.0f, 0, 0}},
         {"Room Noise Tamer", {60.0f, -55.0f, 0, 0}}, {"Backing Vocal Cut", {125.0f, -75.0f, 0, 0}} }} },
    { "COMP", "Shape vocal dynamics without losing feeling", {"threshold", "ratio", "compAttack", "compRelease"}, 4,
      {{ {"Transparent", {-14.0f, 1.8f, 25.0f, 180.0f}}, {"Velvet Levelling", {-22.0f, 2.8f, 15.0f, 140.0f}},
         {"Tight Pop", {-25.0f, 4.0f, 7.0f, 90.0f}}, {"Slow Ballad", {-19.0f, 2.2f, 35.0f, 240.0f}} }} },
    { "EQ", "Three-band musical vocal tone", {"low", "body", "air", nullptr}, 3,
      {{ {"Flat", {0.0f, 0.0f, 0.0f, 0}}, {"Warm Body", {0.5f, 2.0f, -1.0f, 0}},
         {"Airy R&B", {-1.0f, -0.5f, 2.5f, 0}}, {"Muddy Vocal Fix", {-3.0f, -2.0f, 1.0f, 0}} }} },
    { "DE-ESS", "Tame high-frequency sibilance", {"deess", "deessFreq", nullptr, nullptr}, 2,
      {{ {"Soft Control", {20.0f, 7000.0f, 0, 0}}, {"Natural", {35.0f, 6200.0f, 0, 0}},
         {"Strong Sibilance", {65.0f, 5700.0f, 0, 0}}, {"Bright Voice", {52.0f, 8500.0f, 0, 0}} }} },
    { "TONE", "Add character through soft saturation", {"warmth", nullptr, nullptr, nullptr}, 1,
      {{ {"Clean", {0.0f, 0, 0, 0}}, {"Satin", {12.0f, 0, 0, 0}},
         {"Tape-ish Warmth", {30.0f, 0, 0, 0}}, {"Vintage Edge", {55.0f, 0, 0, 0}} }} },
    { "SPACE", "Choose vocal ambience and depth", {"space", "room", nullptr, nullptr}, 2,
      {{ {"Dry", {0.0f, 45.0f, 0, 0}}, {"Intimate Room", {12.0f, 30.0f, 0, 0}},
         {"Velvet Hall", {25.0f, 65.0f, 0, 0}}, {"Dreamy Wide", {40.0f, 85.0f, 0, 0}} }} },
    { "ECHO", "Delays for depth and hypnotic repeats", {"echo", "echoTime", "feedback", nullptr}, 3,
      {{ {"Off", {0.0f, 330.0f, 24.0f, 0}}, {"Short Slap", {17.0f, 110.0f, 12.0f, 0}},
         {"Soft Eighth", {20.0f, 270.0f, 25.0f, 0}}, {"Dreamy Trail", {32.0f, 470.0f, 43.0f, 0}} }} },
    { "DOUBLER", "Stereo vocal thickness and spread", {"doubler", "doublerSpread", nullptr, nullptr}, 2,
      {{ {"Off", {0.0f, 18.0f, 0, 0}}, {"Subtle Width", {18.0f, 16.0f, 0, 0}},
         {"Wide Backing", {45.0f, 24.0f, 0, 0}}, {"Lush Dream", {60.0f, 36.0f, 0, 0}} }} }
}};

inline void applyEffect (juce::AudioProcessorValueTreeState& state, int bank, int presetIndex)
{
    if (bank < 0 || bank >= static_cast<int> (effectBanks.size())
        || presetIndex < 0 || presetIndex >= 4)
        return;
    const auto& group = effectBanks[static_cast<size_t> (bank)];
    const auto& values = group.presets[static_cast<size_t> (presetIndex)].values;
    for (int i = 0; i < group.parameterCount; ++i)
    {
        if (auto* param = state.getParameter (group.parameterIds[static_cast<size_t> (i)]))
        {
            param->beginChangeGesture();
            param->setValueNotifyingHost (param->convertTo0to1 (values[static_cast<size_t> (i)]));
            param->endChangeGesture();
        }
    }
}

} // namespace DreamVoxPresets
