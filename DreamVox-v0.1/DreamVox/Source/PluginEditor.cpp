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


namespace
{
constexpr const char* subtitles[] = {
    "CLARITY", "DYNAMICS", "SHAPE", "SMOOTH",
    "CHARACTER", "AMBIENCE", "DELAY", "WIDTH"
};

void drawCardSymbol (juce::Graphics& g, int index, float cx, float cy)
{
    const auto hotPink = juce::Colour (0xffff8bdc);
    const auto iceBlue = juce::Colour (0xff96b7ff);
    const auto lavender = juce::Colour (0xffc58dff);
    g.setColour (index == 0 ? iceBlue : index == 5 ? lavender : hotPink);
    switch (index)
    {
        case 0: // Faceted diamond
        {
            juce::Path diamond;
            diamond.startNewSubPath (cx, cy - 29.0f);
            diamond.lineTo (cx + 24.0f, cy - 4.0f);
            diamond.lineTo (cx, cy + 28.0f);
            diamond.lineTo (cx - 24.0f, cy - 4.0f);
            diamond.closeSubPath();
            g.fillPath (diamond);
            g.setColour (juce::Colour (0xff403e83));
            g.drawLine (cx, cy - 27, cx, cy + 27, 1.6f);
            g.drawLine (cx - 22, cy - 4, cx + 22, cy - 4, 1.6f);
            break;
        }
        case 1: // Pearl compressor orb
        {
            g.setGradientFill (juce::ColourGradient (juce::Colour (0xffffd0ed), cx - 17, cy - 21,
                                                    juce::Colour (0xffbe498f), cx + 22, cy + 25, false));
            g.fillEllipse (cx - 27, cy - 27, 54.0f, 54.0f);
            g.setColour (juce::Colour (0xaaeea4ff));
            g.drawEllipse (cx - 29, cy - 29, 58, 58, 2.0f);
            g.setColour (juce::Colour (0xb9ffffff));
            g.fillEllipse (cx - 15, cy - 19, 14, 8);
            break;
        }
        case 2: // Butterfly equaliser
        {
            g.setColour (hotPink);
            juce::Path wing;
            wing.addEllipse (cx - 28, cy - 19, 27, 27);
            g.fillPath (wing, juce::AffineTransform::rotation (-0.38f, cx - 15.0f, cy - 5.5f));
            juce::Path other;
            other.addEllipse (cx + 1, cy - 19, 27, 27);
            g.fillPath (other, juce::AffineTransform::rotation (0.38f, cx + 14.5f, cy - 5.5f));
            g.setColour (lavender);
            g.fillEllipse (cx - 22, cy + 1, 21, 24);
            g.fillEllipse (cx + 1, cy + 1, 21, 24);
            g.setColour (juce::Colour (0xfffbe4ff));
            g.drawLine (cx, cy - 14, cx, cy + 19, 2.5f);
            break;
        }
        case 3: // Vocal de-esser waveform
        {
            for (int i = 0; i < 13; ++i)
            {
                const float heights[] = {8, 20, 12, 34, 48, 25, 60, 36, 51, 28, 16, 25, 9};
                const float h = heights[i];
                g.drawLine (cx - 30.0f + i * 5.0f, cy - h * 0.5f,
                            cx - 30.0f + i * 5.0f, cy + h * 0.5f, 2.6f);
            }
            break;
        }
        case 4: // Heart tone character
        {
            juce::Path heart;
            heart.startNewSubPath (cx, cy + 26);
            heart.cubicTo (cx - 36, cy + 5, cx - 33, cy - 25, cx - 14, cy - 25);
            heart.cubicTo (cx - 4, cy - 25, cx, cy - 16, cx, cy - 12);
            heart.cubicTo (cx + 5, cy - 22, cx + 13, cy - 28, cx + 22, cy - 23);
            heart.cubicTo (cx + 42, cy - 11, cx + 25, cy + 11, cx, cy + 26);
            heart.closeSubPath();
            g.fillPath (heart);
            g.setColour (juce::Colour (0x8cffffff));
            g.drawLine (cx - 15, cy - 7, cx + 7, cy + 12, 1.7f);
            break;
        }
        case 5: // Moonlit space cloud
            g.setColour (juce::Colour (0xffa8a2fc));
            g.fillEllipse (cx - 29, cy - 5, 35, 32);
            g.fillEllipse (cx - 12, cy - 21, 34, 40);
            g.fillEllipse (cx + 9, cy - 8, 27, 32);
            g.fillRoundedRectangle (cx - 27, cy + 4, 60, 20, 10);
            break;
        case 6: // Saturn for the echo engine
        {
            g.setColour (juce::Colour (0xffef72d8));
            g.fillEllipse (cx - 23, cy - 23, 46, 46);
            juce::Path ring;
            ring.addEllipse (cx - 37, cy - 10, 74, 20);
            g.setColour (juce::Colour (0xffebb5ff));
            g.strokePath (ring, juce::PathStrokeType (3.0f),
                          juce::AffineTransform::rotation (-0.35f, cx, cy));
            break;
        }
        default: // two gently offset voices
            g.drawEllipse (cx - 33, cy - 22, 49, 44, 3.0f);
            g.drawEllipse (cx - 9, cy - 22, 49, 44, 3.0f);
            break;
    }
}
}

void DreamVoxEffectCard::paintButton (juce::Graphics& g, bool highlighted, bool down)
{
    const auto r = getLocalBounds().toFloat().reduced (1.8f);
    const auto edge = selected ? juce::Colour (0xffff9cde) : juce::Colour (0xff7c477a);
    const auto c1 = selected ? juce::Colour (0xff31132e) : juce::Colour (0xff1a1026);
    const auto c2 = down ? juce::Colour (0xff5d214d) : juce::Colour (0xff0c0718);
    g.setGradientFill (juce::ColourGradient (c1, 0, 0, c2, 0, r.getBottom(), false));
    g.fillRoundedRectangle (r, 14.0f);
    if (selected || highlighted)
    {
        g.setColour (juce::Colour (0x45ff8bcb));
        g.drawRoundedRectangle (r.reduced (3), 12.0f, 4.0f);
    }
    g.setColour (edge);
    g.drawRoundedRectangle (r, 14.0f, selected ? 2.5f : 1.1f);

    // Real button selection, with the jewel art rendered live in JUCE.
    const float cx = r.getCentreX(), cy = r.getY() + 48.0f;
    g.setColour (juce::Colour (0x32ff8bdc));
    g.fillEllipse (cx - 35.0f, cy - 35.0f, 70.0f, 70.0f);
    drawCardSymbol (g, moduleIndex, cx, cy);
    g.setColour (juce::Colour (0xfffff3ff));
    g.setFont (juce::Font (14.0f, juce::Font::bold));
    g.drawText (getButtonText(), 3, static_cast<int> (r.getY() + 89), getWidth() - 6, 19,
                juce::Justification::centred);
    g.setColour (juce::Colour (0xffcba1ce));
    g.setFont (juce::Font (10.0f));
    g.drawText (subtitles[moduleIndex], 3, static_cast<int> (r.getY() + 109),
                getWidth() - 6, 14, juce::Justification::centred);
    if (selected)
    {
        g.setColour (juce::Colour (0xffff80ce));
        g.fillRoundedRectangle (getWidth() * 0.33f, r.getBottom() - 4.0f,
                                getWidth() * 0.34f, 2.0f, 1.0f);
    }
}

void DreamVoxKnobLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y,
                                                int width, int height, float position,
                                                float start, float end, juce::Slider&)
{
    const auto centre = juce::Point<float> (x + width * 0.5f, y + height * 0.47f);
    const float radius = juce::jmin (width * 0.38f, height * 0.40f);
    g.setColour (juce::Colour (0x404e2c67));
    g.fillEllipse (centre.x - radius - 8, centre.y - radius - 8,
                   (radius + 8) * 2, (radius + 8) * 2);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff30263d),
                                           centre.x - radius, centre.y - radius,
                                           juce::Colour (0xff080812),
                                           centre.x + radius, centre.y + radius, false));
    g.fillEllipse (centre.x - radius, centre.y - radius, radius * 2, radius * 2);
    juce::Path background, foreground;
    background.addCentredArc (centre.x, centre.y, radius + 4, radius + 4, 0,
                              start, end, true);
    const float angle = start + position * (end - start);
    foreground.addCentredArc (centre.x, centre.y, radius + 4, radius + 4, 0,
                              start, angle, true);
    g.setColour (juce::Colour (0xff553950));
    g.strokePath (background, juce::PathStrokeType (4.0f));
    g.setColour (juce::Colour (0xffff85cb));
    g.strokePath (foreground, juce::PathStrokeType (4.0f));
    g.setColour (juce::Colour (0xfff4e7ff));
    const auto thumbPoint = centre.getPointOnCircumference (radius * 0.75f, angle);
    g.fillEllipse (thumbPoint.x - 3.5f, thumbPoint.y - 3.5f, 7.0f, 7.0f);
    g.setColour (juce::Colour (0x55edc6fb));
    g.drawEllipse (centre.x - radius, centre.y - radius, radius * 2, radius * 2, 0.8f);
}

DreamVoxAudioProcessorEditor::DreamVoxAudioProcessorEditor (DreamVoxAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    for (int i = 0; i < 20; ++i)
    {
        auto* slider = controls.add (new juce::Slider());
        slider->setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider->setLookAndFeel (&knobLook);
        slider->setRotaryParameters (juce::MathConstants<float>::pi * 1.25f,
                                      juce::MathConstants<float>::pi * 2.75f, true);
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
        button.moduleIndex = i;
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
    setSize (1120, 740);
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
    {
        moduleButtons[static_cast<size_t> (i)].selected = i == selectedModule;
        moduleButtons[static_cast<size_t> (i)].repaint();
    }

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
