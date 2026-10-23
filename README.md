# gdk-audio
A 3D audio interface for games. The null implementation is included; other implementations are provided by separate libraries.
See [gdk-audio_openal](https://github.com/jfcameron/gdk-audio_openal), for the OpenAL implementation.

## Overview
### Entry point
A context is an instance of the library, this is where the "global state" of a single instantiation of the library exists.

### The 3d scene
The library is structured around 3d scenes. Each scene contains a single listener and any number of emitters. The listener represents the point from which sounds are heard, while emitters represent the positions in 3D space from which sounds originate.

### Audio data
A sound is an audio resource, it could be a foot step, or a song, whatever. It could contain raw PCM, it could be a buffer being streamed to from somewhere outside the library, it could be coming from a microphone, or it could own a decoder that it uses to stream some kind of encoded audio buffer on the fly. The library provides an ogg-vorbis decoder implementation in the ext header path, both as a convenience and also as a reference for writing your own encoded sound implementations.

### Sound effects
Currently user-defined sound effects are limited to insert effects, which means that whatever processing is done on the audio data is done before spatialization.
