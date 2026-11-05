/* Tennix's config.h for AmigaOS 3.x (configure is not run there).
 * SDL2, SDL2_image, SDL2_ttf and SDL2_mixer come from OpenGPU's kit and
 * SDL2_gfx's rotozoomer from amiga/deps/SDL2_gfx-1.0.4. No SDL2_net
 * (network play off), no Python bots, no voice files. */
#define HAVE_SDL_IMAGE
#define HAVE_SDL_TTF
#undef HAVE_SDL_NET
#define HAVE_SDL_MIXER
#define HAVE_SDL_GFX
#undef HAVE_PYTHON
#undef HAVE_VOICE_FILES
#define VERSION "1.3.4"
#define PREFIX "PROGDIR:"
