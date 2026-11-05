Tennix for AmigaOS 3.x
======================

A port of Tennix! Classic Championship Tour 2011 by Thomas Perl
(http://icculus.org/tennix/, release 1.3.4, GPL-2.0 or later), the 2D
tennis game, for AmigaOS 3.x on a 68040 or 68060 with an FPU and an RTG
screen, on OpenGPU's SDL 2.

Base: upstream's release tarball tennix-1.3.4, taken as Debian's
tennix_1.3.4.orig.tar.xz (https://deb.debian.org/debian/pool/main/t/tennix/,
sha256 63e1016363c26ff0e07188ca2b3b9560bf9dfe509960bca2534cdbdb08d34fa5),
imported unchanged as commit bb58e632c675b224cb08c79521bb74929d830496
(tag v1.3.4). Upstream's git (https://repo.or.cz/tennix.git) could not be
reached from here; https://github.com/thp/tennix does not exist.

Status: plays in the lab (Instance-45, windowed 640x480): menu, world
map, controller setup, a match against the computer, Esc and Resume.
The pointer fix and the quicker quit below are built, not yet re-tested.


What you need
-------------

- A 68040 or 68060 with an FPU (not an LC040 or EC040), AmigaOS 3.x, an RTG
  card (or the AmigaChrome lab).
- OpenGPU 0.6 or later: opengpu.library, LIBS:OpenGPU/SDL2.module,
  SDL2_image.module and SDL2_mixer.module (installed by OpenUp).
- About 3.2 MB of disk, about 16 MB of memory.


How to play
-----------

Double-click Tennix in its drawer, or from a Shell:

    cd Tennix
    Tennix [-f] [-b]

-f opens a 640x480 screen instead of a window on the Workbench; -b is
upstream's benchmark/attract mode (two computer players, no menu).

The menu is played with the mouse: Play, click a stadium on the world map,
click the forward arrow, set up the two players (click a player's button
to cycle: the keyboard presets, Mouse, joysticks, "Carl Van Court" = the
computer) and click the arrow again.

Keys in a match (upstream's presets):

- Keyboard (arrows), player 1's default: Up/Down move, Space hit (hold it
  to fill the power bar), Left Ctrl topspin, Left Alt smash.
- Keyboard (WS/DEF), player 1 only: W/S, D hit, E topspin, F smash.
- Keyboard (OL/KIJ), player 2 only: O/L, K hit, I topspin, J smash.
- Mouse: the player follows the pointer up and down; left button hit,
  middle topspin, right smash.
- Joystick: stick up/down, buttons 1, 2, 3.
- Esc or Q: back to the menu, match paused (Resume). Esc or Q in the menu
  quits at once (the paused match is kept in PROGDIR:tennix-match-1.3.4);
  the Quit button and the close gadget roll the credits first (about
  14 s; a key or a click skips them).


Building
--------

GFOpenAmigaGCC 16.2.0-open4 or later, the OpenGPU developer kit, and
python3 on the build machine (amiga/tools/tnxpack.py packs data/ into
tennix.tnx):

    make -f amiga/Makefile.amigaos -j2 dist \
         STOVE=/path/to/openamigagcc-16.2.0-open4 \
         NDK=/path/to/OpenNDK/Include_H SDL2KIT=/path/to/opengpu-sdk

The defaults are CradleLab's paths. Output: build/amigaos/Tennix (stripped;
Tennix.debug beside it keeps the symbols) and build/amigaos/tennix.tnx;
'dist' makes dist/Tennix/ (Tennix, tennix.tnx, ReadMe, README-tennix,
COPYING.txt, COPYING-data.txt, COPYING-SDL2_gfx.txt, with GlowIcons) and
dist/Tennix.info. The icons are made from amiga/icons/icons.json (see
"about" there). fpcr_check.py (openamigartg) finds no FPCR store in the
objects.


What was changed
----------------

- amiga/Makefile.amigaos, amiga/config.h: upstream's object list without
  network.o and tennixpy.o; configure is not run (config.h by hand: SDL2
  image, ttf, mixer and gfx; no SDL2_net, Python or voice files). Links the
  kit's libSDL2_ttf.a (FreeType inside), libSDL2_image.a, libSDL2_mixer.a
  and libSDL2.a.
- amiga/deps/SDL2_gfx-1.0.4: SDL2_gfx's rotozoomer (zlib licence,
  unchanged; see SOURCE there). The kit has no SDL2_gfx.
- src/archive.h, src/archive.cc, amiga/tools/tnxpack.py: tennix.tnx is
  upstream's in-memory structs written raw. m68k GCC aligns uint32_t on 2
  bytes, so an item is 96 bytes here and 100 on a PC; a PC-built archive
  would not read. tnxpack.py (instead of upstream's tnxar) writes the same
  format in the Amiga's layout, and static_asserts check the sizes. The
  offsets and lengths are big-endian on every machine (ntohl is
  SDL_SwapBE32 here: libnix has no arpa/inet.h). The PNGs without alpha
  (ctt_clay, ctt_grass, ctt_hard: palette pictures with odd row lengths;
  night) are stored as RGBA with the same pixels, for SDL2_image.module
  0.6's heap trap. A check read the archive back as the Amiga does: all
  48 files are there, the four converted pictures have the same pixels,
  everything else is byte for byte upstream's.
- src/tennix.h: the data and the paused match are read from PROGDIR:.
- amiga/amiga_tennix.cc:
  - Picture: upstream draws into the window's surface. OpenGPU's SDL 2
    reports a 24-bit window, so the game draws into its own 32-bit XRGB
    640x480 surface, and updatescr() hands it to one streaming RGB888
    texture on the window's renderer each frame (src/graphics.cc).
  - Keys and mouse: upstream reads SDL_GetKeyboardState() and
    SDL_GetMouseState(). Here an event watch keeps the keys and the mouse
    from the events, and a press stays latched until a game tick has seen
    it, so a short tap of Space is not lost (src/tennix.h maps the two
    calls onto it). The match drains the event queue each tick (upstream
    only pumped it); closing the window during a match quits.
  - Pointer (amiga/amiga_pointer.c): in the lab the module's mouse
    events were about 16 px right and 32 px down of the real pointer (the
    window borders). The game reads the pointer from its Intuition window
    instead (MouseX/MouseY less BorderLeft/BorderTop, scaled from the
    inner size to 640x480); its window is the active one with the game's
    title, or the active one on SDL's own screen (-f). Elsewhere SDL's
    numbers stay.
  - Quit: the ~10 s of black window after Esc in the menu was upstream's
    credits roll (about 14 s). Esc/Q now leave at once; stderr prints
    "Quit: credits .. ms" and "Quit: freeing .. ms".
  - An Intuition requester when tennix.tnx is missing or the window cannot
    be opened (amiga/amiga_message.c), and the empty "SDL" screens
    SDL2.module 0.6 leaves after a full screen are closed at exit
    (amiga/amiga_screens.c, from the Seven Kingdoms port).
- src/tennix.cc: the menu takes every event of a frame (upstream took one)
  and quits on an Esc/Q press, not on a held key (Esc still down from the
  match quit the menu at once). V (full-screen toggle) is off: use -f.
  The controller cycling no longer reads input_devices[-1].
- src/graphics.cc: the loading bar shows before the fonts and pictures
  load; pictures are converted to ARGB8888 (SDL's fast alpha blit onto
  XRGB) rather than RGBA8888; a picture that fails to load becomes a 1x1
  clear one, never NULL; the off-screen surfaces are checked; rotozoom and
  rectangle_alpha() return if SDL gives no surface.
- src/input.cc: no "Network player" device; the mouse player now reads the
  pointer (upstream never set it, so he could not move); a joystick
  without a name gets "Joystick".
- src/sound.cc: 22050 Hz instead of 44100.
- src/game.cc: see the keys above.
- amiga/stack.c: 256 KB of stack and a $VER string. amiga/pthread_stubs.c:
  what libstdc++ asks of POSIX threads (from the Seven Kingdoms port).
  amiga/amiga_prelude.h: <pthread.h> first, so libnix's cancellation
  macros don't break <istream>.

Big-endian: tennix.tnx (above). The paused match is the GameState struct
written raw, read back by the same program: no change. Pictures are PNG
through SDL2_image, sounds Ogg Vorbis through SDL2_mixer, fonts TrueType.


Cut
---

- Network play (SDL2_net; -m/-s options and the "Network player").
- The Python bots (tennixpy.cc, data/defaultbot.py; no Python on the
  Amiga). The game's own computer player, Carl Van Court, stays.
- The V full-screen toggle (start with -f).
- Upstream's three non-free stadiums are not in its release tarball and
  are not here either (NONFREE_LOCATIONS stays off).


Licences
--------

- Tennix: GPL-2.0 or later (COPYING), Thomas Perl.
- Data (data/, packed into tennix.tnx): the game's own graphics by Thomas
  Perl (GPL-2.0 or later); the four stadium photos and the world map are
  public domain (data/COPYING lists their Wikimedia Commons sources and
  authors); the sounds are public domain (MelodyLoops.com); the fonts
  itwasntme.ttf (Dustin Norlander, 2002) and dustismo.ttf are GPL. This is
  as Debian's copyright file for tennix 1.3.4 records it (Debian ships
  this data in main).
- SDL2_gfx's rotozoomer: zlib licence (COPYING-SDL2_gfx.txt).
- SDL 2 and its satellites: OpenGPU's kit (its Licences/).


Known limits / to check in the lab
----------------------------------

- Re-test: the game's cursor on the real pointer, windowed and with -f;
  Esc in the menu quits at once; the Quit button shows the credits.
- Speed: the whole 640x480 picture goes to the texture every frame; the
  game's physics are tick-based, so a slow frame rate only makes it jerky.
- Sound: Ogg decoding through SDL2_mixer.module at load time; the audience
  and rain loops.
- -f (full screen) and quitting from it: no "SDL" screens must be left.
- Mouse as a controller, and the menu's mouse coordinates in full screen.
