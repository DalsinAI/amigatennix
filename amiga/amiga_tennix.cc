/* Tennix on AmigaOS 3.x with OpenGPU's SDL 2: the picture, the keys and
 * the mouse, and what is left behind at the end.
 *
 * Picture: upstream draws into the window's surface (SDL_GetWindowSurface)
 * and updates rectangles of it. OpenGPU's SDL2.module reports a 24-bit
 * window, so here the game draws into a 32-bit XRGB surface of its own
 * (SDL_PIXELFORMAT_RGB888, 640x480), and tnx_amiga_present() copies it
 * into one streaming texture of the same format on the window's renderer.
 *
 * Keys and mouse: upstream reads SDL_GetKeyboardState() and
 * SDL_GetMouseState(). Here an event watch keeps both from the events
 * themselves, with a press latched until the game has looked at it once
 * (tnx_amiga_input_tick_done()), so a tap shorter than a game tick is
 * not lost.
 *
 * The AmiCradle Team. GPL 2 or later, like Tennix. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tennix.h"

extern "C" {
void amiga_message(const char* title, const char* text);
void amiga_screens_snapshot(void);
void amiga_screens_close_leftovers(void);
}

static SDL_Renderer* renderer;
static SDL_Texture* texture;
static SDL_Surface* frame;

static Uint8 key_down[SDL_NUM_SCANCODES];
static Uint8 key_latch[SDL_NUM_SCANCODES];
static Uint8 key_view[SDL_NUM_SCANCODES];
static int mouse_x = WIDTH / 2, mouse_y = HEIGHT / 2;
static Uint32 mouse_down, mouse_latch;

void tnx_amiga_message(const char* title, const char* text)
{
    fprintf(stderr, "%s: %s\n", title, text);
    amiga_message(title, text);
}

static void amiga_quit(void)
{
    /* SDL first (exit() from an error skips upstream's SDL_Quit()), then
     * the empty "SDL" screens SDL2.module 0.6 leaves after a full screen. */
    SDL_Quit();
    amiga_screens_close_leftovers();
}

void tnx_amiga_init(void)
{
    amiga_screens_snapshot();
    atexit(amiga_quit);
}

static void video_fail(const char* what)
{
    char text[256];
    snprintf(text, sizeof(text), "%s:\n%s", what, SDL_GetError());
    tnx_amiga_message("Tennix", text);
    exit(EXIT_FAILURE);
}

SDL_Surface* tnx_amiga_video_init(SDL_Window* window)
{
    if (window == NULL) {
        video_fail("Cannot open the 640x480 window");
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer == NULL) {
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (renderer == NULL) {
        video_fail("Cannot draw in the window");
    }
    SDL_RenderSetLogicalSize(renderer, WIDTH, HEIGHT);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888,
            SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
    if (texture == NULL) {
        video_fail("Cannot make the screen texture");
    }

    frame = SDL_CreateRGBSurfaceWithFormat(0, WIDTH, HEIGHT, 32, SDL_PIXELFORMAT_RGB888);
    if (frame == NULL) {
        video_fail("Cannot make the 640x480 picture");
    }
    SDL_FillRect(frame, NULL, 0);
    tnx_amiga_present();

    return frame;
}

void tnx_amiga_present(void)
{
    SDL_UpdateTexture(texture, NULL, frame->pixels, frame->pitch);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

static int SDLCALL input_watch(void* userdata, SDL_Event* e)
{
    int sc;
    (void)userdata;

    switch (e->type) {
        case SDL_KEYDOWN:
            sc = e->key.keysym.scancode;
            if (sc > 0 && sc < SDL_NUM_SCANCODES) {
                key_down[sc] = 1;
                key_latch[sc] = 1;
            }
            break;
        case SDL_KEYUP:
            sc = e->key.keysym.scancode;
            if (sc > 0 && sc < SDL_NUM_SCANCODES) {
                key_down[sc] = 0;
            }
            break;
        case SDL_MOUSEMOTION:
            mouse_x = e->motion.x;
            mouse_y = e->motion.y;
            break;
        case SDL_MOUSEBUTTONDOWN:
            mouse_x = e->button.x;
            mouse_y = e->button.y;
            mouse_down |= SDL_BUTTON(e->button.button);
            mouse_latch |= SDL_BUTTON(e->button.button);
            break;
        case SDL_MOUSEBUTTONUP:
            mouse_x = e->button.x;
            mouse_y = e->button.y;
            mouse_down &= ~SDL_BUTTON(e->button.button);
            break;
        case SDL_WINDOWEVENT:
            if (e->window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                /* The key-ups go to another window: nothing stays held. */
                memset(key_down, 0, sizeof(key_down));
                mouse_down = 0;
            }
            break;
    }
    return 0;
}

void tnx_amiga_input_init(void)
{
    /* After SDL_CreateRenderer(): the renderer's own watch has put the
     * mouse coordinates into its logical 640x480 by then. */
    SDL_AddEventWatch(input_watch, NULL);
}

const Uint8* tnx_amiga_keyboard_state(int* numkeys)
{
    int i;
    for (i = 0; i < SDL_NUM_SCANCODES; i++) {
        key_view[i] = key_down[i] | key_latch[i];
    }
    if (numkeys != NULL) {
        *numkeys = SDL_NUM_SCANCODES;
    }
    return key_view;
}

Uint32 tnx_amiga_mouse_state(int* x, int* y)
{
    if (x != NULL) {
        *x = mouse_x;
    }
    if (y != NULL) {
        *y = mouse_y;
    }
    return mouse_down | mouse_latch;
}

void tnx_amiga_input_tick_done(void)
{
    memset(key_latch, 0, sizeof(key_latch));
    mouse_latch = 0;
}

static bool quit_seen = false;

bool tnx_amiga_quit_seen(void)
{
    return quit_seen;
}

bool tnx_amiga_drain_events(void)
{
    SDL_Event e;
    bool quit = false;

    /* The match only pumps; without this the queue grows all game long.
     * The watch has seen every event already. */
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            quit = quit_seen = true;
        }
    }
    return quit;
}
