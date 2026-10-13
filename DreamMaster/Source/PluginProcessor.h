#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "MasterEngine.h"
#include "LoudnessMeter.h"
#include "TruePeakMeter.h"
#include "Presets.h"
class DreamMasterProcessor final : public juce::AudioProcessor
{
public:
    DreamMasterProcessor();
    void prepareToPlay(double,int) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
    void processBlockBypassed(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "DreamMaster"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return .5; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int,const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*,int) override;
    juce::AudioProcessorParameter* getBypassParameter() const override;
    void applyPreset(int);
    juce::AudioProcessorValueTreeState parameters;
    std::atomic<float> inDb{-90},outDb{-90},rmsDb{-90},compDb{0},limitDb{0};
    std::atomic<float> tpDb{-90},shortLufs{-90},integratedLufs{-90};
private:
    static juce::AudioProcessorValueTreeState::ParameterLayout layout();
    void render(juce::AudioBuffer<float>&,bool);
    dream::MasterEngine engine;
    dream::LoudnessMeter loudness;
    dream::TruePeakMeter truePeak;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DreamMasterProcessor)
};
