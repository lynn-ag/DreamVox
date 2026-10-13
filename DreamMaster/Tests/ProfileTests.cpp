#include "../Source/SonicProfile.h"
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <limits>
static void check(bool v){if(!v)throw std::runtime_error("robust profile test failed");}
int main() {
 dream::SonicProfileBuilder b;dream::SonicFeatures a,c,d,e;
 for(double& v:a.spectral)v=1;
 for(double& v:c.spectral)v=2;
 for(double& v:d.spectral)v=2;
 for(double& v:e.spectral)v=20;
 a.integratedLufs=-16;c.integratedLufs=-15;d.integratedLufs=-15;e.integratedLufs=-6;
 c.rmsDbfs=-25;d.rmsDbfs=-24; // Different musical references can share tonal signature.
 e.spectral[8]=-90;
 check(b.add(a)&&b.add(c)&&b.add(d)&&b.add(e));
 check(!b.add(a)); // Copied analysis is not another independent reference.
 const auto p=b.build();
 check(p.referenceCount==4);
 check(std::abs(p.tonalMedian[0]-2)<1e-12);
 check(std::abs(p.tonalMad[0]-.5)<1e-12);
 check(p.tonalObservations[8]==3);
 check(std::abs(p.integratedLufs+15)<1e-12);
 dream::SonicFeatures nearSilent=a;nearSilent.spectral[2]=-160;
 check(b.add(nearSilent));
 check(b.build().tonalObservations[2]==4); // Missing/near-silent band never skews its median.
 dream::SonicFeatures bad=a;bad.correlation=2.0;
 check(!b.add(bad));
 bad=a;bad.spectral[0]=std::numeric_limits<double>::quiet_NaN();
 check(!b.add(bad));
 std::cout<<"PASS robust median, MAD, unavailable high band, invalid input and loudness\n";
}
