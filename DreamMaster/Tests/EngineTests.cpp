#include "../Source/Presets.h"
#include <iostream>
#include <random>
#include <limits>
#include <stdexcept>
using Audio=std::array<std::vector<float>,2>;
void require(bool ok,const char* message) { if(!ok)throw std::runtime_error(message); }
Audio run(dream::MasterEngine& e,const Audio& input,int nc,int block,const dream::Settings& s)
{
    auto out=input;
    for(size_t offset=0;offset<input[0].size();offset+=static_cast<size_t>(block))
    {
        float* p[]={out[0].data()+offset,out[1].data()+offset};
        e.process(p,nc,static_cast<int>(std::min(static_cast<size_t>(block),input[0].size()-offset)),s);
    }
    return out;
}
int main()
{
 try {
    for(const auto sr:{8000,44100,48000,96000,192000}) for(int nc:{1,2})
    {
        dream::MasterEngine e;e.prepare(sr);dream::Settings s;
        Audio input;for(auto& c:input)c.resize(static_cast<size_t>(sr));
        std::mt19937 rng(47);std::uniform_real_distribution<float> noise(-.1f,.1f);
        for(int i=0;i<sr;++i) { input[0][i]=noise(rng);input[1][i]=-.5f*input[0][i]; }
        const auto neutral=run(e,input,nc,127,s);
        for(int ch=0;ch<nc;++ch)for(int i=0;i<sr;++i)
            require(std::abs(neutral[ch][i]-(i<e.latency()?0:input[ch][i-e.latency()]))<1.e-6f,"Neutral is not delayed unity");
        s.bypass=true;s.input=6;s.bass=3;s.clarity=100;s.smooth=100;s.air=3;s.glue=100;s.width=120;s.drive=9;s.lowCut=true;
        e.prepare(sr);const auto bypass=run(e,input,nc,512,s);
        for(int ch=0;ch<nc;++ch)for(int i=0;i<sr;++i)
            require(std::abs(bypass[ch][i]-(i<e.latency()?0:input[ch][i-e.latency()]))<1.e-6f,"Bypass is not delayed original");
        s={};s.drive=9;
        for(int i=0;i<sr;++i){input[0][i]=4*std::sin(static_cast<float>(i)*.137f);input[1][i]=-.5f*input[0][i];}
        e.prepare(sr);const auto limited=run(e,input,nc,61,s);
        for(int ch=0;ch<nc;++ch)for(float v:limited[ch])require(std::isfinite(v)&&std::abs(v)<=dream::gain(-1)+1.e-6,"Ceiling exceeded");
        if(nc==2)for(int i=0;i<sr;++i) require(std::abs(limited[1][i]+.5f*limited[0][i])<1.e-6f,"Limiter is not stereo linked");
        // The engine must produce the same result for arbitrary host buffer partitioning.
        s.clarity=40;s.smooth=50;s.glue=50;s.width=110;s.bass=1;s.air=-1;s.lowCut=true;
        dream::MasterEngine a,b;a.prepare(sr);b.prepare(sr);
        const auto one=run(a,input,nc,1,s),many=run(b,input,nc,1024,s);
        for(int ch=0;ch<nc;++ch)for(int i=0;i<sr;++i)require(std::abs(one[ch][i]-many[ch][i])<1.e-6f,"Block partition changed audio");
        // Full parameter-range automation, non-finite input, zero-length blocks.
        e.prepare(sr);
        for(int block=0;block<200;++block)
        {
            s.input=block%2?6:-12;s.bass=block%2?3:-3;s.air=-s.bass;s.drive=block%2?9:0;
            s.ceiling=block%2?-.3:-3;s.width=block%2?80:120;s.glue=block%2?100:0;s.lowCut=block%2==0;
            float l[64],r[64];for(int i=0;i<64;++i){l[i]=noise(rng)*10;r[i]=noise(rng)*10;}
            if(block==50)l[0]=std::numeric_limits<float>::quiet_NaN();
            float* ptr[]={l,r};e.process(ptr,nc,0,s);e.process(ptr,nc,64,s);
            for(int ch=0;ch<nc;++ch)for(int i=0;i<64;++i)require(std::isfinite(ptr[ch][i])&&std::abs(ptr[ch][i])<=1,"Automation produced invalid audio");
        }
        e.prepare(sr);Audio silence;for(auto& c:silence)c.resize(5000);
        const auto quiet=run(e,silence,nc,256,s);
        for(int ch=0;ch<nc;++ch)for(auto v:quiet[ch])require(v==0,"Reset silence is not silent");
        std::cout<<"PASS "<<sr<<" Hz, "<<nc<<" channels: unity, latency, bypass, ceiling, stereo link, block sizes, automation, reset\n";
    }
    // Exercise every preset on over-level, transient-rich stereo/mono material.
    for(const auto sr:{44100,48000,96000,192000})for(int nc:{1,2})for(int index=0;index<dream::presetCount;++index)
    {
        dream::MasterEngine engine;engine.prepare(sr);const auto settings=dream::preset(index);
        Audio source;for(auto& c:source)c.resize(static_cast<size_t>(sr/2));
        for(size_t i=0;i<source[0].size();++i) {
            source[0][i]=static_cast<float>(1.2*std::sin(i*2*3.141592653589793*70/sr)+.6*std::sin(i*2*3.141592653589793*3800/sr));
            source[1][i]=static_cast<float>(.9*std::sin(i*2*3.141592653589793*130/sr));
            if(i%997==0)source[0][i]=2.f;
        }
        const auto output=run(engine,source,nc,137,settings);
        double energy=0;
        for(int ch=0;ch<nc;++ch)for(auto v:output[ch]){
            require(std::isfinite(v)&&std::abs(v)<=dream::gain(settings.ceiling)+1.e-6,"Preset exceeded sample ceiling");
            energy+=v*v;
        }
        require(energy>0.01,"Preset unexpectedly muted audio");
    }
    std::cout<<"PASS all 13 presets: finite output, sample ceiling, non-silent output at 4 sample rates in mono/stereo\n";
    std::cout<<"ALL DSP TESTS PASSED\n";return 0;
 } catch(const std::exception& ex) { std::cerr<<"FAIL: "<<ex.what()<<"\n";return 1; }
}
