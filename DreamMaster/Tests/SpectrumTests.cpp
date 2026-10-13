#include "../Source/SpectrumSignature.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <vector>
static void check(bool cond){if(!cond)throw std::runtime_error("spectrum test failed");}
int main(){for(int sr:{44100,48000,96000}){
 dream::SpectrumSignature spec;spec.prepare(sr,2);
 std::vector<float> l(sr),r(sr);
 constexpr double pi=3.141592653589793;
 for(int i=0;i<sr;++i){l[i]=.8f*std::sin(2*pi*440*i/sr);r[i]=-l[i];}
 for(int i=0;i<sr;i+=257){const float* p[]={l.data()+i,r.data()+i};spec.process(p,2,std::min(257,sr-i));}
 const auto bands=spec.relativeBandsDb();check(spec.valid());
 dream::SpectrumSignature alt;alt.prepare(sr,2);
 for(int i=0;i<sr;i+=4096){const float* q[]={l.data()+i,r.data()+i};alt.process(q,2,std::min(4096,sr-i));}
 const auto again=alt.relativeBandsDb();
 for(int b=0;b<dream::SpectrumSignature::bandCount;++b)check(std::abs(bands[b]-again[b])<1e-9);
 check(bands[3]>bands[1]+20&&bands[3]>bands[6]+25);
 check(std::isfinite(bands[3]));
 std::cout<<sr<<"Hz 440Hz band="<<bands[3]<<"dB\n";
 }
 dream::SpectrumSignature silence;silence.prepare(48000,1);
 float zeros[4096]{};const float* ptr[]={zeros};silence.process(ptr,1,4096);
 check(!silence.valid());
 std::cout<<"ALL SPECTRAL TESTS PASSED\n";
}
