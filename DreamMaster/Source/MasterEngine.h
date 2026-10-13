#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>
#include "TruePeakDetector.h"

namespace dream
{
inline double gain(double db) { return std::pow(10.0, db / 20.0); }
inline double db(double x) { return 20.0 * std::log10(std::max(x, 1.0e-9)); }
struct Settings
{
    double input=0, bass=0, clarity=0, smooth=0, air=0, glue=0;
    double width=100, drive=0, ceiling=-1;
    bool lowCut=false, bypass=false, truePeak=false, autoMaster=true, levelMatchBypass=true;
};

struct Biquad
{
    double b0=1,b1=0,b2=0,a1=0,a2=0;
    std::array<double,2> z1{},z2{};
    void reset() { z1={}; z2={}; }
    void coeff(double n0,double n1,double n2,double d0,double d1,double d2)
    { b0=n0/d0;b1=n1/d0;b2=n2/d0;a1=d1/d0;a2=d2/d0; }
    double tick(double x,int ch)
    { const auto y=b0*x+z1[ch]; z1[ch]=b1*x-a1*y+z2[ch];z2[ch]=b2*x-a2*y;return y; }
    void bell(double sr,double hz,double q,double boost)
    {
        const auto w=2*3.141592653589793*std::min(hz,sr*.4)/sr;
        const auto a=std::sin(w)/(2*q), A=gain(boost/2), c=std::cos(w);
        coeff(1+a*A,-2*c,1-a*A,1+a/A,-2*c,1-a/A);
    }
    void shelf(double sr,double hz,double boost,bool high)
    {
        const auto w=2*3.141592653589793*std::min(hz,sr*.4)/sr;
        const auto A=gain(boost/2),c=std::cos(w),beta=std::sqrt(2*A)*std::sin(w);
        if(high) coeff(A*((A+1)+(A-1)*c+beta),-2*A*((A-1)+(A+1)*c),A*((A+1)+(A-1)*c-beta),
                       (A+1)-(A-1)*c+beta,2*((A-1)-(A+1)*c),(A+1)-(A-1)*c-beta);
        else coeff(A*((A+1)-(A-1)*c+beta),2*A*((A-1)-(A+1)*c),A*((A+1)-(A-1)*c-beta),
                   (A+1)+(A-1)*c+beta,-2*((A-1)+(A+1)*c),(A+1)+(A-1)*c-beta);
    }
    void band(double sr,double hz,double q)
    {
        const auto w=2*3.141592653589793*std::min(hz,sr*.4)/sr;
        const auto a=std::sin(w)/(2*q),c=std::cos(w);
        coeff(a,0,-a,1+a,-2*c,1-a);
    }
    void highpass(double sr,double hz)
    {
        const auto w=2*3.141592653589793*hz/sr,c=std::cos(w),a=std::sin(w)/std::sqrt(2.0);
        coeff((1+c)/2,-(1+c),(1+c)/2,1+a,-2*c,1-a);
    }
};

// No allocation or locks in process(). All buffers are sized in prepare().
class MasterEngine
{
public:
    void prepare(double sampleRate)
    {
        sr=std::max(8000.0,sampleRate); delay=std::max(1,static_cast<int>(std::round(sr*.005)));
        size=delay+1; wet.assign(static_cast<size_t>(size),{}); dry.assign(static_cast<size_t>(size),{});
        queue.assign(static_cast<size_t>(size+1),{}); qhead=qtail=pos=0;counter=0;
        bass.reset();mid.reset();air.reset();harsh.reset();cut.reset();
        truePeakDetector.prepare();
        harsh.band(sr,3800,.7);cut.highpass(sr,22);
        settings={}; first=true; phase=0;envelope=harshEnv=sideLow=0;lowLevel=highLevel=0;lowState=0;bypassGain=1;limiterGain=1;wetMix=1;cutMix=0;tpMargin=0;
        inputPeak=outputPeak=rms=compressReduction=limitReduction=0;
        paramCoeff=std::exp(-1/(sr*.03));release=std::exp(-1/(sr*.12));
        attackComp=std::exp(-1/(sr*.03));releaseComp=std::exp(-1/(sr*.18));
        attackHarsh=std::exp(-1/(sr*.003));releaseHarsh=std::exp(-1/(sr*.07));
        sideCoeff=std::exp(-2*3.141592653589793*150/sr);
        mixCoeff=std::exp(-1/(sr*.005));
    }
    int latency() const { return delay; }
    void process(float* const* channels,int channelCount,int samples,const Settings& target)
    {
        if(size==0 || samples<=0 || channelCount<1) return;
        const int nc=std::min(channelCount,2);
        if(first) { settings=target;cutMix=target.lowCut?1:0;wetMix=target.bypass?0:1;first=false; }
        double inPeak=0,outPeak=0,energy=0,dryEnergy=0,maxComp=0,maxLimit=0;
        for(int n=0;n<samples;++n)
        {
            auto smooth=[&](double& value,double wanted) { value=wanted+paramCoeff*(value-wanted); };
            smooth(settings.input,target.input);smooth(settings.bass,target.bass);smooth(settings.clarity,target.clarity);
            smooth(settings.smooth,target.smooth);smooth(settings.air,target.air);smooth(settings.glue,target.glue);
            smooth(settings.width,target.width);smooth(settings.drive,target.drive);smooth(settings.ceiling,target.ceiling);
            cutMix=(target.lowCut?1.0:0.0)+mixCoeff*(cutMix-(target.lowCut?1.0:0.0));
            wetMix=(target.bypass?0.0:1.0)+mixCoeff*(wetMix-(target.bypass?0.0:1.0));
            // Avoid abrupt output-level jumps when high-fidelity limiting is toggled.
            tpMargin=(target.truePeak?.45:0.0)+paramCoeff*(tpMargin-(target.truePeak?.45:0.0));
            if(phase==0)
            {
                const auto autoBass=settings.autoMaster?std::clamp((highLevel-lowLevel)*2.0,-1.25,1.25):0.0;
                const auto autoAir=settings.autoMaster?std::clamp((lowLevel-highLevel)*1.5,-1.0,1.0):0.0;
                bass.shelf(sr,100,settings.bass+autoBass,false);
                mid.bell(sr,320,.75,-settings.clarity*.035);
                air.shelf(sr,10000,settings.air+autoAir,true);
            }
            phase=(phase+1)%32;
            std::array<double,2> x{},h{};
            const auto inGain=gain(settings.input);
            for(int ch=0;ch<nc;++ch)
            {
                auto raw=static_cast<double>(channels[ch][n]);
                if(!std::isfinite(raw)) raw=0;
                raw=std::clamp(raw,-64.0,64.0);
                dry[static_cast<size_t>(pos)][ch]=raw;
                inPeak=std::max(inPeak,std::abs(raw));
                if(ch==0)
                {
                    const auto lowCoeff=std::exp(-2*3.141592653589793*140/sr);
                    lowState=lowCoeff*lowState+(1-lowCoeff)*raw;
                    const auto low=lowState*lowState;
                    const auto high=(raw-lowState)*(raw-lowState);
                    lowLevel=0.9995*lowLevel+0.0005*low;
                    highLevel=0.9995*highLevel+0.0005*high;
                }
                auto v=raw*inGain;
                const auto filtered=cut.tick(v,ch);v+=cutMix*(filtered-v);
                x[ch]=air.tick(mid.tick(bass.tick(v,ch),ch),ch);
                h[ch]=harsh.tick(x[ch],ch);
            }
            const auto hp=std::max(std::abs(h[0]),std::abs(h[1]));
            const auto hc=hp>harshEnv?attackHarsh:releaseHarsh;
            harshEnv=hc*harshEnv+(1-hc)*hp;
            const auto reduction=std::clamp((db(harshEnv)+24)*.3,0.0,3.0)*settings.smooth/100;
            const auto hg=gain(-reduction)-1;
            for(int ch=0;ch<nc;++ch) x[ch]+=hg*h[ch];
            if(nc==2)
            {
                const auto m=(x[0]+x[1])*.5,s=(x[0]-x[1])*.5;
                sideLow=sideCoeff*sideLow+(1-sideCoeff)*s;
                const auto side=sideLow+(s-sideLow)*settings.width/100;
                x[0]=m+side;x[1]=m-side;
            }
            const auto p=std::max(std::abs(x[0]),std::abs(x[1]));
            const auto ec=p>envelope?attackComp:releaseComp;
            envelope=ec*envelope+(1-ec)*p;
            // Gentle 1.5:1 compression, 6 dB knee, no automatic makeup, max 3 dB GR.
            const auto adaptiveGlue=settings.autoMaster?std::clamp(0.35+0.0008*(db(envelope)+18),0.15,0.8):0.0;
            const auto glueAmount=settings.autoMaster?std::max(settings.glue/100.0,adaptiveGlue):settings.glue/100.0;
            const auto over=db(envelope)-(-12-settings.glue*.12);
            const auto curve=over<=-3?0:(over>=3?over/3:(over+3)*(over+3)/36);
            const auto gr=std::min(3.0,curve)*glueAmount;
            maxComp=std::max(maxComp,gr);
            const auto preLimit=gain(settings.drive-gr);
            for(int ch=0;ch<nc;++ch) wet[static_cast<size_t>(pos)][ch]=x[ch]*preLimit;
            double peak=std::max(std::abs(x[0]*preLimit),std::abs(x[1]*preLimit));
            if(target.truePeak)
            {
                const auto& delayed=wet[static_cast<size_t>(pos)];
                peak=std::max(peak,truePeakDetector.push(delayed[0],delayed[1],nc));
            }
            // Monotonic queue: maximum stereo peak over the entire lookahead window.
            const auto qsize=static_cast<int>(queue.size());
            while(qhead!=qtail && queue[static_cast<size_t>(qhead)].index+static_cast<uint64_t>(delay)<counter) qhead=(qhead+1)%qsize;
            while(qhead!=qtail)
            {
                const auto back=(qtail+qsize-1)%qsize;
                if(queue[static_cast<size_t>(back)].value>peak) break;
                qtail=back;
            }
            queue[static_cast<size_t>(qtail)]={counter++,peak};qtail=(qtail+1)%qsize;
            // True-peak safety margin covers interpolation/variable-gain uncertainty.
            // Legacy sample-peak mode remains bit-for-bit compatible by default.
            const auto ceiling=gain(settings.ceiling-tpMargin);
            const auto wanted=std::min(1.0,ceiling/std::max(1.0e-12,queue[static_cast<size_t>(qhead)].value));
            limiterGain=std::min(wanted,release*limiterGain+(1-release)*wanted);
            maxLimit=std::max(maxLimit,-db(limiterGain));
            const auto read=(pos+1)%size;
            for(int ch=0;ch<nc;++ch)
            {
                const auto processed=wet[static_cast<size_t>(read)][ch]*limiterGain;
                const auto bypassed=dry[static_cast<size_t>(read)][ch];
                const auto matchedBypass=settings.levelMatchBypass?bypassed*bypassGain:bypassed;
                const auto y=matchedBypass+wetMix*(processed-matchedBypass);
                channels[ch][n]=static_cast<float>(y);outPeak=std::max(outPeak,std::abs(y));energy+=y*y;
                dryEnergy+=bypassed*bypassed;
            }
            pos=read;
        }
        const auto fall=std::exp(-samples/(sr*.6));
        inputPeak=std::max(inPeak,inputPeak*fall);outputPeak=std::max(outPeak,outputPeak*fall);
        const auto rmsFall=std::exp(-samples/(sr*.3));
        rms=std::sqrt(rmsFall*rms*rms+(1-rmsFall)*energy/(samples*nc));
        if(settings.levelMatchBypass && dryEnergy>1.0e-12 && energy>1.0e-12)
        {
            const auto wanted=std::clamp(std::sqrt(energy/dryEnergy),.5,2.0);
            bypassGain=wanted+std::exp(-samples/(sr*.08))*(bypassGain-wanted);
        }
        compressReduction=std::max(maxComp,compressReduction*fall);limitReduction=std::max(maxLimit,limitReduction*fall);
    }
    double inputPeak=0,outputPeak=0,rms=0,compressReduction=0,limitReduction=0;
private:
    struct Peak { uint64_t index=0;double value=0; };
    std::vector<std::array<double,2>> wet,dry;
    std::vector<Peak> queue;
    int delay=0,size=0,pos=0,qhead=0,qtail=0,phase=0;
    uint64_t counter=0;
    double sr=44100,paramCoeff=0,release=0,attackComp=0,releaseComp=0,attackHarsh=0,releaseHarsh=0,sideCoeff=0,mixCoeff=0;
    double envelope=0,harshEnv=0,sideLow=0,lowLevel=0,highLevel=0,lowState=0,bypassGain=1,limiterGain=1,wetMix=1,cutMix=0,tpMargin=0;
    bool first=true;
    Settings settings;
    Biquad bass,mid,air,harsh,cut;
    TruePeakDetector truePeakDetector;
};
}
