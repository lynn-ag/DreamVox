#include "PluginEditor.h"
#include <VoxArtwork.h>
namespace {
const char* ids[]={"low","body","air","threshold","ratio","deess","warmth","output"};
const char* names[]={"Low","Body","Air","Threshold","Ratio","Amount","Warmth","Output"};
const char* hints[]={"Low shelf at 180 Hz.","Midrange body around 1.1 kHz.","High shelf at 9 kHz.","Compressor threshold in dB.","Compression ratio. 1:1 disables compression.","De-essing amount. 0% disables de-essing.","Saturation blend. 0% is clean.","Output trim after processing."};
}
DreamVoxAudioProcessorEditor::DreamVoxAudioProcessorEditor(DreamVoxAudioProcessor& p):AudioProcessorEditor(&p),processor(p) {
    setLookAndFeel(&look);artwork=juce::ImageCache::getFromMemory(VoxArtwork::VelvetNights_png,VoxArtwork::VelvetNights_pngSize);
    for(size_t i=0;i<controls.size();++i) {
        night::slider(controls[i],labels[i],*this,names[i],hints[i]);
        controls[i].getProperties().set("parameterId",ids[i]);
        attachments[i]=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters,ids[i],controls[i]);
        controls[i].setTextValueSuffix(i==4?" :1":(i==5||i==6)?" %":" dB");
        controls[i].setDoubleClickReturnValue(true,p.parameters.getParameter(ids[i])->convertFrom0to1(p.parameters.getParameter(ids[i])->getDefaultValue()));
    }
    presets.addItemList({"Original starting settings","Velvet vocal","Soft air","Clean tone"},1);
    presets.setTextWhenNothingSelected("Choose a vocal starting point...");
    presets.setTooltip("Replaces the eight tone/dynamics settings. Adjust by ear for each recording.");
    presets.onChange=[this] {
        const int index=presets.getSelectedId()-1;if(index<0)return;
        const float settings[][8]={{0,0,0,-18,2.5f,28,14,0},{-.5f,.5f,.5f,-18,2.5f,28,14,0},{-1,-.5f,1,-16,2,32,8,0},{0,0,0,0,1,0,0,0}};
        for(size_t i=0;i<controls.size();++i) {auto* parameter=processor.parameters.getParameter(ids[i]);parameter->beginChangeGesture();parameter->setValueNotifyingHost(parameter->convertTo0to1(settings[index][i]));parameter->endChangeGesture();}
        presets.setSelectedId(0,juce::dontSendNotification);
    };
    addAndMakeVisible(presets);addAndMakeVisible(bypass);addAndMakeVisible(zoom);
    bypassAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.parameters,"bypass",bypass);
    zoom.addItemList({"80%","100%","125%"},1);zoom.setSelectedId(2,juce::dontSendNotification);
    zoom.onChange=[this] {const float scale=zoom.getSelectedId()==1?.8f:zoom.getSelectedId()==3?1.25f:1.f;setSize(int(1200*scale),int(780*scale));};
    setResizable(true,true);setResizeLimits(960,624,1500,975);getConstrainer()->setFixedAspectRatio(1200.0/780.0);
    setSize(1200,780);startTimerHz(5);
}
DreamVoxAudioProcessorEditor::~DreamVoxAudioProcessorEditor(){stopTimer();setLookAndFeel(nullptr);}
void DreamVoxAudioProcessorEditor::resized() {
    presets.setBounds(425,65,376,28);bypass.setBounds(899,28,132,28);zoom.setBounds(1061,28,90,28);
    night::place(controls[0],labels[0],28,492,100);night::place(controls[1],labels[1],132,492,100);night::place(controls[2],labels[2],236,492,100);
    night::place(controls[3],labels[3],368,492,100);night::place(controls[4],labels[4],476,492,100);
    night::place(controls[5],labels[5],607,483,122,145);night::place(controls[6],labels[6],807,483,122,145);night::place(controls[7],labels[7],1011,483,122,145);
    const float sx=getWidth()/1200.f,sy=getHeight()/780.f;
    for(auto* c:getChildren()) if(dynamic_cast<juce::ResizableCornerComponent*>(c)==nullptr) {auto r=c->getBounds();c->setBounds(juce::roundToInt(r.getX()*sx),juce::roundToInt(r.getY()*sy),juce::roundToInt(r.getWidth()*sx),juce::roundToInt(r.getHeight()*sy));}
}
void DreamVoxAudioProcessorEditor::paint(juce::Graphics& g) {
    g.addTransform(juce::AffineTransform::scale(getWidth()/1200.f,getHeight()/780.f));
    night::background(g,artwork,look,"Velvet Nights");
    night::panel(g,artwork,look,20,324,474,385,"EQ","TONE & CLARITY");
    night::panel(g,artwork,look,354,225,314,385,"COMP","DYNAMICS");
    night::panel(g,artwork,look,589,190,638,385,"DE-ESS","SMOOTH");
    night::panel(g,artwork,look,789,190,804,385,"TONE","CHARACTER");
    night::panel(g,artwork,look,989,191,143,385,"OUTPUT","FINAL LEVEL");
    night::text(g,"180 Hz        1.1 kHz         9 kHz",31,642,300,24,11,night::muted);
    night::text(g,"Shape the low end, body and air.",30,674,303,20,11,night::muted);
    night::text(g,"12 ms attack / 120 ms release",360,663,213,28,10,night::muted);
    night::text(g,"Sibilance control\n6.2 kHz detector",600,655,168,40,11,night::muted);
    night::text(g,"Soft saturation\nBlend to taste",800,655,168,40,11,night::muted);
    night::text(g,"Match listening levels\nwhen comparing bypass",998,655,173,40,11,night::muted);
    const bool off=processor.parameters.getRawParameterValue("bypass")->load()>.5f;
    night::footer(g,look,off?"BYPASSED  /  Original signal":"Same voice. Different universe.   /   Double-click a knob to reset; type a value for precision.","DreamVox  " JucePlugin_VersionString);
}
