#include "PluginEditor.h"
#include <MasterArtwork.h>
namespace {
const char* ids[]={"input","bass","clarity","smooth","air","glue","width","drive","ceiling"};
const char* names[]={"Input","Bass","Clarity","Smooth","Air","Glue","Width","Loudness","Ceiling"};
const char* hints[]={"Input trim before processing.","Low shelf at 100 Hz.","Reduces low-mid buildup around 320 Hz.","Dynamic harshness reduction around 3.8 kHz.","High shelf at 10 kHz.","Stereo-linked compression, up to 3 dB reduction.","Stereo width above the low-bass region. 100% retains original width.","Gain into the limiter. Start at zero.","Sample-peak ceiling, not true peak. Leave export headroom."};
}
DreamMasterEditor::DreamMasterEditor(DreamMasterProcessor& p):AudioProcessorEditor(&p),processor(p) {
    setLookAndFeel(&look);artwork=juce::ImageCache::getFromMemory(MasterArtwork::MidnightHorizon_png,MasterArtwork::MidnightHorizon_pngSize);
    for(size_t i=0;i<controls.size();++i) {
        night::slider(controls[i],labels[i],*this,names[i],hints[i]);controls[i].getProperties().set("parameterId",ids[i]);
        attachments[i]=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters,ids[i],controls[i]);
        controls[i].setTextValueSuffix(i==2||i==3||i==5||i==6?" %":i==8?" dBFS":" dB");
        controls[i].setDoubleClickReturnValue(true,p.parameters.getParameter(ids[i])->convertFrom0to1(p.parameters.getParameter(ids[i])->getDefaultValue()));
    }
    presets.setComponentID("masteringPresets");
    presets.addSectionHeading("Original starting points");
    for(int i=0;i<5;++i)presets.addItem(dream::presetName(i),i+1);
    presets.addSeparator();presets.addSectionHeading("Suno starting points");
    for(int i=5;i<dream::presetCount;++i)presets.addItem(dream::presetName(i),i+1);
    presets.setTextWhenNothingSelected("Choose a mastering preset...");
    presets.setTooltip("Select a starting point. Adjust processing by ear.");
    presets.onChange=[this] {
        const int index=presets.getSelectedId()-1;
        if(index<0 || index>=dream::presetCount)return;
        processor.applyPreset(index);
        // Keep the selection visible inside the ComboBox like a normal preset menu.
        // The longer description stays available as a tooltip, not a separate banner.
        presets.setTooltip(juce::String(dream::presetName(index))+": "+dream::presetHint(index));
    };
    addAndMakeVisible(presets);addAndMakeVisible(bypass);addAndMakeVisible(lowCut);addAndMakeVisible(tpMode);addAndMakeVisible(zoom);
    bypass.setTooltip("Latency-matched bypass. Loudness is not automatically matched.");
    lowCut.setTooltip("Optional 22 Hz high-pass filter for sub-rumble.");
    tpMode.setTooltip("Optional 8x true-peak-informed lookahead limiting. Uses a conservative safety margin; reduces intersample overshoots but requires listening verification.");
    bypassAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.parameters,"bypass",bypass);
    cutAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.parameters,"lowcut",lowCut);
    tpAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.parameters,"truepeak",tpMode);
    zoom.addItemList({"80%","100%","125%"},1);zoom.setSelectedId(2,juce::dontSendNotification);
    zoom.onChange=[this]{const float scale=zoom.getSelectedId()==1?.8f:zoom.getSelectedId()==3?1.25f:1.f;setSize(int(1200*scale),int(780*scale));};
    setResizable(true,true);setResizeLimits(960,624,1500,975);getConstrainer()->setFixedAspectRatio(1200.0/780.0);
    setSize(1200,780);startTimerHz(20);
}
DreamMasterEditor::~DreamMasterEditor(){stopTimer();setLookAndFeel(nullptr);}
void DreamMasterEditor::resized() {
    presets.setBounds(425,65,376,28);bypass.setBounds(899,28,132,28);zoom.setBounds(1061,28,90,28);lowCut.setBounds(38,668,173,28);tpMode.setBounds(744,706,169,25);
    night::place(controls[0],labels[0],25,493,100);night::place(controls[3],labels[3],129,493,100);
    night::place(controls[1],labels[1],242,476,96,98);night::place(controls[4],labels[4],342,476,96,98);night::place(controls[2],labels[2],291,593,96,78);
    night::place(controls[5],labels[5],457,493,115,133);night::place(controls[6],labels[6],598,493,115,133);
    night::place(controls[7],labels[7],741,493,100);night::place(controls[8],labels[8],845,493,100);
    const float sx=getWidth()/1200.f,sy=getHeight()/780.f;
    for(auto* c:getChildren()) if(dynamic_cast<juce::ResizableCornerComponent*>(c)==nullptr){const auto r=c->getBounds();c->setBounds(juce::roundToInt(r.getX()*sx),juce::roundToInt(r.getY()*sy),juce::roundToInt(r.getWidth()*sx),juce::roundToInt(r.getHeight()*sy));}
}
void DreamMasterEditor::paint(juce::Graphics& g) {
    g.addTransform(juce::AffineTransform::scale(getWidth()/1200.f,getHeight()/780.f));
    night::background(g,artwork,look,"Midnight Horizon");
    night::panel(g,artwork,look,20,215,146,386,"CLEAN","INPUT & SMOOTHING");
    night::panel(g,artwork,look,245,193,316,386,"EQ","BALANCE");
    night::panel(g,artwork,look,448,132,481,386,"GLUE","COMPRESSION");
    night::panel(g,artwork,look,590,132,803,386,"SPACE","STEREO WIDTH");
    night::panel(g,artwork,look,732,220,1145,386,"LIMITER","LOUDNESS");
    night::panel(g,artwork,look,962,218,1340,386,"METER","LIVE LEVELS");
    night::text(g,"Gentle harshness control",31,646,194,20,10,night::muted);
    night::text(g,"Stereo linked\nUp to 3 dB reduction",452,653,124,44,10,night::muted);
    night::text(g,"Protected low end\n100% = original",594,653,124,44,10,night::muted);
    night::text(g,tpMode.getToggleState()?"8x true-peak predictor\n5 ms lookahead":"Sample-peak ceiling\n5 ms lookahead",739,653,205,44,11,night::muted);
    auto meter=[&](const char* label,float value,int y,bool reduction){
        if(!std::isfinite(value))value=-90.f;
        night::text(g,label,975,y,125,18,10,night::muted,juce::Justification::left);
        night::text(g,juce::String(juce::jmax(-90.f,value),1),1094,y,69,18,12,night::white,juce::Justification::right);
        g.setColour(juce::Colour(0xff222939));g.fillRoundedRectangle(976,float(y+22),187,5,2);
        g.setColour(reduction?look.accent:look.blue);const float amount=juce::jlimit(0.f,1.f,reduction?value/9.f:(value+60.f)/60.f);
        g.fillRoundedRectangle(976,float(y+22),187*amount,5,2);
    };
    meter("TRUE PEAK",processor.tpDb.load(),480,false);
    meter("SHORT LUFS",processor.shortLufs.load(),521,false);
    meter("INTEG LUFS",processor.integratedLufs.load(),562,false);
    meter("GLUE REDUCTION",processor.compDb.load(),603,true);
    meter("LIMIT REDUCTION",processor.limitDb.load(),644,true);
    night::text(g,"TP: dBTP / Loudness: LUFS / GR: dB",974,681,193,18,9,night::muted);
    const bool off=processor.parameters.getRawParameterValue("bypass")->load()>.5f;
    const float ceiling=processor.parameters.getRawParameterValue("ceiling")->load();
    const bool overshoot=processor.tpDb.load()>ceiling+.2f;
    const char* status=off?"BYPASSED  /  Original signal with matched latency":
        overshoot?"TRUE PEAK WARNING: intersample overshoot; current limiter is sample-peak only.":
        processor.limitDb.load()>3?"Heavy limiting: reduce Loudness and compare at a similar listening level.":
        "Suno: choose a starting point, adjust by ear, compare at a similar listening level.";
    night::footer(g,look,status,"DreamMaster  " JucePlugin_VersionString);
}
