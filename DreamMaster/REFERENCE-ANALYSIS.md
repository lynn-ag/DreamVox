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
