#pragma once
#include <juce_gui_extra/juce_gui_extra.h>

namespace night {
inline const juce::Colour white(0xfff5eff7), muted(0xffc2b8ce), dark(0xff070c15);
class Look final : public juce::LookAndFeel_V4 {
public:
    explicit Look(bool master):accent(master?juce::Colour(0xffffd18c):juce::Colour(0xffffa1d2)),blue(master?juce::Colour(0xff78d9ff):juce::Colour(0xffb2a1ff)) {
        setColour(juce::Slider::textBoxTextColourId,white);
        setColour(juce::Slider::textBoxBackgroundColourId,juce::Colour(0xff11121e));
        setColour(juce::Slider::textBoxOutlineColourId,accent.withAlpha(.25f));
        setColour(juce::Slider::textBoxHighlightColourId,accent.withAlpha(.3f));
        setColour(juce::ComboBox::backgroundColourId,dark);
        setColour(juce::ComboBox::textColourId,white);
        setColour(juce::ComboBox::outlineColourId,accent.withAlpha(.45f));
        setColour(juce::PopupMenu::backgroundColourId,dark);
        setColour(juce::PopupMenu::textColourId,white);
        setColour(juce::PopupMenu::highlightedBackgroundColourId,accent.withAlpha(.25f));
        setColour(juce::ToggleButton::textColourId,white);
        setColour(juce::ToggleButton::tickColourId,accent);
        setColour(juce::Label::textColourId,white);
    }
    void drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float p,float start,float end,juce::Slider&) override {
        const auto r=juce::jmin(w,h)*.5f-10.f,cx=x+w*.5f,cy=y+h*.5f;
        for(int i=4;i>0;--i) {g.setColour(accent.withAlpha(.018f));g.fillEllipse(cx-r-i*2,cy-r-i*2,2*r+i*4,2*r+i*4);}
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff30303c),cx-r,cy-r,juce::Colours::black,cx+r,cy+r,false));
        g.fillEllipse(cx-r,cy-r,r*2,r*2);
        g.setColour(accent.withAlpha(.35f));g.drawEllipse(cx-r,cy-r,r*2,r*2,1.f);
        juce::Path arc;arc.addCentredArc(cx,cy,r+3,r+3,0,start,end,true);
        g.setColour(accent.withAlpha(.15f));g.strokePath(arc,juce::PathStrokeType(3));
        juce::Path value;value.addCentredArc(cx,cy,r+3,r+3,0,start,start+p*(end-start),true);
        g.setColour(accent);g.strokePath(value,juce::PathStrokeType(2.5f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
        const float a=start+p*(end-start);
        g.setColour(white);g.drawLine(cx+std::sin(a)*(r*.64f),cy-std::cos(a)*(r*.64f),cx+std::sin(a)*(r*.92f),cy-std::cos(a)*(r*.92f),2.5f);
        g.setColour(blue.withAlpha(.12f));g.drawEllipse(cx-r+6,cy-r+6,2*r-12,2*r-12,1);
    }
    juce::Colour accent,blue;
};
inline void text(juce::Graphics& g,const juce::String& s,int x,int y,int w,int h,float size,juce::Colour colour=white,int align=juce::Justification::centred) {
    g.setColour(colour);g.setFont(juce::Font(juce::FontOptions(size)));g.drawFittedText(s,x,y,w,h,align,2);
}
inline void background(juce::Graphics& g,const juce::Image& image,const Look& look,const juce::String& theme) {
    g.fillAll(dark);
    if(image.isValid()) g.drawImage(image,0,0,1200,266,0,0,image.getWidth(),340);
    // Replace all illustrated preset/menu widgets with actual native controls.
    g.setColour(dark);g.fillRoundedRectangle(406,17,414,89,20);g.fillRoundedRectangle(880,17,294,50,20);
    text(g,theme,426,24,374,28,19,look.accent);
    // The reference navigation is not part of this editor.
    g.setColour(dark);g.fillRoundedRectangle(-15,136,108,140,22);
    text(g,"VOCAL / MASTER\nAUDIO SUITE",8,163,75,68,10,look.accent);
    g.setGradientFill(juce::ColourGradient(dark.withAlpha(0.f),0,219,dark,0,279,false));g.fillRect(0,215,1200,67);
    g.setColour(look.accent.withAlpha(.35f));g.drawHorizontalLine(277,20,1180);
}
inline void panel(juce::Graphics& g,const juce::Image& art,const Look& look,int x,int w,int iconX,int iconY,const juce::String& title,const juce::String& subtitle) {
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff171522),float(x),288,dark,float(x+w),716,false));
    g.fillRoundedRectangle(float(x),288,float(w),424,16);
    g.setColour(look.accent.withAlpha(.45f));g.drawRoundedRectangle(float(x)+.5f,288.5f,float(w)-1,423,16,1);
    if(art.isValid())g.drawImage(art,x+w/2-52,310,104,67,iconX,iconY,134,86);
    text(g,title,x+8,392,w-16,26,16);
    text(g,subtitle,x+8,421,w-16,22,10,look.accent);
    g.setColour(look.accent.withAlpha(.16f));g.drawHorizontalLine(455,float(x+14),float(x+w-14));
}
inline void slider(juce::Slider& s,juce::Label& label,juce::Component& owner,const char* name,const char* hint) {
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);s.setTextBoxStyle(juce::Slider::TextBoxBelow,false,94,23);
    s.setNumDecimalPlacesToDisplay(1);s.setName(name);s.setTooltip(hint);s.setWantsKeyboardFocus(true);
    label.setText(name,juce::dontSendNotification);label.setJustificationType(juce::Justification::centred);label.setFont(juce::Font(juce::FontOptions(12.f)));
    owner.addAndMakeVisible(s);owner.addAndMakeVisible(label);
}
inline void place(juce::Slider& s,juce::Label& label,int x,int y,int w=106,int h=120) {label.setBounds(x,y,w,22);s.setBounds(x,y+22,w,h);}
inline void footer(juce::Graphics& g,const Look& look,const juce::String& message,const juce::String& version) {
    g.setColour(look.accent.withAlpha(.25f));g.drawHorizontalLine(734,20,1180);
    text(g,message,24,746,920,30,12,muted,juce::Justification::left);
    text(g,version,970,746,205,30,12,look.accent,juce::Justification::right);
}
}
