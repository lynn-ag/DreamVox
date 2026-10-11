#pragma once
#include "PluginProcessor.h"

class DreamVoxAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit DreamVoxAudioProcessorEditor (DreamVoxAudioProcessor&);
    void paint (juce::Graphics&) override;
    void resized() override;
private:
    DreamVoxAudioProcessor& processor;
    juce::OwnedArray<juce::Slider> controls;
    juce::OwnedArray<juce::Label> sectionLabels;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> attachments;
    juce::ToggleButton bypass {"BYPASS"};
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DreamVoxAudioProcessorEditor)
};
