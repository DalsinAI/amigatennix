/* A Shell gives 4 KB of stack; libnix's startup sets this much up instead
 * (SDL_ttf's FreeType and the C++ runtime want more than that).
 * The AmiCradle Team. GPL 2 or later, like Tennix. */
unsigned long __stack = 256 * 1024;
static const char version[] __attribute__((used)) = "$VER: Tennix 1.3.4 Amiga (05.11.2026)";
