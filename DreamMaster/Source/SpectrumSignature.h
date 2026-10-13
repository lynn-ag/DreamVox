#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <cstddef>
#include <vector>

namespace dream {
// Broad, read-only tonal signature: 8192-point Hann FFT, 50% overlap.
// Averaged power per bin in physical-Hz bands. No EQ action is inferred.
class SpectrumSignature {
public:
 static constexpr int bandCount=9;
 static constexpr std::array<double,10> edgesHz={30,80,160,315,630,1250,2500,5000,10000,20000};
 void prepare(double sampleRate,int numChannels) {
  rate=std::max(8000.0,sampleRate);channels=std::clamp(numChannels,1,2);
  ring={};powerSum={};powerCount={};cursor=seen=sinceLast=0;windows=0;
  fftBuffer.assign(fftSize,{});
  for(int i=0;i<fftSize;++i)window[i]=.5-.5*std::cos(2*pi*i/(fftSize-1));
 }
 void process(const float* const* audio,int numChannels,int frames) {
  if(frames<=0||numChannels<1||!audio||fftBuffer.size()!=fftSize)return;
  const int nc=std::min(channels,numChannels);
  for(int n=0;n<frames;++n) {
   for(int ch=0;ch<nc;++ch){double x=audio[ch][n];if(!std::isfinite(x))x=0;ring[ch][cursor]=static_cast<float>(x);}
   for(int ch=nc;ch<2;++ch)ring[ch][cursor]=0;
   cursor=(cursor+1)%fftSize;if(seen<fftSize)++seen;++sinceLast;
   if(seen==fftSize&&sinceLast>=fftSize/2){analyze(nc);sinceLast=0;}
  }
 }
 bool valid() const {return windows>0;}
 bool hasBand(int i) const {return i>=0&&i<bandCount&&powerCount[static_cast<size_t>(i)]>0;}
 std::array<double,bandCount> relativeBandsDb() const {
  std::array<double,bandCount> output{};
  double sum=0;unsigned long long count=0;
  for(int i=0;i<bandCount;++i){sum+=powerSum[i];count+=powerCount[i];}
  const double baseline=count?sum/static_cast<double>(count):0;
  for(int i=0;i<bandCount;++i){const double p=powerCount[i]?powerSum[i]/powerCount[i]:0;
   output[i]=baseline>1e-30&&p>1e-30?10*std::log10(p/baseline):-90;}
  return output;
 }
private:
 static constexpr int fftSize=8192;
 static constexpr double pi=3.1415926535897932384626433832795;
 void fft(){
  for(int i=1,j=0;i<fftSize;++i){int bit=fftSize>>1;for(;j&bit;bit>>=1)j^=bit;j^=bit;
   if(i<j)std::swap(fftBuffer[i],fftBuffer[j]);}
  for(int len=2;len<=fftSize;len*=2){
   const std::complex<double> angle(std::cos(-2*pi/len),std::sin(-2*pi/len));
   for(int i=0;i<fftSize;i+=len){std::complex<double> rotation(1,0);
    for(int j=0;j<len/2;++j){const auto a=fftBuffer[i+j],b=fftBuffer[i+j+len/2]*rotation;
     fftBuffer[i+j]=a+b;fftBuffer[i+j+len/2]=a-b;rotation*=angle;}
   }
  }
 }
 void analyze(int nc) {
  ++windows;
  for(int ch=0;ch<nc;++ch) {
   for(int i=0;i<fftSize;++i)fftBuffer[i]={ring[ch][(cursor+i)%fftSize]*window[i],0};
   fft();
   for(int k=1;k<fftSize/2;++k){
    const double hz=static_cast<double>(k)*rate/fftSize;
    if(hz<edgesHz.front()||hz>=edgesHz.back())continue;
    for(int b=0;b<bandCount;++b)if(hz>=edgesHz[b]&&hz<edgesHz[b+1]){
     powerSum[b]+=std::norm(fftBuffer[k]);++powerCount[b];break;
    }
   }
  }
 }
 std::array<std::array<float,fftSize>,2> ring{};
 std::array<double,fftSize> window{};
 std::vector<std::complex<double>> fftBuffer;
 std::array<double,bandCount> powerSum{};
 std::array<unsigned long long,bandCount> powerCount{};
 int channels=2,cursor=0,seen=0,sinceLast=0;
 size_t windows=0;
 double rate=48000;
};
}
