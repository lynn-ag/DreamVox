#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

namespace dream {
// Pre-limiter 8x bandlimited predictor. Windowed-sinc interpolation operates
// on incoming stereo-linked audio; no allocation in push().
// Prediction is delayed by 24 input samples, so use only with sufficiently
// long lookahead (the mastering engine has >=40 samples at 8 kHz).
class TruePeakDetector {
public:
    void prepare() {
        for (int phase=0;phase<oversampling;++phase) {
            double norm=0;
            for (int tap=0;tap<taps;++tap) {
                const double d=tap-center+static_cast<double>(phase)/oversampling;
                const double w=.42-.5*std::cos(2*pi*(tap+.5)/taps)
                                 +.08*std::cos(4*pi*(tap+.5)/taps);
                const double sinc=std::abs(d)<1.e-12?1:std::sin(pi*d)/(pi*d);
                coeff[phase][tap]=w*sinc;
                norm+=coeff[phase][tap];
            }
            for (double& c:coeff[phase]) c/=norm;
        }
        history={};pos=0;seen=0;
    }
    double push(double left,double right,int channels) {
        if(!std::isfinite(left))left=0;
        if(!std::isfinite(right))right=0;
        history[0][pos]=left;
        history[1][pos]=channels>1?right:0;
        pos=(pos+1)%taps;
        if(seen<taps)++seen;
        double peak=std::max(std::abs(left),channels>1?std::abs(right):0.0);
        if(seen<taps)return peak;
        const int nc=channels>1?2:1;
        for(int ch=0;ch<nc;++ch)for(int phase=0;phase<oversampling;++phase) {
            double reconstructed=0;
            for(int tap=0;tap<taps;++tap) {
                const int index=(pos+taps-1-tap)%taps;
                reconstructed+=coeff[phase][tap]*history[ch][index];
            }
            peak=std::max(peak,std::abs(reconstructed));
        }
        return peak;
    }
    static constexpr int analysisDelay=24;
private:
    static constexpr int taps=48,center=24,oversampling=8;
    static constexpr double pi=3.1415926535897932384626433832795;
    std::array<std::array<double,taps>,oversampling> coeff{};
    std::array<std::array<double,taps>,2> history{};
    int pos=0,seen=0;
};
}
