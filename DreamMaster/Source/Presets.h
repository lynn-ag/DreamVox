#pragma once
#include "MasterEngine.h"
namespace dream {
inline constexpr int presetCount=13;
inline const char* presetName(int index)
{
    constexpr const char* names[]={"Clean start","Gentle polish","Warm & smooth","Keep the punch","Soften the top",
        "Suno - Gentle start","Suno - Warm R&B","Suno - Balanced clarity","Suno - Punchy pop",
        "Suno - Bass control","Suno - Soften bright vocals","Suno - Already loud","Suno - Airy acoustic"};
    return names[index>=0&&index<presetCount?index:0];
}
inline const char* presetHint(int index)
{
    constexpr const char* hints[]={
        "Neutral controls; peaks above the ceiling are still limited.",
        "Subtle tone and dynamics adjustments. Compare with bypass.",
        "A little warmth with gentle low-mid and high-frequency control.",
        "Light compression and a small gain lift; watch limiter reduction.",
        "For a bright mix: less air and stronger harshness control.",
        "Start here. Mild cleanup, no added loudness or widening.",
        "For bright R&B mixes: gentle warmth and smoother upper mids.",
        "For muddy mixes: restrained low-mid cut without extra brightness.",
        "For pop: light glue and width, no added limiter drive.",
        "For boomy mixes: reduce bass and low mids. Check kick weight.",
        "For sharp vocals or cymbals: reduce air and dynamic harshness.",
        "For dense, already-loud mixes: input trim only; cannot undo clipping.",
        "For dull acoustic mixes: a small air lift. Avoid on brittle highs."};
    return hints[index>=0&&index<presetCount?index:0];
}
inline Settings preset(int index)
{
    Settings s;
    // Preserve the original five indices and values for compatibility.
    if(index==1) { s.clarity=12;s.smooth=15;s.air=-.3;s.glue=12; }
    if(index==2) { s.bass=.5;s.clarity=20;s.smooth=25;s.air=-.5;s.glue=20;s.drive=.5; }
    if(index==3) { s.bass=.4;s.clarity=15;s.smooth=10;s.glue=8;s.width=102;s.drive=1; }
    if(index==4) { s.clarity=10;s.smooth=45;s.air=-1;s.glue=10; }
    // These are manual starting points, not analysis-derived corrections.
    // No Suno preset adds limiter drive. Lower input leaves room for small boosts.
    if(index>=5&&index<presetCount) { s.input=-1.5;s.ceiling=-1.5; }
    if(index==5) { s.clarity=8;s.smooth=12; }
    if(index==6) { s.bass=.3;s.clarity=15;s.smooth=30;s.air=-.6;s.glue=8; }
    if(index==7) { s.bass=-.2;s.clarity=25;s.smooth=15;s.glue=5; }
    if(index==8) { s.bass=.3;s.clarity=12;s.smooth=15;s.glue=8;s.width=102; }
    if(index==9) { s.bass=-1.2;s.clarity=30;s.smooth=10;s.lowCut=true; }
    if(index==10) { s.clarity=8;s.smooth=45;s.air=-1; }
    if(index==11) { s.input=-3; }
    if(index==12) { s.clarity=10;s.smooth=8;s.air=.5; }
    return s;
}
}
