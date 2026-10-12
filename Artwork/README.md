# Interface artwork

Run `python Artwork/prepare-artwork.py` from the repository root before CMake.
The preparation script verifies the two original PNGs with SHA-256.

VelvetNights.png and MidnightHorizon.png are the interface references supplied by the project owner on 12 October 2026. The native editors reuse their skyline, branding and module illustrations. Controls are rendered by JUCE and bound to the existing audio parameters; screenshots are not used as a fake interactive interface.

This update retains the existing DSP and plugin identities. Only implemented controls are shown. DreamVox has three-band tone shaping, compression, de-essing, saturation and output gain. DreamMaster has input trim, low cut, bass, clarity, smoothing, air, glue, width, loudness and a sample-peak ceiling, with live peak/RMS and gain-reduction meters. The references' reverb, echo, doubler, multiband controls, LUFS and true-peak functions are not implemented in these versions.
