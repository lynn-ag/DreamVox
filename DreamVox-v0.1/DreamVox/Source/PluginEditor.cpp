#include "PluginEditor.h"
#include "FactoryPresets.h"
#include <BinaryData.h>
#include <cmath>

namespace
{
const char* ids[] {
    "low", "body", "air", "threshold", "ratio", "deess", "warmth",
    "output", "space", "room", "echo", "echoTime", "feedback", "doubler",
    "highpass", "gateThreshold", "compAttack", "compRelease",
    "deessFreq", "doublerSpread"
};
const char* titles[] {
    "LOW", "BODY", "AIR", "THRESHOLD", "RATIO", "DE-ESS", "WARMTH",
    "OUTPUT", "REVERB MIX", "ROOM SIZE", "ECHO MIX", "ECHO TIME",
    "FEEDBACK", "DOUBLER MIX", "HIGH-PASS", "GATE FLOOR",
    "ATTACK", "RELEASE", "DE-ESS FREQ", "SPREAD"
};
constexpr int moduleKnobs[8][4] = {
    {14, 15, -1, -1}, {3, 4, 16, 17}, {0, 1, 2, -1}, {5, 18, -1, -1},
    {6, -1, -1, -1}, {8, 9, -1, -1}, {10, 11, 12, -1}, {13, 19, -1, -1}
};
constexpr int moduleCounts[8] = {2, 4, 3, 2, 1, 2, 3, 2};
const juce::Colour ink (0xff100922), panel (0xff1a102c), muted (0xffc2addb);
const juce::Colour pearl (0xffffefff), pink (0xfffa84d5), violet (0xffaf91ff), aqua (0xff73cfff);

float levelToNorm (float level)
{
    const float db = juce::Decibels::gainToDecibels (juce::jmax (0.000001f, level), -100.0f);
    return juce::jlimit (0.0f, 1.0f, (db + 60.0f) / 66.0f);
}
}

DreamVoxAudioProcessorEditor::DreamVoxAudioProcessorEditor (DreamVoxAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    for (int i = 0; i < 20; ++i)
    {
        auto* slider = controls.add (new juce::Slider());
        slider->setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 100, 22);
        slider->setColour (juce::Slider::rotarySliderFillColourId, pink);
        slider->setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff423253));
        slider->setColour (juce::Slider::thumbColourId, pearl);
        slider->setColour (juce::Slider::textBoxTextColourId, pearl);
        slider->setColour (juce::Slider::textBoxBackgroundColourId, panel);
        slider->setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        attachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            processor.parameters, ids[i], *slider));
        auto* label = sectionLabels.add (new juce::Label());
        label->setText (titles[i], juce::dontSendNotification);
        label->setJustificationType (juce::Justification::centred);
        label->setColour (juce::Label::textColourId, muted);
        label->setFont (juce::Font (12.0f, juce::Font::bold));
        addAndMakeVisible (*slider);
        addAndMakeVisible (*label);
    }

    presetSelector.addItem ("Select a chain preset...", 1);
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
        {
            DreamVoxPresets::apply (processor.parameters,
                                   DreamVoxPresets::factory[static_cast<size_t> (index)]);
            effectPresetSelector.setSelectedId (1, juce::dontSendNotification);
        }
    };
    for (auto* selector : { &presetSelector, &effectPresetSelector })
    {
        selector->setColour (juce::ComboBox::backgroundColourId, panel);
        selector->setColour (juce::ComboBox::textColourId, pearl);
        selector->setColour (juce::ComboBox::outlineColourId, violet);
        addAndMakeVisible (*selector);
    }
    effectPresetSelector.onChange = [this]
    {
        const int presetIndex = effectPresetSelector.getSelectedId() - 2;
        if (presetIndex >= 0)
            DreamVoxPresets::applyEffect (processor.parameters, selectedModule, presetIndex);
    };

    for (int i = 0; i < 8; ++i)
    {
        auto& button = moduleButtons[static_cast<size_t> (i)];
        button.setButtonText (DreamVoxPresets::effectBanks[static_cast<size_t> (i)].name);
        button.setColour (juce::TextButton::textColourOffId, pearl);
        button.setColour (juce::TextButton::buttonColourId, panel);
        button.setColour (juce::TextButton::buttonOnColourId, pink);
        button.onClick = [this, i] { selectModule (i); };
        addAndMakeVisible (button);
    }
    bypass.setColour (juce::ToggleButton::textColourId, pearl);
    bypass.setColour (juce::ToggleButton::tickColourId, pink);
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        processor.parameters, "bypass", bypass);
    addAndMakeVisible (bypass);

    int artworkBytes = 0;
    const auto* artworkData = BinaryData::getNamedResource (
        "DreamVox_Neon_Vocal_Suite_png", artworkBytes);
    if (artworkData != nullptr && artworkBytes > 0)
        neonArtwork = juce::ImageFileFormat::loadFrom (artworkData, artworkBytes);

    selectModule (0);
    // Keep resizing last: JUCE can call resized() during construction.
    setSize (1120, 720);
    startTimerHz (24);
}

void DreamVoxAudioProcessorEditor::selectModule (int index)
{
    if (index < 0 || index >= 8) return;
    selectedModule = index;
    for (int i = 0; i < 20; ++i)
    {
        bool show = (i == 7); // Global output is always accessible.
        for (int j = 0; j < moduleCounts[selectedModule]; ++j)
            show = show || (moduleKnobs[selectedModule][j] == i);
        controls[i]->setVisible (show);
        sectionLabels[i]->setVisible (show);
    }

    for (int i = 0; i < 8; ++i)
        moduleButtons[static_cast<size_t> (i)].setColour (
            juce::TextButton::buttonColourId,
            i == selectedModule ? juce::Colour (0xff793261) : panel);

    effectPresetSelector.clear (juce::dontSendNotification);
    effectPresetSelector.addItem ("Choose an effect preset...", 1);
    const auto& bank = DreamVoxPresets::effectBanks[static_cast<size_t> (selectedModule)];
    for (int i = 0; i < 4; ++i)
        effectPresetSelector.addItem (bank.presets[static_cast<size_t> (i)].name, i + 2);
    effectPresetSelector.setSelectedId (1, juce::dontSendNotification);
    resized();
    repaint();
}

void DreamVoxAudioProcessorEditor::timerCallback()
{
    // Ballistic decay is visual only; audio-thread readings are never altered.
    for (int ch = 0; ch < 2; ++ch)
    {
        const auto n = static_cast<size_t> (ch);
        shownInput[n] = juce::jmax (
            processor.inputPeaks[n].load (std::memory_order_relaxed), shownInput[n] * 0.89f);
        shownOutput[n] = juce::jmax (
            processor.outputPeaks[n].load (std::memory_order_relaxed), shownOutput[n] * 0.89f);
    }
    shownCompReduction = juce::jmax (
        processor.compressorReductionDb.load (std::memory_order_relaxed),
        shownCompReduction * 0.88f);
    shownDeessReduction = juce::jmax (
        processor.deesserReductionDb.load (std::memory_order_relaxed),
        shownDeessReduction * 0.88f);
    repaint (19, 140, 1082, 187);
    repaint (796, 480, 189, 200);
}

void DreamVoxAudioProcessorEditor::drawPeakMeter (
    juce::Graphics& g, juce::Rectangle<float> bounds,
    const std::array<float, 2>& levels, const juce::String& caption) const
{
    g.setColour (pearl);
    g.setFont (juce::Font (12.0f, juce::Font::bold));
    g.drawText (caption, bounds.toNearestInt().removeFromTop (22), juce::Justification::centred);
    const auto area = bounds.reduced (10.0f, 26.0f);
    for (int ch = 0; ch < 2; ++ch)
    {
        const float x = area.getX() + 12.0f + ch * 31.0f;
        const float height = area.getHeight();
        const float y = area.getBottom() - height * levelToNorm (levels[static_cast<size_t> (ch)]);
        g.setColour (juce::Colour (0xff322643));
        g.fillRoundedRectangle (x, area.getY(), 18.0f, height, 4.0f);
        g.setGradientFill (juce::ColourGradient (pink, x, area.getY(),
                                               aqua, x, area.getBottom(), false));
        g.fillRoundedRectangle (x, y, 18.0f, area.getBottom() - y, 3.0f);
    }
    const float peak = juce::jmax (levels[0], levels[1]);
    const float db = juce::Decibels::gainToDecibels (peak, -90.0f);
    g.setColour (muted);
    g.setFont (juce::Font (11.0f));
    g.drawText (juce::String (db, 1) + " dB", bounds.toNearestInt().withTrimmedTop (
        static_cast<int> (bounds.getHeight()) - 19), juce::Justification::centred);
}

void DreamVoxAudioProcessorEditor::paintMonitor (juce::Graphics& g)
{
    const auto monitor = juce::Rectangle<float> (19.0f, 138.0f, 1082.0f, 186.0f);
    g.setColour (juce::Colour (0xf20b071b));
    g.fillRoundedRectangle (monitor, 17.0f);
    g.setColour (violet.withAlpha (0.7f));
    g.drawRoundedRectangle (monitor, 17.0f, 1.0f);

    drawPeakMeter (g, {28.0f, 155.0f, 106.0f, 157.0f}, shownInput, "INPUT");
    drawPeakMeter (g, {982.0f, 155.0f, 107.0f, 157.0f}, shownOutput, "OUTPUT");

    // The waveform is the actual post-processing audio held in an atomic ring.
    const auto wave = juce::Rectangle<float> (145.0f, 175.0f, 390.0f, 126.0f);
    const auto spec = juce::Rectangle<float> (550.0f, 175.0f, 412.0f, 126.0f);
    g.setColour (muted);
    g.setFont (juce::Font (12.0f, juce::Font::bold));
    g.drawText ("LIVE OUTPUT WAVEFORM", 151, 151, 370, 20, juce::Justification::centred);
    g.drawText ("LIVE OUTPUT SPECTRUM", 560, 151, 390, 20, juce::Justification::centred);
    for (auto rect : {wave, spec})
    {
        g.setColour (juce::Colour (0xff15102a));
        g.fillRoundedRectangle (rect, 7.0f);
        g.setColour (juce::Colour (0xff312440));
        for (int k = 1; k < 4; ++k)
            g.drawHorizontalLine (static_cast<int> (rect.getY() + rect.getHeight() * k / 4.0f),
                                  rect.getX(), rect.getRight());
    }

    const unsigned int write = processor.waveformWrite.load (std::memory_order_acquire);
    juce::Path line;
    const int waveformPoints = static_cast<int> (wave.getWidth());
    for (int i = 0; i < waveformPoints; ++i)
    {
        const unsigned int age = static_cast<unsigned int> ((waveformPoints - 1 - i) * 2);
        const unsigned int pos = (write + DreamVoxAudioProcessor::monitorSize - 1 - age)
                                  % DreamVoxAudioProcessor::monitorSize;
        const float v = juce::jlimit (-1.0f, 1.0f,
                       processor.waveform[pos].load (std::memory_order_relaxed));
        const float x = wave.getX() + static_cast<float> (i);
        const float y = wave.getCentreY() - v * wave.getHeight() * 0.46f;
        if (i == 0) line.startNewSubPath (x, y);
        else line.lineTo (x, y);
    }
    g.setColour (pink.withAlpha (0.95f));
    g.strokePath (line, juce::PathStrokeType (1.5f));

    constexpr int fftSize = 1024;
    for (int i = 0; i < fftSize; ++i)
    {
        const auto pos = (write + DreamVoxAudioProcessor::monitorSize - fftSize + i)
                       % DreamVoxAudioProcessor::monitorSize;
        const float window = 0.5f - 0.5f * std::cos (
            juce::MathConstants<float>::twoPi * i / (fftSize - 1));
        fftScratch[static_cast<size_t> (i)] =
            processor.waveform[pos].load (std::memory_order_relaxed) * window;
    }
    for (int i = fftSize; i < 2 * fftSize; ++i)
        fftScratch[static_cast<size_t> (i)] = 0.0f;
    fft.performFrequencyOnlyForwardTransform (fftScratch.data());
    const float sampleRate = static_cast<float> (juce::jmax (8000.0, processor.getSampleRate()));
    constexpr int bands = 58;
    const float barGap = 1.0f, barWidth = spec.getWidth() / bands - barGap;
    for (int band = 0; band < bands; ++band)
    {
        const float lo = 25.0f * std::pow (900.0f, static_cast<float> (band) / bands);
        const float hi = 25.0f * std::pow (900.0f, static_cast<float> (band + 1) / bands);
        const int minBin = juce::jlimit (1, fftSize / 2 - 1, static_cast<int> (lo * fftSize / sampleRate));
        const int maxBin = juce::jlimit (minBin, fftSize / 2 - 1,
                                        static_cast<int> (hi * fftSize / sampleRate) + 1);
        float magnitude = 0.0f;
        for (int bin = minBin; bin <= maxBin; ++bin)
            magnitude = juce::jmax (magnitude, fftScratch[static_cast<size_t> (bin)]);
        const float db = juce::Decibels::gainToDecibels (magnitude * (2.0f / fftSize), -90.0f);
        const float h = spec.getHeight() * juce::jlimit (0.0f, 1.0f, (db + 78.0f) / 78.0f);
        g.setColour (band % 3 == 0 ? aqua : pink);
        g.fillRoundedRectangle (spec.getX() + band * (barWidth + barGap),
                                spec.getBottom() - h, barWidth, h, 1.0f);
    }
}

void DreamVoxAudioProcessorEditor::paintSelectedModuleMonitor (juce::Graphics& g)
{
    const auto rect = juce::Rectangle<float> (797.0f, 507.0f, 182.0f, 151.0f);
    g.setColour (juce::Colour (0xff130c24));
    g.fillRoundedRectangle (rect, 11.0f);
    g.setColour (violet.withAlpha (0.35f));
    g.drawRoundedRectangle (rect, 11.0f, 1.0f);
    const bool compression = selectedModule == 1, deess = selectedModule == 3;
    const float reduction = compression ? shownCompReduction : shownDeessReduction;
    const float shownPeak = juce::jmax (shownOutput[0], shownOutput[1]);
    const float visual = compression || deess ? juce::jlimit (0.0f, 1.0f, reduction / 18.0f)
                                               : levelToNorm (shownPeak);
    g.setColour (pearl);
    g.setFont (juce::Font (12.0f, juce::Font::bold));
    g.drawText (compression ? "GAIN REDUCTION" : deess ? "DE-ESS REDUCTION" : "LIVE OUTPUT",
                rect.toNearestInt().removeFromTop (35), juce::Justification::centred);
    g.setColour (juce::Colour (0xff3a2645));
    g.fillRoundedRectangle (810.0f, 574.0f, 153.0f, 15.0f, 6.0f);
    g.setColour (pink);
    g.fillRoundedRectangle (810.0f, 574.0f, 153.0f * visual, 15.0f, 6.0f);
    g.setColour (pearl);
    g.setFont (juce::Font (23.0f, juce::Font::bold));
    const float reading = compression || deess ? -reduction :
          juce::Decibels::gainToDecibels (shownPeak, -90.0f);
    g.drawText (juce::String (reading, 1) + " dB", 813, 606, 154, 34,
                juce::Justification::centred);
}

void DreamVoxAudioProcessorEditor::paint (juce::Graphics& g)
{
    if (neonArtwork.isValid())
        g.drawImage (neonArtwork, getLocalBounds().toFloat(),
                     juce::RectanglePlacement::stretchToFit);
    else
        g.fillAll (ink);

    // Functional controls are built above the flattened concept art.
    g.setColour (juce::Colour (0xf00c071b));
    g.fillRoundedRectangle (17.0f, 12.0f, 1086.0f, 109.0f, 17.0f);
    g.setColour (pearl);
    g.setFont (juce::Font (30.0f, juce::Font::bold));
    g.drawText ("DREAMVOX 5", 31, 26, 376, 39, juce::Justification::left);
    g.setColour (pink);
    g.setFont (juce::Font (12.5f));
    g.drawText ("VELVET NIGHTS  •  FUNCTIONAL VOCAL SUITE",
                33, 73, 495, 23, juce::Justification::left);
    g.setColour (muted);
    g.setFont (juce::Font (11.0f));
    g.drawText ("CHAIN PRESETS", 619, 37, 256, 18, juce::Justification::left);

    paintMonitor (g);

    g.setColour (juce::Colour (0xf00f081d));
    g.fillRoundedRectangle (19.0f, 409.0f, 1082.0f, 277.0f, 16.0f);
    g.setColour (violet.withAlpha (0.6f));
    g.drawRoundedRectangle (19.0f, 409.0f, 1082.0f, 277.0f, 16.0f, 1.0f);

    const auto& bank = DreamVoxPresets::effectBanks[static_cast<size_t> (selectedModule)];
    g.setColour (pearl);
    g.setFont (juce::Font (23.0f, juce::Font::bold));
    g.drawText (bank.name, 38, 418, 222, 34, juce::Justification::left);
    g.setColour (muted);
    g.setFont (juce::Font (12.0f));
    g.drawText (bank.description, 38, 457, 554, 19, juce::Justification::left);
    g.drawText ("EFFECT PRESET  •  changes only this module", 560, 419, 400, 20,
                juce::Justification::left);
    paintSelectedModuleMonitor (g);
    g.setColour (muted);
    g.setFont (juce::Font (11.0f));
    g.drawText ("LIVE AUDIO • PRESETS ARE EDITABLE • YOUR ORIGINAL DREAMVOX IS SAFE",
                38, 686, 900, 24, juce::Justification::left);
}

void DreamVoxAudioProcessorEditor::resized()
{
    if (controls.size() != 20) return;
    presetSelector.setBounds (610, 67, 280, 30);
    bypass.setBounds (970, 66, 118, 31);
    for (int i = 0; i < 8; ++i)
        moduleButtons[static_cast<size_t> (i)].setBounds (20 + i * 136, 336, 128, 63);
    effectPresetSelector.setBounds (568, 446, 352, 31);
    for (int j = 0; j < moduleCounts[selectedModule]; ++j)
    {
        const int id = moduleKnobs[selectedModule][j];
        const int x = 53 + j * 176;
        sectionLabels[id]->setBounds (x - 3, 496, 114, 18);
        controls[id]->setBounds (x, 518, 112, 123);
    }
    sectionLabels[7]->setBounds (990, 492, 102, 18);
    controls[7]->setBounds (990, 515, 101, 126);
}
