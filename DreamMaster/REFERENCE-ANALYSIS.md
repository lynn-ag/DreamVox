# DreamMaster reference analyzer (v0.3 development preview)

DreamMasterAnalyze is a **read-only offline audio analyzer**, built to establish accurate, reusable metrics for future reference matching and Personal Sonic DNA. **It does not currently make automatic mastering decisions, learn preferences, or modify any audio.**

## Windows usage

Download the `DreamMaster-test-tools` artifact from a **successful** DreamMaster development Windows build. Extract it. Drag a finished stereo or mono WAV file onto `Analyze-Reference.bat`. The tool creates a `<source>.dreammaster-analysis.json` file beside your original audio, but does not overwrite an existing analysis or your source.

It can also be used in a command prompt:

```powershell
DreamMasterAnalyze.exe "C:\\Music\\My Finished Mix.wav"
```

Use full-length, uncompressed WAV references whenever available. Other formats depend on the JUCE audio decoders included in the build.

## JSON metrics

- `integrated_lufs` — gated K-weighted integrated program loudness
- `short_term_lufs` and `momentary_lufs` — values **at the end** of the file, not the full-song maxima
- `max_true_peak_dbtp` — 4x reconstructed-peak estimate across the entire file
- `sample_peak_dbfs`, `rms_dbfs` — full-file values
- `stereo_correlation` — whole-file L/R correlation (mono reported as 0)
- `loudness_valid` — false for files shorter than 400ms
- `duration_seconds`, `sample_rate`, `channels`

Meters need further independent published BS.1770 conformance validation before their outputs should be considered compliance-certified. Stereo correlation and loudness by themselves do not measure dreaminess or musical quality.

## Privacy and cost

There is no cloud dependency, network connection, subscription, automatic upload or payment requirement. Reference matching, Sonic DNA profiles, AI optimization, and optional consent-based cloud enhancement are future development stages. They must remain opt-in and usable without subscription.

## Broad spectral signature (development)
The analyzer also measures nine-band relative power using an 8192-sample Hann-window FFT with 50% overlap. Band boundaries (Hz): 30, 80, 160, 315, 630, 1250, 2500, 5000, 10000, 20000. `spectral_bands_db` expresses each band's average power per FFT bin relative to the overall mean; unavailable bands are `null`. Stereo is analyzed per channel to avoid cancellation. `spectral_valid` indicates whether the audio was long enough for an FFT window.

These are descriptive measurements, **not automatically safe EQ corrections**. Reference matching must avoid overfitting different instruments and arrangements.

## Build a Personal Sonic DNA reference profile
Create local analysis reports from **at least two** approved masters, preferably three to five stylistically related finished songs. Drag their `.dreammaster-analysis.json` files together onto `Create-Sonic-DNA.bat`, which uses the included `DreamMasterProfile.exe` and writes `DreamMaster-SonicDNA.json` in the tool folder without overwriting an existing profile. No audio is transmitted.

The profile rejects identical feature sets (including copied analysis files) as independent references. The profile stores robust per-band medians and median absolute deviations (MAD), number of usable references in each band, and median loudness/stereo measurements. Variability warns that different tracks may not share one tonal target. This is **descriptive profile creation, not trained neural AI, not automatic EQ, and not completed reference-based mastering**. Future mastering recommendations must apply conservative limits and be reviewed by ear.

## Compare a new song against Sonic DNA (read-only)

Build `DreamMaster-SonicDNA.json` from two or more **different** approved masters. Analyze a new mix with `Analyze-Reference.bat`. Keep the profile next to `DreamMasterMatch.exe`, then drag the new `.dreammaster-analysis.json` file onto `Compare-Sonic-DNA.bat`. The utility writes a new non-overwriting `.dreammaster-match.json` beside the analysis report.

The JSON contains each band's spectral difference, a 0–1 confidence estimate, an uncertainty-aware **conservative tonal hint** capped at ±1.5 dB, and descriptive loudness/correlation differences. Bands with fewer than two useful references, very low energy, or high variability do not produce speculative corrections.

**This is not automatic EQ or trained AI.** Frequency differences may come from instrument choices, arrangement and artistic intent; the hints are for manual consideration at matched playback loudness. Audio files are never uploaded or altered.
