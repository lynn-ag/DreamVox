#include "PluginEditor.h"
#include "FactoryPresets.h"
#include <BinaryData.h>
namespace
{
const juce::String ids[] {"low", "body", "air", "threshold", "ratio", "deess", "warmth", "output", "space", "room", "echo", "echoTime", "feedback", "doubler"};
const juce::String titles[] {"LOW", "BODY", "AIR", "THRESHOLD", "RATIO", "DE-ESS", "WARMTH", "OUTPUT", "REVERB", "ROOM", "ECHO", "TIME", "FEEDBACK", "DOUBLER"};
const juce::Colour ink (0xff100a22), panel (0xff1b1434), muted (0xffc4b4da), pearl (0xfffff4ff), aqua (0xfff18bdb), violet (0xffa989ff);
}

DreamVoxAudioProcessorEditor::DreamVoxAudioProcessorEditor (DreamVoxAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    
    for (int i=0;i<14;++i)
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

// Load the bundled PNG once instead of decoding it on every repaint.
    // The resource lookup is deliberately explicit to make packaging problems visible.
    int artworkBytes = 0;
    const auto* artworkData = BinaryData::getNamedResource (
        "DreamVox_Neon_Vocal_Suite_png", artworkBytes);
    if (artworkData != nullptr && artworkBytes > 0)
        neonArtwork = juce::ImageFileFormat::loadFrom (artworkData, artworkBytes);

    // Keep setSize last: JUCE may invoke resized() synchronously.
    setSize (1020, 700);
}

void DreamVoxAudioProcessorEditor::paint (juce::Graphics& g)
{
    const auto area = getLocalBounds().toFloat();
    if (neonArtwork.isValid())
    {
        // Draw the moonlit city artwork, preserving its aspect ratio in this editor.
        g.drawImage (neonArtwork, area, juce::RectanglePlacement::stretchToFit);
    }
    else
    {
        // Clear, diagnostic fallback if the image was not embedded in this build.
        juce::ColourGradient bg (juce::Colour (0xff32194e), 0.0f, 0.0f, ink,
                                 static_cast<float> (getWidth()), static_cast<float> (getHeight()), false);
        g.setGradientFill (bg);
        g.fillAll();
        g.setColour (juce::Colours::white);
        g.drawText ("NEON ARTWORK MISSING — check the GitHub build",
                    34, 118, 850, 28, juce::Justification::left);
    }

    // An opaque-enough layer makes REAL JUCE controls readable above the
    // flattened design reference; decorative knobs in the image are not controls.
    g.setColour (juce::Colour (0xe6110a27));
    g.fillRoundedRectangle (18.0f, 14.0f, 984.0f, 96.0f, 18.0f);
    g.setColour (juce::Colour (0xe9100b25));
    g.fillRoundedRectangle (18.0f, 266.0f, 984.0f, 369.0f, 18.0f);
    g.setColour (violet.withAlpha (0.65f));
    g.drawRoundedRectangle (18.0f, 266.0f, 984.0f, 369.0f, 18.0f, 1.2f);

    g.setColour (pearl);
    g.setFont (juce::Font (29.0f, juce::Font::bold));
    g.drawText ("DREAMVOX 5", 34, 27, 340, 40, juce::Justification::left);
    g.setColour (aqua);
    g.fillRoundedRectangle (34.0f, 78.0f, 45.0f, 3.0f, 1.5f);
    g.setColour (muted);
    g.setFont (juce::Font (12.0f));
    g.drawText ("MOONLIT VOCAL SUITE  /  R&B  /  POP", 91, 70, 460, 22, juce::Justification::left);
    g.drawText ("V0.5  •  NEON EDITION", 720, 42, 264, 22, juce::Justification::right);

    g.setColour (aqua);
    g.setFont (juce::Font (12.5f, juce::Font::bold));
    g.drawText ("TONE  /  DYNAMICS", 38, 274, 400, 19, juce::Justification::left);
    g.drawText ("SPACE  /  ECHO  /  DOUBLER", 38, 457, 420, 19, juce::Justification::left);

    g.setColour (pearl);
    g.setFont (juce::Font (11.5f));
    g.drawText ("FACTORY PRESETS", 663, 174, 320, 18, juce::Justification::left);
    g.setColour (muted);
    g.drawText ("Make small moves. Let your vocal stay yours.",
                34, 651, 520, 22, juce::Justification::left);
}

void DreamVoxAudioProcessorEditor::resized()
{
    // Two complete, accessible rows: all fourteen DSP parameters stay editable.
    const int left = 38, gap = 139, size = 92;
    for (int i = 0; i < 14; ++i)
    {
        const int row = i / 7, col = i % 7;
        const int x = left + col * gap;
        const int y = row == 0 ? 309 : 492;
        sectionLabels[i]->setBounds (x - 6, y - 17, 106, 18);
        controls[i]->setBounds (x, y + 2, size, 122);
    }
    presetSelector.setBounds (660, 199, 324, 31);
    bypass.setBounds (870, 649, 112, 26);
}
