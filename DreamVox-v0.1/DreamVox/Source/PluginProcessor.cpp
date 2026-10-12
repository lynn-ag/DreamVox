#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

namespace
{
constexpr float pi = 3.14159265358979323846f;
std::unique_ptr<juce::AudioParameterFloat> knob (const char* id, const char* name, float lo, float hi, float def, const char* unit = "")
{
    return std::make_unique<juce::AudioParameterFloat> (juce::ParameterID {id, 1}, name,
        juce::NormalisableRange<float> (lo, hi), def,
        juce::AudioParameterFloatAttributes().withLabel (unit));
}
}

juce::AudioProcessorValueTreeState::ParameterLayout DreamVoxAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout p;
    p.add (knob ("low", "LOW", -6.0f, 6.0f, 0.0f, " dB"));
    p.add (knob ("body", "BODY", -6.0f, 6.0f, 0.0f, " dB"));
    p.add (knob ("air", "AIR", -6.0f, 6.0f, 0.0f, " dB"));
    p.add (knob ("threshold", "THRESHOLD", -36.0f, 0.0f, -18.0f, " dB"));
    p.add (knob ("ratio", "RATIO", 1.0f, 8.0f, 2.5f, ":1"));
    p.add (knob ("deess", "DE-ESS", 0.0f, 100.0f, 28.0f, "%"));
    p.add (knob ("warmth", "WARMTH", 0.0f, 100.0f, 14.0f, "%"));
    p.add (knob ("output", "OUTPUT", -12.0f, 12.0f, 0.0f, " dB"));
    p.add (knob ("space", "REVERB", 0.0f, 100.0f, 0.0f, "%"));
    p.add (knob ("room", "ROOM SIZE", 0.0f, 100.0f, 45.0f, "%"));
    p.add (knob ("echo", "DELAY MIX", 0.0f, 100.0f, 0.0f, "%"));
    p.add (knob ("echoTime", "DELAY TIME", 80.0f, 850.0f, 330.0f, " ms"));
    p.add (knob ("feedback", "FEEDBACK", 0.0f, 65.0f, 24.0f, "%"));
    p.add (knob ("doubler", "DOUBLER", 0.0f, 100.0f, 0.0f, "%"));
    p.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID {"bypass", 1}, "BYPASS", false));
    return p;
}

DreamVoxAudioProcessor::DreamVoxAudioProcessor()
    : AudioProcessor (BusesProperties().withInput ("Input", juce::AudioChannelSet::stereo(), true)
                                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "DreamVoxState", createParameterLayout())
{}

bool DreamVoxAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    return (out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo())
        && out == layouts.getMainInputChannelSet();
}

void DreamVoxAudioProcessor::prepareToPlay (double sr, int)
{
    sampleRate = juce::jmax (8000.0, sr);
    lowShelf.reset(); midBell.reset(); highShelf.reset();
    reverb.setSampleRate (sampleRate);
    reverb.reset();
    delayBuffer.setSize (2, static_cast<int> (sampleRate * 2.0) + 4);
    delayBuffer.clear();
    wetBuffer.setSize (2, 8192);
    delayWrite = 0;
    for (auto* s : { &delayWet, &reverbWet, &doubleWet })
        s->reset (sampleRate, 0.03);
    delayWet.setCurrentAndTargetValue (0.0f);
    reverbWet.setCurrentAndTargetValue (0.0f);
    doubleWet.setCurrentAndTargetValue (0.0f);
    compEnvelope = deEssEnvelope = 0.0f;
    for (int i=0;i<2;++i) detectorIn[i] = detectorHp[i] = 0.0f;
}
void DreamVoxAudioProcessor::releaseResources() {}

void DreamVoxAudioProcessor::Biquad::set (double nb0, double nb1, double nb2, double na0, double na1, double na2) noexcept
{
    if (std::abs (na0) < 1.0e-12) return;
    b0=nb0/na0; b1=nb1/na0; b2=nb2/na0; a1=na1/na0; a2=na2/na0;
}
float DreamVoxAudioProcessor::Biquad::process (float x, int ch) noexcept
{
    const double y=b0*x+z1[ch];
    z1[ch]=b1*x-a1*y+z2[ch]; z2[ch]=b2*x-a2*y;
    return static_cast<float>(y);
}
void DreamVoxAudioProcessor::Biquad::reset() noexcept
{
    for (int i=0;i<2;++i) z1[i]=z2[i]=0.0;
}

void DreamVoxAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int channels=buffer.getNumChannels(), samples=buffer.getNumSamples();
    if (parameters.getRawParameterValue("bypass")->load() > 0.5f) return;
    const float lowDb=parameters.getRawParameterValue("low")->load();
    const float bodyDb=parameters.getRawParameterValue("body")->load();
    const float airDb=parameters.getRawParameterValue("air")->load();
    const float threshold=parameters.getRawParameterValue("threshold")->load();
    const float ratio=parameters.getRawParameterValue("ratio")->load();
    const float deEss=parameters.getRawParameterValue("deess")->load()/100.0f;
    const float warmth=parameters.getRawParameterValue("warmth")->load()/100.0f;
    delayWet.setTargetValue (parameters.getRawParameterValue("echo")->load() * 0.01f);
    reverbWet.setTargetValue (parameters.getRawParameterValue("space")->load() * 0.01f);
    doubleWet.setTargetValue (parameters.getRawParameterValue("doubler")->load() * 0.01f);
    const float delayMs = parameters.getRawParameterValue("echoTime")->load();
    const float feedback = parameters.getRawParameterValue("feedback")->load() * 0.01f;
    juce::Reverb::Parameters rev;
    rev.roomSize = 0.15f + parameters.getRawParameterValue("room")->load() * 0.0075f;
    rev.damping = 0.55f;
    rev.wetLevel = 1.0f;
    rev.dryLevel = 0.0f;
    rev.width = 0.85f;
    rev.freezeMode = 0.0f;
    reverb.setParameters (rev);
    const float output=juce::Decibels::decibelsToGain(parameters.getRawParameterValue("output")->load());

    auto setShelf = [this] (Biquad& f, float hz, float gainDb, bool high)
    {
        const double A=std::pow(10.0, gainDb/40.0), w=2.0*juce::MathConstants<double>::pi*hz/sampleRate;
        const double c=std::cos(w), s=std::sin(w), alpha=s*0.70710678118, beta=2.0*std::sqrt(A)*alpha;
        if (!high) f.set(A*((A+1)-(A-1)*c+beta), 2*A*((A-1)-(A+1)*c), A*((A+1)-(A-1)*c-beta), (A+1)+(A-1)*c+beta, -2*((A-1)+(A+1)*c), (A+1)+(A-1)*c-beta);
        else f.set(A*((A+1)+(A-1)*c+beta), -2*A*((A-1)+(A+1)*c), A*((A+1)+(A-1)*c-beta), (A+1)-(A-1)*c+beta, 2*((A-1)-(A+1)*c), (A+1)-(A-1)*c-beta);
    };
    setShelf (lowShelf, 180.0f, lowDb, false);
    setShelf (highShelf, 9000.0f, airDb, true);
    const double A=std::pow(10.0, bodyDb/40.0), w=2.0*juce::MathConstants<double>::pi*1100.0/sampleRate;
    const double alpha=std::sin(w)/(2.0*0.8), c=std::cos(w);
    midBell.set(1+alpha*A, -2*c, 1-alpha*A, 1+alpha/A, -2*c, 1-alpha/A);

    const float compAtk=std::exp(-1.0f/(0.012f*static_cast<float>(sampleRate)));
    const float compRel=std::exp(-1.0f/(0.120f*static_cast<float>(sampleRate)));
    const float deAtk=std::exp(-1.0f/(0.003f*static_cast<float>(sampleRate)));
    const float deRel=std::exp(-1.0f/(0.080f*static_cast<float>(sampleRate)));
    const float hpA=std::exp(-2.0f*pi*6200.0f/static_cast<float>(sampleRate));
    const float satDrive=1.0f+warmth*3.5f, satNorm=std::tanh(satDrive);
    const float thresholdLin=juce::Decibels::decibelsToGain(threshold);
    const float makeup=juce::Decibels::decibelsToGain(juce::jmax(0.0f, -threshold*0.10f));

    for (int n=0;n<samples;++n)
    {
        float detector=0.0f;
        for (int ch=0;ch<channels;++ch) detector += std::abs(buffer.getSample(ch,n));
        detector /= static_cast<float>(juce::jmax(1,channels));
        const float compCoef=detector>compEnvelope ? compAtk : compRel;
        compEnvelope=compCoef*compEnvelope+(1.0f-compCoef)*detector;
        float compGain=1.0f;
        if (compEnvelope>thresholdLin)
        {
            const float level=juce::Decibels::gainToDecibels(compEnvelope);
            const float reduced=threshold+(level-threshold)/ratio;
            compGain=juce::Decibels::decibelsToGain(reduced-level)*makeup;
        }
        float deGain=1.0f;
        for (int ch=0;ch<channels;++ch)
        {
            const float in=buffer.getSample(ch,n);
            const float hp=hpA*(detectorHp[ch]+in-detectorIn[ch]);
            detectorIn[ch]=in; detectorHp[ch]=hp;
            deEssEnvelope=(std::abs(hp)>deEssEnvelope?deAtk:deRel)*deEssEnvelope+(1.0f-(std::abs(hp)>deEssEnvelope?deAtk:deRel))*std::abs(hp);
        }
        if (deEssEnvelope>0.12f) deGain=1.0f-deEss*juce::jlimit(0.0f,0.65f,(deEssEnvelope-0.12f)*2.5f);
        const float echoMix = delayWet.getNextValue();
        const float doublerMix = doubleWet.getNextValue();
        for (int ch=0;ch<channels;++ch)
        {
            float x=buffer.getSample(ch,n);
            x=lowShelf.process(x,ch); x=midBell.process(x,ch); x=highShelf.process(x,ch);
            const float hp=detectorHp[ch];
            x += hp*(deGain-1.0f);
            x *= compGain;
            const float saturated=std::tanh(x*satDrive)/satNorm;
            x=(x*(1.0f-warmth)+saturated*warmth)*output;
            const int length = delayBuffer.getNumSamples();
            const int delaySamples = juce::jlimit (1, length - 2, static_cast<int> (delayMs * 0.001f * sampleRate));
            const int read = (delayWrite + length - delaySamples) % length;
            const float echoSample = delayBuffer.getSample (ch, read);
            const int doublerOffset = static_cast<int> (sampleRate * (ch == 0 ? 0.018 : 0.027));
            const int doubleRead = (delayWrite + length - doublerOffset) % length;
            const float doubled = delayBuffer.getSample (ch, doubleRead);
            delayBuffer.setSample (ch, delayWrite, x + echoSample * feedback);
            x = x * (1.0f - 0.22f * doublerMix) + doubled * 0.22f * doublerMix;
            x += echoSample * echoMix * 0.45f;
            buffer.setSample(ch,n,x);
        }
        delayWrite = (delayWrite + 1) % delayBuffer.getNumSamples();
    }
    jassert (samples <= wetBuffer.getNumSamples());
    if (samples > wetBuffer.getNumSamples()) return;
    for (int ch = 0; ch < channels; ++ch)
        wetBuffer.copyFrom (ch, 0, buffer, ch, 0, samples);
    if (channels == 1)
        reverb.processMono (wetBuffer.getWritePointer (0), samples);
    else if (channels >= 2)
        reverb.processStereo (wetBuffer.getWritePointer (0), wetBuffer.getWritePointer (1), samples);
    for (int n = 0; n < samples; ++n)
    {
        const float mix = reverbWet.getNextValue() * 0.38f;
        for (int ch = 0; ch < channels; ++ch)
            buffer.setSample (ch, n, buffer.getSample (ch, n) * (1.0f - mix) + wetBuffer.getSample (ch, n) * mix);
    }
}

juce::AudioProcessorEditor* DreamVoxAudioProcessor::createEditor() { return new DreamVoxAudioProcessorEditor (*this); }
void DreamVoxAudioProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    auto state=parameters.copyState(); std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml,dest);
}
void DreamVoxAudioProcessor::setStateInformation (const void* data, int size)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data,size));
    if (xml && xml->hasTagName(parameters.state.getType())) parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new DreamVoxAudioProcessor(); }
