#include "PluginEditor.h"
namespace {
const juce::Colour bg(0xff10161e),card(0xff19232e),ink(0xffeef4f1),dim(0xff91a4b1),mint(0xff8ce4c0),gold(0xfff0b878);
const char* ids[]={"input","bass","clarity","smooth","air","glue","width","drive","ceiling"};
const char* names[]={"INPUT","BASS","CLARITY","SMOOTH","AIR","GLUE","WIDTH","LOUDNESS","CEILING"};
const char* hints[]={"Trim level before processing. Start at 0 dB.","Broad low shelf at 100 Hz. Small moves work best.","Gently reduces low-mid buildup around 320 Hz, up to 3.5 dB.","Dynamic control around 3.8 kHz. Acts only when that band gets loud.","High shelf at 10 kHz. Negative values soften a bright mix.","Gentle stereo-linked compression, capped at 3 dB reduction; no auto makeup.","Stereo width above the low-bass region. 100% keeps the original width.","Gain into the limiter. Start at zero; reduce if LIMIT reduction stays high.","Sample-peak output ceiling. This is not a true-peak limiter; leave export headroom."};
}
DreamMasterEditor::DreamMasterEditor(DreamMasterProcessor& p):AudioProcessorEditor(&p),processor(p)
{
    look.setColour(juce::Slider::rotarySliderFillColourId,mint);
    look.setColour(juce::Slider::rotarySliderOutlineColourId,juce::Colour(0xff344652));
    look.setColour(juce::Slider::thumbColourId,ink);
    look.setColour(juce::Slider::textBoxTextColourId,ink);
    look.setColour(juce::Slider::textBoxBackgroundColourId,card);
    look.setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);
    look.setColour(juce::ComboBox::backgroundColourId,card);
    look.setColour(juce::ComboBox::textColourId,ink);
    look.setColour(juce::ToggleButton::textColourId,ink);
    look.setColour(juce::ToggleButton::tickColourId,mint);
    setLookAndFeel(&look);
    for(size_t i=0;i<controls.size();++i)
    {
        auto& s=controls[i];s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow,false,106,22);s.setNumDecimalPlacesToDisplay(1);
        s.setTooltip(hints[i]);s.setName(names[i]);
        labels[i].setText(names[i],juce::dontSendNotification);labels[i].setJustificationType(juce::Justification::centred);
        labels[i].setColour(juce::Label::textColourId,dim);labels[i].setFont(juce::Font(12.f,juce::Font::bold));
        addAndMakeVisible(s);addAndMakeVisible(labels[i]);
        attachments[i]=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.parameters,ids[i],s);
    }
    controls[7].setColour(juce::Slider::rotarySliderFillColourId,gold);
    controls[8].setColour(juce::Slider::rotarySliderFillColourId,gold);
    presets.addItemList({"Clean start","Gentle polish","Warm & smooth","Keep the punch","Soften the top"},1);
    presets.setTextWhenNothingSelected("Choose a starting point...");
    presets.setTooltip("Starting settings only; adjust for each song. Existing settings are replaced.");
    presets.onChange=[this]{if(presets.getSelectedId()>0) {processor.applyPreset(presets.getSelectedId()-1);presets.setSelectedId(0,juce::dontSendNotification);}};
    addAndMakeVisible(presets);addAndMakeVisible(bypass);addAndMakeVisible(lowCut);
    bypass.setTooltip("Compare against the original with the same 5 ms delay. Levels are not automatically matched.");
    lowCut.setTooltip("Optional 22 Hz high-pass filter. Leave off unless you need sub-rumble cleanup.");
    bypassAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.parameters,"bypass",bypass);
    cutAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.parameters,"lowcut",lowCut);
    // Size only after every control exists: resized() is called synchronously.
    setSize(920,640);startTimerHz(20);
}
DreamMasterEditor::~DreamMasterEditor() { stopTimer();setLookAndFeel(nullptr); }
void DreamMasterEditor::resized()
{
    presets.setBounds(27,84,275,30);lowCut.setBounds(324,84,155,30);bypass.setBounds(782,34,110,30);
    for(size_t i=0;i<controls.size();++i)
    {
        const auto x=28+static_cast<int>(i%3)*212,y=142+static_cast<int>(i/3)*150;
        labels[i].setBounds(x,y,192,22);controls[i].setBounds(x+38,y+24,116,110);
    }
}
void DreamMasterEditor::paint(juce::Graphics& g)
{
    g.fillAll(bg);g.setColour(ink);g.setFont(juce::Font(29.f,juce::Font::bold));g.drawText("DREAMMASTER",27,20,400,40,juce::Justification::left);
    g.setColour(dim);g.setFont(12.f);g.drawText("A GENTLE FINISH FOR YOUR MUSIC",29,59,450,20,juce::Justification::left);
    g.setColour(card);g.fillRoundedRectangle(20,132,638,454,16);g.fillRoundedRectangle(678,132,222,454,16);
    g.setColour(mint);g.setFont(juce::Font(13.f,juce::Font::bold));g.drawText("LEVELS & DYNAMICS",696,152,188,24,juce::Justification::left);
    auto meter=[&](const juce::String& label,float value,int y,bool reduction)
    {
        g.setColour(dim);g.setFont(11.f);g.drawText(label,697,y,112,20,juce::Justification::left);
        g.setColour(ink);g.setFont(14.f);g.drawText(juce::String(juce::jmax(-90.f,value),1)+" dB",799,y,82,20,juce::Justification::right);
        g.setColour(bg);g.fillRoundedRectangle(698.f,static_cast<float>(y+27),183.f,7.f,3.f);
        const auto amount=juce::jlimit(0.f,1.f,reduction?value/9.f:(value+60.f)/60.f);
        g.setColour(reduction?gold:(value>-.1f?juce::Colours::salmon:mint));
        g.fillRoundedRectangle(698.f,static_cast<float>(y+27),183.f*amount,7.f,3.f);
    };
    meter("INPUT PEAK",processor.inDb.load(),194,false);meter("OUTPUT PEAK",processor.outDb.load(),254,false);
    meter("OUTPUT RMS",processor.rmsDb.load(),314,false);meter("GLUE REDUCTION",processor.compDb.load(),385,true);meter("LIMIT REDUCTION",processor.limitDb.load(),445,true);
    g.setColour(dim);g.setFont(11.f);g.drawFittedText("Peak / RMS in dBFS\nSample-peak ceiling | 5 ms lookahead",697,510,184,42,juce::Justification::left,3);
    g.setColour(processor.limitDb.load()>3?gold:dim);g.setFont(12.f);
    const auto bypassed=processor.parameters.getRawParameterValue("bypass")->load()>.5f;
    g.drawText(bypassed?"BYPASSED - ORIGINAL LEVEL":processor.limitDb.load()>3?"Heavy limiting: try less Loudness.":"Start gently. Compare at similar loudness.",27,598,690,24,juce::Justification::left);
    g.setColour(dim);g.drawText("v" JucePlugin_VersionString,796,598,98,24,juce::Justification::right);
}
