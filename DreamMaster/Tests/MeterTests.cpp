#include "../Source/LoudnessMeter.h"
#include "../Source/TruePeakMeter.h"
#include <vector>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <limits>

static void check(bool b, const char* why) { if(!b) throw std::runtime_error(why); }
static constexpr double pi=3.1415926535897932384626433832795;

static void runTests(int sr) {
    dream::LoudnessMeter loud;
    loud.prepare(sr,1);
    std::vector<float> input(sr*4);
    for(size_t i=0;i<input.size();++i) input[i]=.1f*std::sin(2*pi*1000*i/sr);
    for(size_t i=0;i<input.size();i+=137) {
        const float* p[]={input.data()+i};
        loud.process(p,1,(int)std::min<size_t>(137,input.size()-i));
    }
    // 0.1 peak mono 1kHz sine: close to -23 LUFS, across common sample rates.
    const double expected=-23.0;
    check(std::abs(loud.integratedLufs()-expected)<.35,"Integrated sine loudness drift");
    check(std::abs(loud.momentaryLufs()-loud.shortTermLufs())<.12,"Steady momentary/short difference");
    const double first=loud.integratedLufs();
    loud.prepare(sr,1);
    check(loud.integratedLufs()==-90,"Reset integrated loudness");
    loud.prepare(sr,2);
    for(size_t i=0;i<input.size();i+=113) {
        const float* p[]={input.data()+i,input.data()+i};
        loud.process(p,2,(int)std::min<size_t>(113,input.size()-i));
    }
    check(std::abs(loud.integratedLufs()-first-10*std::log10(2.0))<.03,"Stereo K weighting (+3 dB) incorrect");
    // A quiet tail should not substantially affect gated integrated loudness.
    loud.prepare(sr,1);
    const float* src[]={input.data()};
    loud.process(src,1,(int)input.size());
    const double baseline=loud.integratedLufs();
    std::vector<float> quiet(sr*4,0.f);
    for(size_t i=0;i<quiet.size();++i) quiet[i]=1.e-5f*std::sin(2*pi*1000*i/sr);
    for(size_t i=0;i<quiet.size();i+=149) {
        const float* ptr[]={quiet.data()+i};
        loud.process(ptr,1,(int)std::min<size_t>(149,quiet.size()-i));
    }
    check(std::abs(loud.integratedLufs()-baseline)<.25,"Gating included very quiet tail");
    // Silence and nonfinite audio must not contaminate state.
    loud.prepare(sr,1);
    std::vector<float> silence(sr,0.f);
    silence[0]=std::numeric_limits<float>::quiet_NaN();
    const float* ps[]={silence.data()}; loud.process(ps,1,(int)silence.size());
    check(loud.integratedLufs()==-90,"Silence must read gated -90");
    // 12 kHz at 48 kHz with 45-degree phase gives sampled peaks near 0.707
    // and an intersample peak near 1.0.
    if(sr==48000) {
        dream::TruePeakMeter tp;tp.prepare(sr);
        std::vector<float> p(4800);
        for(size_t i=0;i<p.size();++i)p[i]=std::sin(2*pi*12000*i/sr+pi/4);
        double samplePeak=0;for(float v:p)samplePeak=std::max(samplePeak,std::abs((double)v));
        for(size_t i=0;i<p.size();i+=79) {
            const float* ptr[]={p.data()+i};
            tp.process(ptr,1,(int)std::min<size_t>(79,p.size()-i));
        }
        check(samplePeak<.71,"Unexpected sampled peak");
        check(tp.peakLinear()>.94&&tp.peakLinear()<1.10,"4x meter missed intersample peak");
    }
    std::cout<<"PASS metering "<<sr<<"Hz mono/stereo gated LUFS, reset, silence, TP\n";
}
int main() {
    try {for(int sr:{8000,44100,48000,96000,192000})runTests(sr);return 0;}
    catch(const std::exception& ex) {std::cerr<<"FAIL "<<ex.what()<<"\n";return 1;}
}
