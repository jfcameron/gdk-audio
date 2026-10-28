# gdk-audio
The gdk audio interface: 3D scenes of emitters heard by a listener, sounds decoded on the fly, insert effects, and microphones. It carries no implementation but a null one, for tests.

API documentation: https://jfcameron.github.io/gdk-audio/

Implementations:
- [gdk-audio_openal](https://github.com/jfcameron/gdk-audio_openal): OpenAL, on desktop, mobile and the browser

## Getting Started
Install cmake 3.21 or greater, and a c++20 compatible compiler
```
# initialize the tree
git submodule update --init --recursive

# generate the script to build the library and its tests on linux using gcc
cmake --preset linux-gcc

# build the library and tests
cmake --build --preset linux-gcc

# run the tests
ctest --preset linux-gcc
```
See `CMakePresets.json` for the full list of supported platforms and compilers. See `.github/workflows/ci.yml` to see how CI runners build it on different platforms.

To use it from another project, add it with `add_subdirectory` and link `gdk::audio` (and `gdk::audio_vorbis` for Ogg Vorbis), or install it and `find_package(gdk-audio)`. Positions and rotations are gdk-math's, `gdk::math::vector3<float>` and `gdk::math::quaternion<float>`.

## Overview
### Entry point
A context is an instance of the library, this is where the "global state" of a single instantiation of the library exists.

### The 3d scene
The library is structured around 3d scenes. Each scene contains a single listener and any number of emitters. The listener represents the point from which sounds are heard, while emitters represent the positions in 3D space from which sounds originate.

### Audio data
A sound is an audio resource, it could be a foot step, or a song, whatever. It could contain raw PCM, it could be a buffer being streamed to from somewhere outside the library, it could be coming from a microphone, or it could own a decoder that it uses to stream some kind of encoded audio buffer on the fly. The library provides an ogg-vorbis decoder implementation in the ext header path, both as a convenience and also as a reference for writing your own encoded sound implementations.

### Sound effects
Currently user-defined sound effects are limited to insert effects, which means that whatever processing is done on the audio data is done before spatialization.

## Conformance
`gdkaudio_conformance` is the suite every implementation passes. An implementation's test target links it and defines `gdk::audio::conformance::make_context()` and `implementation()` (see `test/conformance/conformance.h`); the null implementation runs it here.

`gdkaudio_implementation` gives an implementation the private helpers the null one uses too (the microphone's capture buffer and its request checks). It is a build-time target and is never installed.
