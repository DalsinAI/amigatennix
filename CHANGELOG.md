# ChangeLog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## 1.3.4 (2023-07-07)

### Fixed

- `configure`: Search for dependencies in `--prefix` (`<prefix>/lib` and `<prefix>/include`)

## 1.3.3 (2023-06-29)

### Fixed

- `configure`: Fix detection of working compiler and library/header search
- `configure`: Fix `SDL2_rotozoom.h` header name

## 1.3.2 (2023-06-28)

### Changed

- Port from SDL 1.2 to SDL 2, including all libraries (image, mixer, ttf, net)
- Fullscreen toggle is now <kbd>V</kbd> instead of <kbd>F</kbd>
- Python 3.8 is now required/recommended (bumped from version 3.3)

### Removed

- `SDL_rotozoom.c` / `SDL_rotozoom.h` (replaced with `SDL2_gfx` dependency)
