# DreamVox

DreamVox is a first-pass 64-bit VST3 vocal rack for FL Studio, aimed at dreamy
R&B and pop vocals. It combines three broad tone controls, compression,
high-frequency de-essing, soft saturation, output level, host automation, and
saved plug-in state in a custom dark violet and aqua interface.

## Build

The project pins JUCE 8.0.13. On Windows with Visual Studio 2022 and CMake,
configure and build with:

```powershell
cmake -S DreamVox -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target DreamVox_VST3
```

The automated workflow in `.github/workflows/build-dreamvox.yml` does the
Windows build and publishes a ZIP artifact. See [INSTALL.txt](INSTALL.txt) for
the no-local-toolchain path and installation steps.

## Controls

| Control | What it does |
| --- | --- |
| Low | Low shelf centered around 180 Hz |
| Body | Broad presence bell centered around 1.1 kHz |
| Air | High shelf centered around 9 kHz |
| Threshold / Ratio | Compressor threshold and ratio |
| De-Ess | Dynamic reduction of high-frequency content |
| Warmth | Soft saturation blend |
| Output | Post-processing gain |
| Bypass | Bypasses the complete rack |

The compressor uses a linked detector, with a 12 ms attack and 120 ms release.
The de-esser targets a broad band above roughly 6.2 kHz. These settings are
starting points, not a substitute for listening and level matching.

## Licensing

The project depends on JUCE 8. JUCE 8 is available under its AGPLv3 terms or a
commercial JUCE license; comply with the applicable JUCE terms before sharing
or distributing a built plugin. This package does not include a JUCE
commercial license.

## Status

Prototype v0.1. The DSP and user interface have not been compiled or validated
inside FL Studio in this environment. The Windows workflow is included to
produce the VST3 bundle; validate the resulting build in a DAW before relying
on it in a release session.
