#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

namespace dream {
// Streaming 4x oversampled output true-peak ESTIMATOR using a Blackman-windowed
// 48-tap bandlimited interpolator. Useful for measurement; NOT a limiter.
// 24-sample analysis delay. 4x measurements must be independently validated.
class TruePeakMeter {
public:
    void prepare(double sampleRate) {
        rate=std::max(8000.0,sampleRate);
        for (int phase=0;phase<4;++phase) {
            double norm=0;
            for (int j=0;j<taps;++j) {
                const double distance=j-center+phase*.25;
                const double normalized=(j+.5)/(double)taps;
                const double window=.42-.5*std::cos(2*pi*normalized)+.08*std::cos(4*pi*normalized);
                const double sinc=std::abs(distance)<1.e-12?1:std::sin(pi*distance)/(pi*distance);
                coeff[phase][j]=window*sinc;
                norm+=coeff[phase][j];
            }
            for (double& c:coeff[phase]) c/=norm;
        }
        history={}; pos=0; frames=0; current=0;
    }
    void process(const float* const* data,int channelCount,int samples) {
        if (samples<=0 || channelCount<=0 || !data) return;
        const int nc=std::min(2,channelCount);
        const double decay=std::exp(-samples/(rate*.6));
        double peak=0;
        for (int n=0;n<samples;++n) {
            for(int ch=0;ch<nc;++ch) {
                double x=data[ch][n]; if(!std::isfinite(x)) x=0;
                history[ch][pos]=x;
                peak=std::max(peak,std::abs(x));
            }
            pos=(pos+1)%taps;
            ++frames;
            if(frames<static_cast<size_t>(taps)) continue;
            for(int ch=0;ch<nc;++ch)for(int phase=0;phase<4;++phase) {
                double y=0;
                for(int j=0;j<taps;++j) {
                    const int index=(pos+taps-1-j)%taps;
                    y+=coeff[phase][j]*history[ch][index];
                }
                peak=std::max(peak,std::abs(y));
            }
        }
        current=std::max(peak,current*decay);
    }
    double peakLinear() const { return current; }
    double peakDbtp() const { return 20*std::log10(std::max(1.e-9,current)); }
private:
    static constexpr int taps=48,center=24;
    static constexpr double pi=3.1415926535897932384626433832795;
    std::array<std::array<double,taps>,4> coeff{};
    std::array<std::array<double,taps>,2> history{};
    int pos=0; size_t frames=0;
    double rate=48000,current=0;
};
}
