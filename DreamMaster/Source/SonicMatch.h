#pragma once
#include "SonicProfile.h"
#include <algorithm>
#include <array>
#include <cmath>
namespace dream {
// Advisory analysis ONLY: these values are not EQ coefficients.
struct SonicMatchBand {bool comparable=false;double differenceDb=0,hintDb=0,confidence=0;};
struct SonicMatchResult {
 std::array<SonicMatchBand,9> bands{};
 double loudnessDifferenceLu=0,correlationDifference=0;
 int comparableBands=0;bool valid=false;
};
class SonicMatcher {
public:
 static SonicMatchResult compare(const SonicProfile& p,const SonicFeatures& f) {
  SonicMatchResult r;
  if(p.referenceCount<2||!std::isfinite(p.integratedLufs)||p.integratedLufs<=-90
     ||!std::isfinite(f.integratedLufs)||f.integratedLufs<=-90||f.integratedLufs>10
     ||!std::isfinite(p.correlation)||!std::isfinite(f.correlation))return r;
  r.valid=true;r.loudnessDifferenceLu=p.integratedLufs-f.integratedLufs;
  r.correlationDifference=std::clamp(p.correlation-f.correlation,-2.,2.);
  for(int b=0;b<9;++b) {
   const int n=p.tonalObservations[b];
   const double median=p.tonalMedian[b],mad=p.tonalMad[b],mix=f.spectral[b];
   if(n<2||n>p.referenceCount||!std::isfinite(median)||!std::isfinite(mad)
      ||mad<0||!std::isfinite(mix)||median<=-89||mix<=-89)continue;
   auto& out=r.bands[b];out.comparable=true;out.differenceDb=median-mix;
   const double support=std::min(1.,n/3.);
   const double coverage=static_cast<double>(n)/p.referenceCount;
   const double stability=1./(1.+mad/3.);
   out.confidence=std::clamp(support*coverage*stability,0.,1.);
   const double excess=std::max(0.,std::abs(out.differenceDb)-std::max(1.25,1.4826*mad));
   out.hintDb=excess>0?std::copysign(std::min(1.5,excess*.30*out.confidence),out.differenceDb):0;
   ++r.comparableBands;
  }
  return r;
 }
};
}
