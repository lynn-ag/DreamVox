# DreamMaster 0.1.0

A Windows x64 VST3 mastering rack for stereo mixes. It has a distinct plug-in ID
from DreamVox and can be used alongside it. This is a first version with gentle
controls, rather than an automatic mastering or source-repair system.

Signal path: input trim, optional 22 Hz high-pass, 100 Hz bass shelf, broad
320 Hz clarity cut, 10 kHz air shelf, dynamic 3.8 kHz smoothing, stereo width
above the low-bass region, gentle linked compression, loudness gain, and a
stereo-linked sample-peak limiter with 5 ms lookahead.\n\nAutomatic mastering is enabled by default in the saved state. It uses the existing real-time envelope and broad-band energy followers, is bounded to gentle tonal moves, and does not attempt to copy a reference. The state switch is intentionally not exposed in this UI pass; manual controls remain available. Internal bypass can use approximate loudness matching for fair A/B comparison.

The ceiling is in dBFS, not dBTP. This version does not provide true-peak limiting
or LUFS metering. Its output meters display sample peaks and RMS. Leave headroom
and check a true-peak meter after processing and after lossy encoding.

## Start here

Insert DreamMaster near the end of the master chain. Begin with Clean start or
Gentle polish and Loudness at 0 dB. Use small Bass/Air changes. Clarity gently
reduces low-mid buildup; Smooth dynamically reduces a harsh mid-high band.
Glue has no auto makeup and is capped at 3 dB gain reduction. Width is 100% by
default; widening cannot fix phase problems in the original mix.

Compare at similar listening loudness. Bypass preserves latency but does not
automatically match loudness, and bypassed peaks are not limited. A constantly
high LIMIT reduction reading is a reason to turn down Loudness or Input.
The optional Low cut changes sub-bass; enable only when useful.

Presets are starting points, not song-specific mastering decisions. The plugin
cannot recover missing detail, undo clipping, remove all generated artifacts,
separate instruments, or replace listening. No audio files are sent anywhere.

## Build and validation

JUCE 8.0.13, CMake, C++17, Visual Studio 2022 x64. The workflow builds the VST3,
runs DSP tests at 8/44.1/48/96/192 kHz in mono and stereo, and runs pluginval
1.0.4 level 5 three times including GUI tests before publishing the ZIP.
The source includes a local offline renderer using the exact DSP engine.

Plugin IDs and parameter IDs should remain stable in future versions. Saved
state uses AudioProcessorValueTreeState. Parameter changes are smoothed; bypass
and low-cut changes crossfade; processing does not allocate buffers.

JUCE is a separate dependency: review its licensing requirements before public
commercial distribution. No third-party brand emulation or endorsement is claimed.
