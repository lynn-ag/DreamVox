#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

class DreamVoxAudioProcessor final : public juce::AudioProcessor
{
public:
    DreamVoxAudioProcessor();
    ~DreamVoxAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "DreamVox"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;
    bool isBusesLayoutSupported (const BusesLayout&) const override;

    juce::AudioProcessorValueTreeState parameters;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    struct Biquad
    {
        double b0=1.0, b1=0.0, b2=0.0, a1=0.0, a2=0.0;
        double z1[2] {0.0, 0.0}, z2[2] {0.0, 0.0};
        float process (float x, int ch) noexcept;
        void set (double nb0, double nb1, double nb2, double na0, double na1, double na2) noexcept;
        void reset() noexcept;
    };
    Biquad lowShelf, midBell, highShelf;
    double sampleRate = 44100.0;
    float compEnvelope = 0.0f;
    float deEssEnvelope = 0.0f;
    float detectorIn[2] {0.0f, 0.0f};
    float detectorHp[2] {0.0f, 0.0f};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DreamVoxAudioProcessor)
};
