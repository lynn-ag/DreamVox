#pragma once
#include "PluginProcessor.h"
#include "../../../Artwork/NightLook.h"
class DreamVoxAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit DreamVoxAudioProcessorEditor(DreamVoxAudioProcessor&);
    ~DreamVoxAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    void timerCallback() override { repaint(); }
    DreamVoxAudioProcessor& processor;
    night::Look look{false};
    juce::Image artwork;
    std::array<juce::Slider,8> controls;
    std::array<juce::Label,8> labels;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>,8> attachments;
    juce::ToggleButton bypass{"BYPASS"};
    juce::ComboBox presets,zoom;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;
    juce::TooltipWindow tips{this,500};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DreamVoxAudioProcessorEditor)
};
