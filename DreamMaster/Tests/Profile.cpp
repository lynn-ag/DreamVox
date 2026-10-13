#include "../Source/SonicProfile.h"
#include <juce_core/juce_core.h>
#include <cmath>
#include <iostream>
#include <set>
#include <stdexcept>
#include <string>

// Creates a read-only descriptive sound profile from two or more local JSON
// references. It does not infer mastering parameters or upload audio.
static double number(const juce::DynamicObject& obj,const char* key)
{
    const auto value=obj.getProperty(juce::Identifier(key));
    if(!(value.isDouble()||value.isInt()||value.isInt64()))
        throw std::runtime_error(std::string("Missing numeric feature: ")+key);
    const auto result=static_cast<double>(value);
    if(!std::isfinite(result))throw std::runtime_error("Nonfinite analysis feature");
    return result;
}
int main(int argc,char** argv)
{
    if(argc<3) {
        std::cerr<<"Usage: DreamMasterProfile master1.dreammaster-analysis.json master2.dreammaster-analysis.json [more]\n";
        return 1;
    }
    try {
        std::set<std::string> seen;
        dream::SonicProfileBuilder builder;
        for(int i=1;i<argc;++i) {
            const auto file=juce::File::getCurrentWorkingDirectory().getChildFile(juce::String::fromUTF8(argv[i]));
            if(!file.existsAsFile())throw std::runtime_error("Analysis file not found");
            if(!seen.insert(file.getFullPathName().toLowerCase().toStdString()).second)
                throw std::runtime_error("Duplicate analysis file");
            const auto root=juce::JSON::parse(file.loadFileAsString());
            const auto* obj=root.getDynamicObject();
            if(!obj||obj->getProperty(juce::Identifier("schema")).toString()!="dreammaster.analysis.v1")
                throw std::runtime_error("Invalid or incompatible reference analysis");
            if(!static_cast<bool>(obj->getProperty(juce::Identifier("loudness_valid")))
                 ||!static_cast<bool>(obj->getProperty(juce::Identifier("spectral_valid"))))
                throw std::runtime_error("Reference too short for valid spectral/loudness features");
            auto bandValue=obj->getProperty(juce::Identifier("spectral_bands_db"));
            const auto* array=bandValue.getArray();
            if(!array||array->size()!=9)throw std::runtime_error("Missing 9-band spectrum");
            dream::SonicFeatures f;
            for(int b=0;b<9;++b) {
                const auto& v=array->getReference(b);
                if(!v.isVoid()&&!(v.isDouble()||v.isInt()||v.isInt64()))
                    throw std::runtime_error("Non-numeric spectral feature");
                f.spectral[b]=v.isVoid()?-90:static_cast<double>(v);
            }
            f.integratedLufs=number(*obj,"integrated_lufs");
            f.truePeakDbtp=number(*obj,"max_true_peak_dbtp");
            f.rmsDbfs=number(*obj,"rms_dbfs");
            f.correlation=number(*obj,"stereo_correlation");
            if(!builder.add(f))throw std::runtime_error("Invalid or unsafe analysis metrics");
        }
        const auto p=builder.build();
        juce::var result(new juce::DynamicObject());
        auto* output=result.getDynamicObject();
        output->setProperty("schema","dreammaster.sonic-dna.v1");
        output->setProperty("reference_count",p.referenceCount);
        output->setProperty("profile_type","descriptive_median_not_automatic_eq");
        output->setProperty("median_integrated_lufs",p.integratedLufs);
        output->setProperty("median_true_peak_dbtp",p.truePeakDbtp);
        output->setProperty("median_rms_dbfs",p.rmsDbfs);
        output->setProperty("median_stereo_correlation",p.correlation);
        juce::Array<juce::var> bandValues,variability,counts,edges;
        for(int hz:{30,80,160,315,630,1250,2500,5000,10000,20000})edges.add(hz);
        for(int b=0;b<9;++b) {
            if(p.tonalObservations[b]>0) {
                bandValues.add(p.tonalMedian[b]);variability.add(p.tonalMad[b]);
            } else {bandValues.add(juce::var());variability.add(juce::var());}
            counts.add(p.tonalObservations[b]);
        }
        output->setProperty("tonal_band_edges_hz",juce::var(edges));
        output->setProperty("tonal_median_db",juce::var(bandValues));
        output->setProperty("tonal_mad_db",juce::var(variability));
        output->setProperty("tonal_valid_counts",juce::var(counts));
        std::cout<<juce::JSON::toString(result,true).toStdString()<<"\n";
        return 0;
    }catch(const std::exception& e) {
        std::cerr<<"Profile creation failed: "<<e.what()<<"\n";return 1;
    }
}
