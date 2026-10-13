#include "../Source/SonicMatch.h"
#include <juce_core/juce_core.h>
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
static double num(const juce::DynamicObject& o,const char* key){
 const auto v=o.getProperty(juce::Identifier(key));
 if(!(v.isInt()||v.isInt64()||v.isDouble()))throw std::runtime_error(std::string("Missing numeric: ")+key);
 const double x=static_cast<double>(v);if(!std::isfinite(x))throw std::runtime_error("Nonfinite metric");return x;
}
static juce::var read(const char* name,const char* schema){
 const auto f=juce::File::getCurrentWorkingDirectory().getChildFile(juce::String::fromUTF8(name));
 if(!f.existsAsFile())throw std::runtime_error("Input JSON not found");
 const auto v=juce::JSON::parse(f.loadFileAsString());
 if(!v.isObject()||v.getDynamicObject()->getProperty(juce::Identifier("schema")).toString()!=schema)
  throw std::runtime_error("Unsupported JSON schema");
 return v;
}
static std::array<double,9> bands(const juce::DynamicObject& o,const char* key,bool nullable){
 const auto v=o.getProperty(juce::Identifier(key));const auto* arr=v.getArray();
 if(!arr||arr->size()!=9)throw std::runtime_error(std::string("Missing 9-band array: ")+key);
 std::array<double,9> out{};
 for(int i=0;i<9;++i){
  const auto& x=arr->getReference(i);
  if(nullable&&x.isVoid()){out[i]=std::numeric_limits<double>::quiet_NaN();continue;}
  if(!(x.isInt()||x.isInt64()||x.isDouble()))throw std::runtime_error("Non-numeric band");
  out[i]=static_cast<double>(x);if(!std::isfinite(out[i]))throw std::runtime_error("Nonfinite band");
 }
 return out;
}
int main(int argc,char** argv){
 if(argc!=3){std::cerr<<"Usage: DreamMasterMatch SonicDNA.json song.dreammaster-analysis.json\n";return 1;}
 try{
  auto profileJson=read(argv[1],"dreammaster.sonic-dna.v1");
  auto mixJson=read(argv[2],"dreammaster.analysis.v1");
  const auto& a=*profileJson.getDynamicObject();const auto& b=*mixJson.getDynamicObject();
  if(!static_cast<bool>(b.getProperty(juce::Identifier("spectral_valid")))
     ||!static_cast<bool>(b.getProperty(juce::Identifier("loudness_valid"))))
   throw std::runtime_error("Mix too short for valid analysis");
  dream::SonicProfile p;
  const double n=num(a,"reference_count");
  if(n<2||n>100000||std::floor(n)!=n)throw std::runtime_error("At least two references required");
  p.referenceCount=static_cast<int>(n);
  p.integratedLufs=num(a,"median_integrated_lufs");
  p.correlation=num(a,"median_stereo_correlation");
  p.tonalMedian=bands(a,"tonal_median_db",true);
  p.tonalMad=bands(a,"tonal_mad_db",true);
  const auto counts=bands(a,"tonal_valid_counts",false);
  for(int i=0;i<9;++i){
   if(counts[i]<0||counts[i]>p.referenceCount||std::floor(counts[i])!=counts[i])
    throw std::runtime_error("Invalid per-band reference count");
   p.tonalObservations[i]=static_cast<int>(counts[i]);
  }
  dream::SonicFeatures mix;
  mix.integratedLufs=num(b,"integrated_lufs");
  mix.correlation=num(b,"stereo_correlation");
  mix.spectral=bands(b,"spectral_bands_db",true);
  for(double& x:mix.spectral)if(!std::isfinite(x))x=-90;
  const auto result=dream::SonicMatcher::compare(p,mix);
  if(!result.valid)throw std::runtime_error("Profile/mix are not comparable");
  juce::var out(new juce::DynamicObject());auto* obj=out.getDynamicObject();
  obj->setProperty("schema","dreammaster.match-advisory.v1");
  obj->setProperty("mode","read_only_reference_analysis_not_eq_settings");
  obj->setProperty("profile_reference_count",p.referenceCount);
  obj->setProperty("comparable_band_count",result.comparableBands);
  obj->setProperty("reference_minus_mix_lufs",result.loudnessDifferenceLu);
  obj->setProperty("reference_minus_mix_stereo_correlation",result.correlationDifference);
  juce::Array<juce::var> differences,hints,confidence;
  for(const auto& band:result.bands){
   differences.add(band.comparable?juce::var(band.differenceDb):juce::var());
   hints.add(band.comparable?juce::var(band.hintDb):juce::var());
   confidence.add(juce::var(band.confidence));
  }
  obj->setProperty("band_difference_db",juce::var(differences));
  obj->setProperty("conservative_tonal_hint_db",juce::var(hints));
  obj->setProperty("band_confidence_0_to_1",juce::var(confidence));
  obj->setProperty("caution","Descriptive suggestions only. Different musical arrangements may justify different balances. No audio processing was applied.");
  std::cout<<juce::JSON::toString(out,true).toStdString()<<"\n";return 0;
 }catch(const std::exception& e){std::cerr<<"Match analysis failed: "<<e.what()<<"\n";return 1;}
}
