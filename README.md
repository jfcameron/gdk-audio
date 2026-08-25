## gdk-audio

3D audio interface library for games. Besides null, implementations are libraries of their own.
See [gdk-audio_openal](https://github.com/jfcameron/gdk-audio_openal), for the OpenAL implementation. Requires C++20.

## Building

```
cmake --preset linux-clang
cmake --build --preset linux-clang
ctest --preset linux-clang
```

`CMakePresets.json` lists the supported builds.

