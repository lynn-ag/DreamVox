#pragma once
#include "PluginProcessor.h"
#include "../../Artwork/NightLook.h"
class DreamMasterEditor final : public juce::AudioProcessorEditor,private juce::Timer {
public:
    explicit DreamMasterEditor(DreamMasterProcessor&);
    ~DreamMasterEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    void timerCallback() override {repaint();}
    DreamMasterProcessor& processor;
    night::Look look{true};
    juce::Image artwork;
    std::array<juce::Slider,9> controls;
    std::array<juce::Label,9> labels;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>,9> attachments;
    juce::ComboBox presets,zoom;
    juce::Label presetHelp;
    juce::ToggleButton bypass{"BYPASS"},lowCut{"LOW CUT 22 Hz"};
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment,cutAttachment;
    juce::TooltipWindow tips{this,500};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DreamMasterEditor)
};
