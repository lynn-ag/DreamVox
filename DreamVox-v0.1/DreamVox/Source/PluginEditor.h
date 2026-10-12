#pragma once
#include "PluginProcessor.h"
#include <array>
#include <vector>

// Pure JUCE artwork: these widgets are independently clickable, not PNG hotspots.
class DreamVoxEffectCard final : public juce::TextButton
{
public:
    int moduleIndex = 0;
    bool selected = false;
    void paintButton (juce::Graphics&, bool highlighted, bool down) override;
};

class DreamVoxKnobLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider (juce::Graphics&, int, int, int, int, float, float,
                           float, juce::Slider&) override;
};

class DreamVoxAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit DreamVoxAudioProcessorEditor (DreamVoxAudioProcessor&);
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void selectModule (int index);
    void paintMonitor (juce::Graphics&);
    void drawPeakMeter (juce::Graphics&, juce::Rectangle<float>, const std::array<float, 2>&,
                        const juce::String&) const;
    void paintSelectedModuleMonitor (juce::Graphics&);

    DreamVoxAudioProcessor& processor;
    DreamVoxKnobLookAndFeel knobLook;
    juce::Image neonArtwork;
    juce::OwnedArray<juce::Slider> controls;
    juce::OwnedArray<juce::Label> sectionLabels;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> attachments;
    std::array<DreamVoxEffectCard, 8> moduleButtons;
    juce::ComboBox presetSelector, effectPresetSelector;
    juce::ToggleButton bypass {"BYPASS"};
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;

    int selectedModule = 0;
    std::array<float, 2> shownInput {{0.0f, 0.0f}}, shownOutput {{0.0f, 0.0f}};
    float shownCompReduction = 0.0f, shownDeessReduction = 0.0f;
    juce::dsp::FFT fft {10}; // FFT runs on the message thread, never the audio thread.
    std::array<float, 2048> fftScratch {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DreamVoxAudioProcessorEditor)
};
