# EMBER 175 Reference Lab (VST3)

An independent DSP research project exploring a high-ratio, tube-character compressor workflow. It is **not** a Universal Audio product, and it does not claim circuit-accurate or perceptually identical emulation.

## Build

Push this repository to GitHub. The `Build and test VST3` workflow uses Windows Server 2022 + Visual Studio 2022, fetches JUCE 8.0.6 with CMake, builds the VST3 and DSP sanity tests, runs tests, and uploads a ZIP artifact.

The build has not been run from this generated workspace; treat the first GitHub Actions run as the integration check and fix any toolchain issues surfaced there.

## DSP architecture

- Input/headroom gain stage
- Feedback-inspired envelope estimate and fixed high-ratio gain reduction
- Attack/release smoothing
- Controllable nonlinear color stage
- Parallel dry/wet blend and output compensation
- Parameter state persistence via JUCE APVTS

This is a **research baseline**, not a measured model of the 175B circuit. In particular, the current nonlinear stage is a simple saturator, stereo linking is not yet fully implemented as a selectable mode, and the detector/gain law requires tuning against measured reference material.

## Validation roadmap

1. Capture dry and processed outputs from a legally available reference plug-in or hardware at documented sample rate, bit depth, settings, and input level.
2. Align files to sub-sample precision and loudness-match before listening tests.
3. Measure static transfer/gain-reduction curves with stepped sine and burst signals.
4. Measure attack/release response using gated tone bursts and amplitude steps.
5. Measure THD+N and harmonic spectra at multiple levels and frequencies.
6. Measure intermodulation, frequency response, noise, and aliasing at 44.1/48/96 kHz.
7. Run randomized, level-matched ABX trials; log settings and results.
8. Change one DSP subsystem at a time and preserve regression fixtures.

## Required reference data

For meaningful similarity claims, supply captures from the reference device/plugin. Public product descriptions identify features such as the 175B's fixed 12:1 ratio, independent attack/release, input/output controls, headroom and dry/wet, but do not reveal the full transfer functions or circuit implementation. A reference capture set is needed to estimate those parameters.

## JUCE licensing

JUCE is offered under AGPLv3 or a commercial licence. Review current licensing terms before distributing closed-source or commercial binaries.


## User interface
The editor now has a resizable, dark-brass hardware-inspired layout, eight rotary controls, parameter attachments, bypass/stereo-link toggles, and a live gain-reduction bar. The UI is an original design and is not an official Universal Audio interface. The DSP is still a research baseline and has not been verified as a sonic match to the UA 175B.
