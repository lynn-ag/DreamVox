#pragma once
#include <array>
#include <atomic>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

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
    const juce::String getName() const override { return "DreamVox 5"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 4.0; }
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

    // Read-only UI monitoring. Audio thread publishes atomically; GUI never locks the DSP.
    static constexpr int monitorSize = 2048;
    std::array<std::atomic<float>, monitorSize> waveform {};
    std::atomic<unsigned int> waveformWrite {0};
    std::array<std::atomic<float>, 2> inputPeaks {};
    std::array<std::atomic<float>, 2> outputPeaks {};
    std::atomic<float> compressorReductionDb {0.0f};
    std::atomic<float> deesserReductionDb {0.0f};

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
    void publishMonitoring (const juce::AudioBuffer<float>&, float compressorDb, float deesserDb) noexcept;
    float cleanHpIn[2] {0.0f, 0.0f}, cleanHpOut[2] {0.0f, 0.0f};
    float gateEnvelope = 0.0f, gateGain = 1.0f;
    juce::Reverb reverb;
    juce::AudioBuffer<float> delayBuffer;
    juce::AudioBuffer<float> wetBuffer;
    int delayWrite = 0;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> delayWet, reverbWet, doubleWet;
    double sampleRate = 44100.0;
    float compEnvelope = 0.0f;
    float deEssEnvelope = 0.0f;
    float detectorIn[2] {0.0f, 0.0f};
    float detectorHp[2] {0.0f, 0.0f};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DreamVoxAudioProcessor)
};
