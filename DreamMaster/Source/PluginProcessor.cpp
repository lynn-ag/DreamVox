#include "PluginProcessor.h"
#include "PluginEditor.h"
juce::AudioProcessorValueTreeState::ParameterLayout DreamMasterProcessor::layout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout p;
    auto add=[&](const char* id,const char* name,float low,float high,float value,const char* unit)
    { p.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{id,1},name,juce::NormalisableRange<float>(low,high,.01f),value,juce::AudioParameterFloatAttributes().withLabel(unit))); };
    add("input","Input",-12,6,0,"dB");add("bass","Bass",-3,3,0,"dB");
    add("clarity","Clarity",0,100,0,"%");add("smooth","Smooth",0,100,0,"%");
    add("air","Air",-3,3,0,"dB");add("glue","Glue",0,100,0,"%");
    add("width","Width",80,120,100,"%");add("drive","Loudness",0,9,0,"dB");
    add("ceiling","Ceiling",-3,-.3f,-1,"dBFS");
    p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"lowcut",1},"Low cut",false));
    // Opt-in so existing FL Studio projects load with unchanged sound.
    p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"truepeak",1},"True peak limiting",false));
    p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"bypass",1},"Bypass",false));
    p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"automaster",1},"Automatic mastering",true));
    p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"levelmatch",1},"Level-matched bypass",true));
    return p;
}
DreamMasterProcessor::DreamMasterProcessor()
 : AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),parameters(*this,nullptr,"DreamMasterState",layout()) {}
bool DreamMasterProcessor::isBusesLayoutSupported(const BusesLayout& b) const
{ const auto c=b.getMainOutputChannelSet();return (c==juce::AudioChannelSet::mono()||c==juce::AudioChannelSet::stereo())&&c==b.getMainInputChannelSet(); }
void DreamMasterProcessor::prepareToPlay(double sr,int)
{
    engine.prepare(sr);
    loudness.prepare(sr,getTotalNumOutputChannels());
    truePeak.prepare(sr);
    setLatencySamples(engine.latency());
}
void DreamMasterProcessor::render(juce::AudioBuffer<float>& b,bool hostBypass)
{
    juce::ScopedNoDenormals guard;
    auto v=[&](const char* id){return parameters.getRawParameterValue(id)->load();};
    dream::Settings s;
    s.input=v("input");s.bass=v("bass");s.clarity=v("clarity");s.smooth=v("smooth");s.air=v("air");
    s.glue=v("glue");s.width=v("width");s.drive=v("drive");s.ceiling=v("ceiling");s.lowCut=v("lowcut")>.5f;s.truePeak=v("truepeak")>.5f;s.bypass=hostBypass||v("bypass")>.5f;s.autoMaster=v("automaster")>.5f;s.levelMatchBypass=!hostBypass&&v("levelmatch")>.5f;
    engine.process(b.getArrayOfWritePointers(),b.getNumChannels(),b.getNumSamples(),s);
    // Post-processing measurements: display real loudness and intersample peaks.
    // This does not change the sound or turn the existing sample-peak limiter into a true-peak limiter.
    const auto* const* output=b.getArrayOfReadPointers();
    loudness.process(output,b.getNumChannels(),b.getNumSamples());
    truePeak.process(output,b.getNumChannels(),b.getNumSamples());
    tpDb.store(static_cast<float>(truePeak.peakDbtp()),std::memory_order_relaxed);
    shortLufs.store(static_cast<float>(loudness.shortTermLufs()),std::memory_order_relaxed);
    integratedLufs.store(static_cast<float>(loudness.integratedLufs()),std::memory_order_relaxed);
    inDb.store(static_cast<float>(dream::db(engine.inputPeak)));outDb.store(static_cast<float>(dream::db(engine.outputPeak)));
    rmsDb.store(static_cast<float>(dream::db(engine.rms)));compDb.store(static_cast<float>(engine.compressReduction));limitDb.store(static_cast<float>(engine.limitReduction));
}
void DreamMasterProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&) { render(b,false); }
void DreamMasterProcessor::processBlockBypassed(juce::AudioBuffer<float>& b,juce::MidiBuffer&) { render(b,true); }
juce::AudioProcessorParameter* DreamMasterProcessor::getBypassParameter() const { return parameters.getParameter("bypass"); }
juce::AudioProcessorEditor* DreamMasterProcessor::createEditor() { return new DreamMasterEditor(*this); }
void DreamMasterProcessor::getStateInformation(juce::MemoryBlock& b) { auto xml=parameters.copyState().createXml();copyXmlToBinary(*xml,b); }
void DreamMasterProcessor::setStateInformation(const void* d,int n) { auto xml=getXmlFromBinary(d,n);if(xml&&xml->hasTagName(parameters.state.getType()))parameters.replaceState(juce::ValueTree::fromXml(*xml)); }
void DreamMasterProcessor::applyPreset(int index)
{
    const auto s=dream::preset(index);
    const char* ids[]={"input","bass","clarity","smooth","air","glue","width","drive","ceiling","lowcut"};
    const double values[]={s.input,s.bass,s.clarity,s.smooth,s.air,s.glue,s.width,s.drive,s.ceiling,s.lowCut?1.0:0.0};
    for(size_t i=0;i<std::size(ids);++i) if(auto* p=parameters.getParameter(ids[i])) { p->beginChangeGesture();p->setValueNotifyingHost(p->convertTo0to1(static_cast<float>(values[i])));p->endChangeGesture(); }
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new DreamMasterProcessor; }
