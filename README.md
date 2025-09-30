# gmu
video game music file cli player. See [libgme](https://github.com/mcfiredrill/libgme) for supported formats.

![screenshot](Screenshot-2024-02-21.png)

## Dependencies
- [libgme](https://github.com/mcfiredrill/libgme)
    - macOS: `brew install libgme`
    - Ubuntu/Debian: `apt install libgme-dev`
- [portaudio](https://www.portaudio.com/)
    - macOS: `brew install portaudio`
    - Ubuntu/Debian: `apt install portaudio19-dev`

## Build
Pull git submodules: `git submodule update --init --recursive`. Then run `make`.

## Controls
```
n next track
b prev track
p pause
. skip forward 5 seconds
, skip backwards 5 seconds
L toggle loop
SPC toggle play
```

## Known Issues
- tiny tracks (less than 4 seconds) fail to fadeout
- VGM/VGZ files that are not SMS/Genesis output silence (likely unsupported by libgme)
    - TODO: Swap out back end for vgmplay?
