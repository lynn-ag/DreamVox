# DreamMaster v0.3 — mastering-engine upgrade specification

This branch preserves v0.2.1 and the Suno presets. Do not merge until audio tests and a Windows VST3 build pass.

## Verified current implementation
- `Source/MasterEngine.h`: 5 ms lookahead, sample-peak stereo-linked limiter, gentle compressor, EQ, smoothing, stereo width and bypass.
- `Source/PluginProcessor.cpp`: 10 mastering controls and peak/RMS/reduction telemetry.
- `Tests/EngineTests.cpp`: sample-ceiling, bypass, block-size invariance, presets, mono/stereo, sample rates, nonfinite-input checks.
- The existing sample-peak ceiling is **not** a true-peak guarantee. RMS is **not** LUFS.

## Delivery milestones (in dependency order)
1. Add independent ITU-R BS.1770-5 K-weighted loudness analysis (400 ms momentary, 3 s short-term, gated integrated). Verify against published reference signals; never label RMS as LUFS.
2. Add oversampled true-peak detection and output limiting (4x minimum; selectable higher-quality offline mode). Test intersample overshoots, high-frequency sine waves, impulses, clipping, stereo link, latency and ceiling tolerances.
3. Add loudness-matched A/B with latency-aligned dry signal and independent bypass. Keep automation clicks and plugin-host bypass transitions artifact-free.
4. Add adjustable limiter timing, transient preservation and reliable gain-reduction metering; run null/THD/aliasing/CPU tests.
5. Add dynamic tonal correction, low-frequency mono control, stereo correlation meter, gentle multiband dynamics; default to minimal changes.
6. Add reference analysis and suggested parameter settings. Do not claim neural AI mastering until an actual trained inference backend is implemented and validated.
7. Add Suno-specific profiles as starting points for adaptive analysis, not claims that artifacts are automatically removable.
8. Windows x64 VST3 release build and FL Studio validation with mono/stereo, automation, preset recall, sample-rate switching and project reopening.

## Non-negotiable acceptance criteria
- No audio-thread allocation, blocking I/O, network, or locks.
- No NaN/Inf output; correct state recall and sample-rate changes.
- Measured true-peak tolerance documented and checked by an independent reference meter.
- LUFS checked against EBU/ITU reference material.
- A/B comparisons level-matched; no default excessive limiting or ambience collapse.
- CPU usage measured on a Windows Intel-graphics 16 GB RAM machine; safe offline-first processing.
- Every effect has audible, tested DSP and a bypass; no nonfunctional UI controls.
- Keep all existing Suno presets, with backwards-compatible state identifiers.
- Do not replace the stable build before a passing CI build and listening tests.

## Standards
- ITU-R BS.1770-5: https://www.itu.int/rec/R-REC-BS.1770-5-202311-I/en
- EBU R 128: https://tech.ebu.ch/publications/r128

## Status
Planning and branch isolation complete. DSP features in this document are **not implemented yet**.
