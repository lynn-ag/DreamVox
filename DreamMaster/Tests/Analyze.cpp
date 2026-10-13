#include "../Source/LoudnessMeter.h"
#include "../Source/TruePeakMeter.h"
#include "../Source/SpectrumSignature.h"
#include <juce_audio_formats/juce_audio_formats.h>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>

// Read-only, offline full-file analyzer. Shares the exact same metering
// implementation as the plug-in; never uploads or modifies the reference.
int main(int argc,char** argv)
{
    if(argc!=2) {
        std::cerr<<"Usage: DreamMasterAnalyze reference.wav\n"
                 <<"Supports audio formats provided by JUCE's basic readers.\n";
        return 1;
    }
    try {
        juce::AudioFormatManager formats;
        formats.registerBasicFormats();
        const auto file=juce::File::getCurrentWorkingDirectory().getChildFile(
            juce::String::fromUTF8(argv[1]));
        if(!file.existsAsFile())throw std::runtime_error("Audio file does not exist");
        std::unique_ptr<juce::AudioFormatReader> reader(formats.createReaderFor(file));
        if(!reader)throw std::runtime_error("Unsupported or unreadable audio file");
        const double rate=reader->sampleRate;
        const int nc=static_cast<int>(std::min(2u,reader->numChannels));
        if(rate<8000||rate>384000||nc<1||reader->numChannels>2||reader->lengthInSamples<=0)
            throw std::runtime_error("Unsupported sample rate, channel count or length");
        dream::LoudnessMeter loudness;
        loudness.prepare(rate,nc);
        dream::TruePeakMeter truePeak;
        truePeak.prepare(rate);
        dream::SpectrumSignature signature;
        signature.prepare(rate,nc);
        juce::AudioBuffer<float> audio(2,4096);
        double energy=0,peak=0,sumL=0,sumR=0,cross=0;
        int64_t frames=0;
        double maximumTruePeak=-90;
        while(frames<reader->lengthInSamples)
        {
            const int n=static_cast<int>(std::min<int64_t>(4096,reader->lengthInSamples-frames));
            audio.clear();
            if(!reader->read(&audio,0,n,frames,true,nc>1))
                throw std::runtime_error("Decoder failed while reading audio");
            const auto* const* samples=audio.getArrayOfReadPointers();
            loudness.process(samples,nc,n);
            truePeak.process(samples,nc,n);
            signature.process(samples,nc,n);
            maximumTruePeak=std::max(maximumTruePeak,truePeak.peakDbtp());
            for(int i=0;i<n;++i) {
                const double l=std::isfinite(samples[0][i])?samples[0][i]:0;
                const double r=nc>1&&std::isfinite(samples[1][i])?samples[1][i]:0;
                peak=std::max(peak,std::max(std::abs(l),std::abs(r)));
                energy+=l*l+r*r;sumL+=l*l;sumR+=r*r;cross+=l*r;
            }
            frames+=n;
        }
        // Flush interpolation only; no additional samples enter the LUFS analysis.
        float zeros[64]{};const float* tail[]={zeros,zeros};
        truePeak.process(tail,nc,64);
        maximumTruePeak=std::max(maximumTruePeak,truePeak.peakDbtp());
        const double rms=std::sqrt(energy/std::max<double>(1,frames*nc));
        const double correlation=nc==2&&sumL>1e-20&&sumR>1e-20
            ?std::clamp(cross/std::sqrt(sumL*sumR),-1.0,1.0):0;
        std::cout<<std::fixed<<std::setprecision(4)
                 <<"{\"schema\":\"dreammaster.analysis.v1\","
                 <<"\"duration_seconds\":"<<frames/rate<<","
                 <<"\"sample_rate\":"<<rate<<","
                 <<"\"channels\":"<<nc<<","
                 <<"\"integrated_lufs\":"<<loudness.integratedLufs()<<","
                 <<"\"momentary_lufs\":"<<loudness.momentaryLufs()<<","
                 <<"\"short_term_lufs\":"<<loudness.shortTermLufs()<<","
                 <<"\"max_true_peak_dbtp\":"<<maximumTruePeak<<","
                 <<"\"sample_peak_dbfs\":"<<20*std::log10(std::max(1e-9,peak))<<","
                 <<"\"rms_dbfs\":"<<20*std::log10(std::max(1e-9,rms))<<","
                 <<R"("stereo_correlation":)"<<correlation<<",";
        const auto bands=signature.relativeBandsDb();
        std::cout<<R"("spectral_band_edges_hz":[30,80,160,315,630,1250,2500,5000,10000,20000],)";
        std::cout<<R"("spectral_bands_db":[)";
        for(int i=0;i<dream::SpectrumSignature::bandCount;++i){
            if(i)std::cout<<",";
            if(signature.valid()&&signature.hasBand(i))std::cout<<bands[i];
            else std::cout<<"null";
        }
        std::cout<<R"(],"spectral_valid":)"<<(signature.valid()?"true":"false")<<","
                 <<R"("loudness_valid":)"<<(frames>=static_cast<int64_t>(rate*.4)?"true":"false")
                 <<"}\n";
        return 0;
    } catch(const std::exception& e) {
        std::cerr<<"Analysis failed: "<<e.what()<<"\n";
        return 1;
    }
}
