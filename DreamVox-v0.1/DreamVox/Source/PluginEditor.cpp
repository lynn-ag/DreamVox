#include "PluginEditor.h"
#include "FactoryPresets.h"

namespace
{
const juce::String ids[] {"low", "body", "air", "threshold", "ratio", "deess", "warmth", "output"};
const juce::String titles[] {"LOW", "BODY", "AIR", "THRESHOLD", "RATIO", "DE-ESS", "WARMTH", "OUTPUT"};
const juce::Colour ink (0xff11121b), panel (0xff191b28), muted (0xff9296ac), pearl (0xfff3f0ff), aqua (0xff77e2d1), violet (0xffaa8cff);
}

DreamVoxAudioProcessorEditor::DreamVoxAudioProcessorEditor (DreamVoxAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setSize (760, 510);
    for (int i=0;i<8;++i)
    {
        auto* slider=controls.add (new juce::Slider());
        slider->setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 82, 22);
        slider->setColour (juce::Slider::rotarySliderFillColourId, aqua);
        slider->setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff37394d));
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
    presetSelector.addItem ("Select a factory preset...", 1);
    for (size_t i = 0; i < DreamVoxPresets::factory.size(); ++i)
    {
        const auto& preset = DreamVoxPresets::factory[i];
        presetSelector.addItem (juce::String (preset.category) + " / " + preset.name,
                                static_cast<int> (i) + 2);
    }
    presetSelector.setSelectedId (1, juce::dontSendNotification);
    presetSelector.onChange = [this]
    {
        const int index = presetSelector.getSelectedId() - 2;
        if (index >= 0 && index < static_cast<int> (DreamVoxPresets::factory.size()))
            DreamVoxPresets::apply (processor.parameters, DreamVoxPresets::factory[static_cast<size_t> (index)]);
    };
    presetSelector.setColour (juce::ComboBox::backgroundColourId, panel);
    presetSelector.setColour (juce::ComboBox::textColourId, pearl);
    presetSelector.setColour (juce::ComboBox::outlineColourId, violet);
    addAndMakeVisible (presetSelector);
    bypass.setColour (juce::ToggleButton::textColourId, pearl);
    bypass.setColour (juce::ToggleButton::tickColourId, aqua);
    bypass.setColour (juce::ToggleButton::tickDisabledColourId, muted);
    bypassAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (processor.parameters, "bypass", bypass);
    addAndMakeVisible (bypass);
}

void DreamVoxAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto bounds=getLocalBounds().toFloat();
    juce::ColourGradient bg (juce::Colour (0xff1d1d31), 0.0f, 0.0f, ink, 760.0f, 510.0f, false);
    g.setGradientFill (bg); g.fillAll();
    g.setColour (juce::Colour (0x1877e2d1)); g.fillEllipse (540.0f, -125.0f, 310.0f, 310.0f);
    g.setColour (juce::Colour (0x17aa8cff)); g.fillEllipse (-120.0f, 330.0f, 270.0f, 270.0f);
    g.setColour (juce::Colour (0xff292b3d)); g.drawLine (34.0f, 104.0f, 726.0f, 104.0f, 1.0f);
    g.setColour (pearl); g.setFont (juce::Font (29.0f, juce::Font::bold)); g.drawText ("DREAMVOX", 34, 24, 270, 40, juce::Justification::left);
    g.setColour (aqua); g.fillRoundedRectangle (34.0f, 73.0f, 28.0f, 3.0f, 1.5f);
    g.setColour (muted); g.setFont (juce::Font (12.5f)); g.drawText ("VOCAL TONE  /  DYNAMICS  /  FINISH", 74, 66, 300, 20, juce::Justification::left);
    g.setColour (muted); g.setFont (juce::Font (11.0f)); g.drawText ("V0.1  •  R&B / POP VOCAL RACK", 415, 37, 310, 18, juce::Justification::right);
    g.setColour (juce::Colour (0xff252738)); g.fillRoundedRectangle (18.0f, 119.0f, 724.0f, 350.0f, 18.0f);
    g.setColour (juce::Colour (0x2277e2d1)); g.drawRoundedRectangle (18.0f, 119.0f, 724.0f, 350.0f, 18.0f, 1.0f);
    g.setColour (aqua); g.setFont (juce::Font (10.5f, juce::Font::bold)); g.drawText ("TONE", 40, 133, 90, 14, juce::Justification::left);
    g.drawText ("DYNAMICS & OUTPUT", 40, 300, 180, 14, juce::Justification::left);
    g.setColour (muted); g.setFont (juce::Font (10.5f)); g.drawText ("Make small moves. Let your vocal stay yours.", 34, 482, 400, 16, juce::Justification::left);
}

void DreamVoxAudioProcessorEditor::resized()
{
    const int left=55, gap=170, size=94;
    for (int i=0;i<8;++i)
    {
        const int row=i/4, col=i%4;
        const int x=left+col*gap, y=row==0?155:322;
        sectionLabels[i]->setBounds (x-10, y, 114, 18);
        controls[i]->setBounds (x, y+21, size, 118);
    }
    presetSelector.setBounds (404, 71, 322, 27);
    bypass.setBounds (625, 476, 105, 25);
}
