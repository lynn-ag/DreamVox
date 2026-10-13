#include "../Source/Presets.h"
#include "../Source/LoudnessMeter.h"
#include "../Source/TruePeakMeter.h"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
// Local test utility: interleaved stereo float32 input/output, supplied sample rate.
int main(int argc,char** argv)
{
    if(argc!=6) { std::cerr<<"Usage: DreamMasterRender input.f32 output.f32 sampleRate presetIndex driveDb\n";return 1; }
    try {
        std::ifstream input(argv[1],std::ios::binary);std::ofstream output(argv[2],std::ios::binary);
        if(!input||!output)throw std::runtime_error("Cannot open audio file");
        const auto sr=std::stod(argv[3]);if(sr<8000||sr>384000)throw std::runtime_error("Unsupported sample rate");
        dream::MasterEngine e;e.prepare(sr);auto s=dream::preset(std::stoi(argv[4]));s.drive=std::clamp(std::stod(argv[5]),0.0,9.0);
        dream::LoudnessMeter loudness;loudness.prepare(sr,2);
        dream::TruePeakMeter truePeak;truePeak.prepare(sr);
        const int block=512;float interleaved[block*2],l[block],r[block];float* p[]={l,r};
        int skip=e.latency();uint64_t frames=0;double peak=0,maxGR=0,maxTP=-90;
        auto render=[&](int n)
        {
            e.process(p,2,n,s);maxGR=std::max(maxGR,e.limitReduction);
            const int remove=std::min(skip,n);
            skip-=remove;
            if(n>remove) {
                const float* measured[]={l+remove,r+remove};
                loudness.process(measured,2,n-remove);
                truePeak.process(measured,2,n-remove);
                maxTP=std::max(maxTP,truePeak.peakDbtp());
            }
            for(int i=remove;i<n;++i) {
                float pair[]={l[i],r[i]};output.write(reinterpret_cast<const char*>(pair),sizeof(pair));
                peak=std::max({peak,std::abs(static_cast<double>(l[i])),std::abs(static_cast<double>(r[i]))});
            }
        };
        while(input.read(reinterpret_cast<char*>(interleaved),sizeof(interleaved))||input.gcount())
        {
            if(input.gcount()%8)throw std::runtime_error("Incomplete stereo frame");
            const int n=static_cast<int>(input.gcount()/8);frames+=static_cast<uint64_t>(n);
            for(int i=0;i<n;++i){l[i]=interleaved[i*2];r[i]=interleaved[i*2+1];}render(n);
        }
        for(int remain=e.latency();remain>0;)
        {const int n=std::min(block,remain);std::fill_n(l,n,0);std::fill_n(r,n,0);render(n);remain-=n;}
        // Include true-peak interpolation at the very end of the exported audio.
        float zeros[64]{};const float* tail[]={zeros,zeros};truePeak.process(tail,2,64);
        maxTP=std::max(maxTP,truePeak.peakDbtp());
        if(!output)throw std::runtime_error("Audio write failed");
        if(maxTP>s.ceiling+.2)
            std::cerr<<"WARNING: reconstructed true peak is above the selected sample-peak ceiling by "
                     <<(maxTP-s.ceiling)<<" dB; this is not yet a true-peak limiter.\n";
        std::cout<<"frames="<<frames<<" peak_dBFS="<<dream::db(peak)<<" max_limit_reduction_dB="<<maxGR
                 <<" integrated_LUFS="<<loudness.integratedLufs()
                 <<" max_true_peak_dBTP="<<maxTP<<"\n";return 0;
    } catch(const std::exception& ex){std::cerr<<ex.what()<<"\n";return 1;}
}
