#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <vector>

namespace dream {
// ITU-R BS.1770 K-weighting and EBU R128 absolute/relative gating.
// Stereo and mono only. Memory is reserved in prepare(); process() never allocates.
class LoudnessMeter {
public:
    void prepare(double sampleRate, int channelCount) {
        rate = std::max(8000.0, sampleRate);
        channels = std::clamp(channelCount, 1, 2);
        const double shelfF = 1681.974450955533;
        const double shelfQ = 0.7071752369554196;
        const double shelfDb = 3.999843853973347;
        const double k = std::tan(pi * shelfF / rate);
        const double vh = std::pow(10.0, shelfDb / 20.0);
        const double vb = std::pow(vh, 0.4996667741545416);
        const double d = 1.0 + k / shelfQ + k*k;
        const Coeff shelf{(vh + vb*k/shelfQ + k*k)/d,
                          2*(k*k - vh)/d, (vh - vb*k/shelfQ + k*k)/d,
                          2*(k*k - 1)/d, (1 - k/shelfQ + k*k)/d};
        const double hpK = std::tan(pi * 38.13547087602444 / rate);
        const double hpQ = 0.5003270373238773;
        const double hpD = 1 + hpK/hpQ + hpK*hpK;
        // BS.1770 RLB filter numerator deliberately is not divided by hpD.
        const Coeff highPass{1,-2,1,2*(hpK*hpK-1)/hpD,(1-hpK/hpQ+hpK*hpK)/hpD};
        for (auto& c : filters) { c.shelf.configure(shelf); c.highpass.configure(highPass); }
        count400 = std::max<size_t>(1, static_cast<size_t>(std::llround(rate*.4)));
        count3000 = std::max<size_t>(1, static_cast<size_t>(std::llround(rate*3)));
        hop = std::max<size_t>(1, static_cast<size_t>(std::llround(rate*.1)));
        ring400.assign(count400, 0);
        ring3000.assign(count3000, 0);
        // 90 minutes at 10 blocks/s; bounded integrated history for real-time safety.
        energies.assign(54000, 0);
        pos400=pos3000=frames=blocks=0;
        sum400=sum3000=0;
        momentary=shortTerm=integrated=-90;
    }
    void process(const float* const* input, int nChannels, int samples) {
        if (nChannels<1 || !input || samples<=0 || ring400.empty()) return;
        const auto nc = std::min(channels, nChannels);
        for (int i=0;i<samples;++i) {
            double power=0;
            for (int ch=0;ch<nc;++ch) {
                double x=input[ch][i];
                if (!std::isfinite(x)) x=0;
                x=filters[static_cast<size_t>(ch)].shelf.tick(x);
                x=filters[static_cast<size_t>(ch)].highpass.tick(x);
                power+=x*x;
            }
            sum400 += power-ring400[pos400]; ring400[pos400]=power;
            if (++pos400==count400) pos400=0;
            sum3000 += power-ring3000[pos3000]; ring3000[pos3000]=power;
            if (++pos3000==count3000) pos3000=0;
            ++frames;
            if (frames>=count400) {
                momentary=toLufs(sum400/count400);
                if ((frames-count400)%hop==0) {
                    const double mean=std::max(0.0, sum400/count400);
                    // Absolute gate: discard blocks below -70 LUFS.
                    if (toLufs(mean)>=-70.0) {
                        energies[blocks%energies.size()]=mean;
                        ++blocks;
                    }
                    recomputeIntegrated();
                }
            }
            if (frames>=count3000) shortTerm=toLufs(sum3000/count3000);
        }
    }
    double momentaryLufs() const { return momentary; }
    double shortTermLufs() const { return shortTerm; }
    double integratedLufs() const { return integrated; }
private:
    static constexpr double pi=3.1415926535897932384626433832795;
    struct Coeff { double b0,b1,b2,a1,a2; };
    struct Filter {
        Coeff c{1,0,0,0,0}; double s1=0,s2=0;
        void configure(Coeff v) { c=v; s1=s2=0; }
        double tick(double x) {
            const double y=c.b0*x+s1;
            s1=c.b1*x-c.a1*y+s2;
            s2=c.b2*x-c.a2*y;
            // Rejection of denormal states avoids excessive CPU on silence.
            if (std::abs(s1)<1.e-25) s1=0;
            if (std::abs(s2)<1.e-25) s2=0;
            return y;
        }
    };
    struct Channel { Filter shelf,highpass; };
    std::array<Channel,2> filters{};
    static double toLufs(double power) {
        return power>1.e-14 ? -0.691+10*std::log10(power) : -90.0;
    }
    void recomputeIntegrated() {
        const size_t n=std::min(blocks, energies.size());
        if (!n) { integrated=-90; return; }
        double absoluteSum=0;
        for(size_t i=0;i<n;++i) absoluteSum+=energies[i];
        const double mean=absoluteSum/n;
        const double relativeThreshold=mean*.1; // 10 LU below absolute-gated mean.
        double gatedSum=0; size_t gatedCount=0;
        for(size_t i=0;i<n;++i) if(energies[i]>=relativeThreshold) {
            gatedSum+=energies[i]; ++gatedCount;
        }
        integrated=gatedCount?toLufs(gatedSum/gatedCount):-90;
    }
    std::vector<double> ring400,ring3000,energies;
    size_t count400=0,count3000=0,hop=0,pos400=0,pos3000=0,frames=0,blocks=0;
    int channels=2;
    double rate=48000,sum400=0,sum3000=0,momentary=-90,shortTerm=-90,integrated=-90;
};
}
