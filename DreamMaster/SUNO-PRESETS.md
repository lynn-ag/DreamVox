# DreamMaster 0.2.1 — Suno starting points

This version adds eight optional presets to the existing five. These are manual starting points designed around common mix symptoms, not settings measured from your songs. No Suno audio was supplied for tuning. They do not add AI analysis, LUFS measurement, true-peak limiting, or repair of clipped/generated artifacts.

## Choose an option
- **Suno - Gentle start:** Begin here for mild cleanup.
- **Suno - Warm R&B:** For bright R&B mixes needing gentle warmth and smoother upper mids.
- **Suno - Balanced clarity:** For muddy low mids; adds no high-frequency boost.
- **Suno - Punchy pop:** Light glue and 102% width; no added limiter drive. It does not restore lost transients.
- **Suno - Bass control:** For boomy bass, with a 22 Hz low cut. Check that kick/bass weight is retained.
- **Suno - Soften bright vocals:** For sharp vocal/cymbal energy; reduces air and increases Smooth.
- **Suno - Already loud:** Input trim only. It cannot reverse clipping or restore compressed dynamics.
- **Suno - Airy acoustic:** A small air lift for dull acoustic mixes; avoid on brittle highs.

## Use in FL Studio
1. Close FL Studio and back up your existing complete DreamMaster.vst3 folder outside the scan folders.
2. Extract this ZIP and replace the COMPLETE DreamMaster.vst3 folder in `C:/Program Files/Common Files/VST3/`.
3. Rescan with verification enabled. The plug-in retains its identity and existing parameter IDs.
4. Open a copy of your project and select a starting point from the preset menu.
5. Listen to a verse and the loudest chorus. Compare at a similar listening level; bypass is latency-matched but does not match loudness.
6. Keep Loudness at 0 dB initially. Watch limiter reduction. Avoid repeated mastering if your export is already heavily processed.
7. Save and reopen the project to confirm recall. Restore the backed-up bundle to roll back.

The eight Suno presets use a -1.5 dBFS SAMPLE-PEAK ceiling and no positive limiter drive. Most start with -1.5 dB input trim; Already loud uses -3 dB. This leaves room for processing but is not a true-peak or streaming compliance guarantee. All controls remain editable. Presets replace processing values but preserve the current bypass setting; turn bypass off to hear the result.

The interface shows a description of the last chosen starting point. It is guidance, not an indication that your edited settings still equal that preset. The original five presets and their values remain available. Existing projects keep their saved settings.

## Verification
The build runs engine regression tests, a stress test for all 13 presets at four sample rates in mono/stereo, checks all preset menu choices and saved parameter recall, captures three editor sizes, and runs pluginval at strictness 5. See the bundled logs for actual results. These checks do not establish subjective sound quality or guarantee behavior in every DAW.

## Next tuning step
Use two or three original Suno WAV exports, with genre and the issue you hear in each. Tune against those files and compare at matched loudness. Do not describe these presets as song-matched until that work is done.
