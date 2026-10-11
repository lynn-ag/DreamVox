#include "PluginEditor.h"

namespace
{
const juce::String ids[] {"low", "body", "air", "threshold", "ratio", "deess", "warmth", "output"};
const juce::String titles[] {"LOW", "BODY", "AIR", "THRESHOLD", "RATIO", "DE-ESS", "WARMTH", "OUTPUT"};
const juce::Colour ink (0xff100c1e), panel (0xff21182f), muted (0xffb7a9c9), pearl (0xfffff0fc), aqua (0xffff8cce), violet (0xffa78bfa);
}

DreamVoxAudioProcessorEditor::DreamVoxAudioProcessorEditor (DreamVoxAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setSize (820, 560);
    for (int i=0;i<8;++i)
    {
        auto* slider=controls.add (new juce::Slider());
        slider->setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 82, 22);
        slider->setColour (juce::Slider::rotarySliderFillColourId, aqua);
        slider->setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff51405f));
        slider->setColour (juce::Slider::thumbColourId, pearl);
        slider->setColour (juce::Slider::textBoxTextColourId, pearl);
        slider->setColour (juce::Slider::textBoxBackgroundColourId, panel);
        slider->setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        attachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.parameters, ids[i], *slider));
        auto* label=sectionLabels.add (new juce::Label());
        label->setText (titles[i], juce::dontSendNotification);
        label->setJustificationType (juce::Justification::centred);
        label->setColour (juce::Label::textColourId, muted);
        label->setFont (juce::Font (12.0f, juce::Font::bold));
        addAndMakeVisible (*slider); addAndMakeVisible (*label);
    }
    bypass.setColour (juce::ToggleButton::textColourId, pearl);
    bypass.setColour (juce::ToggleButton::tickColourId, aqua);
    bypass.setColour (juce::ToggleButton::tickDisabledColourId, muted);
    bypassAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (processor.parameters, "bypass", bypass);
    addAndMakeVisible (bypass);
}

void DreamVoxAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto bounds=getLocalBounds().toFloat();
    juce::ColourGradient bg (juce::Colour (0xff1d1d31), 0.0f, 0.0f, ink, 820.0f, 560.0f, false);
    g.setGradientFill (bg); g.fillAll();
    g.setColour (juce::Colour (0x20ff8cce)); g.fillEllipse (470.0f, -210.0f, 470.0f, 470.0f);
    g.setColour (juce::Colour (0x18a78bfa)); g.fillEllipse (-200.0f, 300.0f, 460.0f, 460.0f);
    g.setColour (juce::Colour (0x1877e2d1)); g.fillEllipse (540.0f, -125.0f, 310.0f, 310.0f);
    g.setColour (juce::Colour (0x17aa8cff)); g.fillEllipse (-120.0f, 330.0f, 270.0f, 270.0f);
    g.setColour (juce::Colour (0xff292b3d)); g.drawLine (34.0f, 112.0f, 786.0f, 112.0f, 1.0f);
    g.setColour (pearl); g.setFont (juce::Font (29.0f, juce::Font::bold)); g.drawText ("DreamVox", 34, 24, 310, 42, juce::Justification::left);
    g.setColour (aqua); g.fillRoundedRectangle (34.0f, 73.0f, 28.0f, 3.0f, 1.5f);
    g.setColour (muted); g.setFont (juce::Font (12.5f)); g.drawText ("SAME VOICE. DIFFERENT UNIVERSE.", 74, 66, 300, 20, juce::Justification::left);
    g.setColour (muted); g.setFont (juce::Font (11.0f)); g.drawText ("V0.2 DREAMSCAPE  /  R&B VOCAL SUITE", 440, 37, 345, 18, juce::Justification::right);
    g.setColour (juce::Colour (0xff252738)); g.fillRoundedRectangle (18.0f, 130.0f, 784.0f, 372.0f, 18.0f);
    g.setColour (juce::Colour (0x2277e2d1)); g.drawRoundedRectangle (18.0f, 119.0f, 724.0f, 350.0f, 18.0f, 1.0f);
    g.setColour (aqua); g.setFont (juce::Font (10.5f, juce::Font::bold)); g.drawText ("01  /  SCULPT YOUR TONE", 40, 148, 90, 14, juce::Justification::left);
    g.drawText ("02  /  SHAPE YOUR DYNAMICS", 40, 332, 180, 14, juce::Justification::left);
    g.setColour (muted); g.setFont (juce::Font (10.5f)); g.drawText ("Start subtle. Make the atmosphere yours.", 34, 525, 400, 16, juce::Justification::left);
}

void DreamVoxAudioProcessorEditor::resized()
{
    const int left=64, gap=190, size=106;
    for (int i=0;i<8;++i)
    {
        const int row=i/4, col=i%4;
        const int x=left+col*gap, y=row==0?174:358;
        sectionLabels[i]->setBounds (x-10, y, 114, 18);
        controls[i]->setBounds (x, y+21, size, 118);
    }
    bypass.setBounds (690, 521, 110, 26);
}
