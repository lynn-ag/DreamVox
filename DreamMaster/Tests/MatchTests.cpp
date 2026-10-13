#include "../Source/SonicMatch.h"
#include <cmath>
#include <iostream>
#include <stdexcept>
static void check(bool ok,const char* s){if(!ok)throw std::runtime_error(s);}
int main(){try{
 dream::SonicProfile p;
 p.referenceCount=5;p.tonalMedian.fill(0);p.tonalMad.fill(0);p.tonalObservations.fill(5);
 p.integratedLufs=-16;p.correlation=.7;
 dream::SonicFeatures f;f.spectral.fill(0);f.integratedLufs=-18;f.correlation=.5;
 const auto same=dream::SonicMatcher::compare(p,f);
 check(same.valid&&same.comparableBands==9,"Valid profile");
 check(std::abs(same.loudnessDifferenceLu-2)<1e-10,"Loudness difference");
 for(const auto& band:same.bands)check(band.comparable&&band.hintDb==0,"No false advice for matching spectrum");
 f.spectral[3]=-10;auto r=dream::SonicMatcher::compare(p,f);
 check(r.bands[3].hintDb>0&&r.bands[3].hintDb<=1.5,"Positive advice bounded");
 f.spectral[4]=-.5;r=dream::SonicMatcher::compare(p,f);
 check(r.bands[4].hintDb==0,"Dead zone suppresses small difference");
 f.spectral[5]=-10;p.tonalMad[5]=9;r=dream::SonicMatcher::compare(p,f);
 check(r.bands[5].hintDb==0,"Reference uncertainty suppresses advice");
 p.tonalObservations[6]=1;f.spectral[6]=-10;r=dream::SonicMatcher::compare(p,f);
 check(!r.bands[6].comparable,"Insufficient reference support");
 f.spectral[7]=-90;r=dream::SonicMatcher::compare(p,f);
 check(!r.bands[7].comparable,"Near-silent band not compared");
 f.spectral[3]=90;r=dream::SonicMatcher::compare(p,f);
 check(r.bands[3].hintDb>=-1.5,"Negative advice bounded");
 p.referenceCount=1;check(!dream::SonicMatcher::compare(p,f).valid,"Two references required");
 std::cout<<"PASS Sonic DNA bounds, uncertainty, no-change and missing-band cases\n";
 return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<"\n";return 1;}
}
