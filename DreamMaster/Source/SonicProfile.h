#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <vector>

namespace dream {
struct SonicFeatures {
    std::array<double,9> spectral{};
    double integratedLufs=-90,truePeakDbtp=-90,rmsDbfs=-90,correlation=0;
};
struct SonicProfile {
    std::array<double,9> tonalMedian{},tonalMad{};
    std::array<int,9> tonalObservations{};
    double integratedLufs=-90,truePeakDbtp=-90,rmsDbfs=-90,correlation=0;
    int referenceCount=0;
};
// Robust descriptive profile only: never converts a profile into EQ controls.
class SonicProfileBuilder {
public:
    bool add(const SonicFeatures& f) {
        if(!std::isfinite(f.integratedLufs)||f.integratedLufs<=-90||f.integratedLufs>10
            ||!std::isfinite(f.truePeakDbtp)||f.truePeakDbtp<-90||f.truePeakDbtp>30
            ||!std::isfinite(f.rmsDbfs)||f.rmsDbfs<-90||f.rmsDbfs>30
            ||!std::isfinite(f.correlation)||std::abs(f.correlation)>1.001)return false;
        for(double x:f.spectral)if(!std::isfinite(x)||x<-300||x>120)return false;
        // A copied analysis report must not count as another independent master.
        // Exact full-feature duplicates are rejected, even under different filenames.
        for(const auto& earlier:data)
            if(earlier.spectral==f.spectral && earlier.integratedLufs==f.integratedLufs
                && earlier.truePeakDbtp==f.truePeakDbtp && earlier.rmsDbfs==f.rmsDbfs
                && earlier.correlation==f.correlation)return false;
        data.push_back(f);return true;
    }
    SonicProfile build() const {
        SonicProfile p;p.referenceCount=static_cast<int>(data.size());
        if(data.empty())return p;
        for(int b=0;b<9;++b) {
            std::vector<double> values;
            for(const auto& f:data)if(f.spectral[b]>-89)values.push_back(f.spectral[b]);
            p.tonalObservations[b]=static_cast<int>(values.size());
            p.tonalMedian[b]=values.empty()?std::numeric_limits<double>::quiet_NaN():median(values);
            for(double& value:values)value=std::abs(value-p.tonalMedian[b]);
            p.tonalMad[b]=values.empty()?std::numeric_limits<double>::quiet_NaN():median(values);
        }
        p.integratedLufs=featureMedian([](const SonicFeatures& f){return f.integratedLufs;});
        p.truePeakDbtp=featureMedian([](const SonicFeatures& f){return f.truePeakDbtp;});
        p.rmsDbfs=featureMedian([](const SonicFeatures& f){return f.rmsDbfs;});
        p.correlation=featureMedian([](const SonicFeatures& f){return f.correlation;});
        return p;
    }
private:
    static double median(std::vector<double>& values) {
        if(values.empty())return std::numeric_limits<double>::quiet_NaN();
        std::sort(values.begin(),values.end());
        const auto n=values.size();return n%2?values[n/2]:(values[n/2-1]+values[n/2])*.5;
    }
    template<class Getter>double featureMedian(Getter get) const {
        std::vector<double> values;for(const auto& f:data)values.push_back(get(f));
        return median(values);
    }
    std::vector<SonicFeatures> data;
};
}
