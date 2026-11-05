
/**
 *
 * Tennix! SDL Port
 * Copyright (C) 2003, 2007, 2008, 2009 Thomas Perl <thp@thpinfo.com>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, 
 * MA  02110-1301, USA.
 *
 **/

#ifndef __TENNIX_H
#define __TENNIX_H

#include "config.h"

#define tnx_assert(x) do { \
    if (!(x)) { \
        fprintf(stderr, "Assertion fail: %s\n", #x); \
        exit(1); \
    } \
} while(0)

#include <sys/param.h>

#include <SDL.h>

#ifdef __amigaos__
/* amiga/amiga_tennix.cc: the window's picture, keys and mouse through
 * events, the requester and the leftover SDL screens. */
void tnx_amiga_message(const char* title, const char* text);
void tnx_amiga_init(void);
SDL_Surface* tnx_amiga_video_init(SDL_Window* window);
void tnx_amiga_present(void);
void tnx_amiga_input_init(void);
const Uint8* tnx_amiga_keyboard_state(int* numkeys);
Uint32 tnx_amiga_mouse_state(int* x, int* y);
void tnx_amiga_input_tick_done(void);
bool tnx_amiga_drain_events(void);
bool tnx_amiga_quit_seen(void);
/* OpenGPU's SDL2.module: keys and the mouse are read from the events
 * (an event watch keeps their state), not from SDL's own state tables. */
#define SDL_GetKeyboardState(n) tnx_amiga_keyboard_state(n)
#define SDL_GetMouseState(x, y) tnx_amiga_mouse_state(x, y)
#endif

#ifdef HAVE_SDL_NET
#include <SDL_net.h>
#endif

typedef struct {
    unsigned char x;
    unsigned char y;
    unsigned char keys; /* bitfield (0 = key pressed, 1 = not pressed) */
} NetworkInputData;

#ifdef HAVE_SDL_NET
typedef struct {
    UDPsocket send_socket;
    UDPsocket recv_input_socket;
    UDPsocket recv_state_socket;

    UDPpacket* input_packet;
    UDPpacket* state_packet;

    IPaddress peer;

    bool input_available;
    bool state_available;

    Uint16 base_port_local;
    Uint16 base_port_remote;

    NetworkInputData input_data;

    bool master;
} TennixNet;
#else
typedef void TennixNet;
#endif /* HAVE_SDL_NET */

#ifdef __amigaos__
/* The data and the paused match live in the program's drawer. */
#define ARCHIVE_FILE "PROGDIR:tennix.tnx"
#define ARCHIVE_FILE_INSTALLED "tennix.tnx"
#define GAMESTATE_FILE "PROGDIR:tennix-match-" VERSION
#else
#define ARCHIVE_FILE "tennix.tnx"
#define ARCHIVE_FILE_INSTALLED "/" PREFIX "/share/tennix/" ARCHIVE_FILE

#define GAMESTATE_FILE ".tennix-current_match-" VERSION
#endif

#define COPYRIGHT "Copyright 2003, 2007-2011, 2013, 2017, 2019-2020, 2023 Thomas Perl"
#define URL "http://icculus.org/tennix/"

#define WIDTH 640
#define HEIGHT 480

#define MENU_BUTTON_HEIGHT 62
#define MENU_BUTTON_WIDTH 185

#define MENU_START_YPOS 150
#define MENU_RESUME_YPOS 230
#define MENU_QUIT_YPOS 350

#define M_POS_XPOS(x)    (x>WIDTH-MENU_BUTTON_WIDTH)
#define M_POS_START(y)   (y>MENU_START_YPOS && y<MENU_START_YPOS+MENU_BUTTON_HEIGHT)
#define M_POS_RESUME(y) (y>MENU_RESUME_YPOS && y<MENU_RESUME_YPOS+MENU_BUTTON_HEIGHT)
#define M_POS_QUIT(y)    (y>MENU_QUIT_YPOS && y<MENU_QUIT_YPOS+MENU_BUTTON_HEIGHT)

#define MENU_OPTIONS_BORDER 20
#define MENU_OPTIONS_BUTTON_HEIGHT 25
#define MENU_OPTIONS_BUTTON_WIDTH 150

/* Menu items */
enum {
    MENU_NONE = 0,
    MENU_START = 1,
    MENU_RESUME = 2,
    MENU_COURT_CHANGE = 4,
    MENU_PLAYER1 = 8,
    MENU_PLAYER2 = 16,
    MENU_QUIT = 32
};

#define M_POS_DECODE(x,y) (M_POS_XPOS(x)?((M_POS_START(y))?(MENU_START):((M_POS_QUIT(y))?(MENU_QUIT):((M_POS_RESUME(y))?(MENU_RESUME):(MENU_NONE)))):(MENU_NONE))

/* Menu states */
enum {
    MENU_STATE_STARTED = 0,
    MENU_STATE_SLIDE_TO_MAINMENU,
    MENU_STATE_SLIDE_TO_MAINMENU_IN_PROGRESS,
    MENU_STATE_MAINMENU,
    MENU_STATE_SLIDE_TO_LOCATION,
    MENU_STATE_SLIDE_TO_LOCATION_IN_PROGRESS,
    MENU_STATE_FADE_TO_LOCATION,
    MENU_STATE_LOCATION,
    MENU_STATE_FADE_TO_OPTIONS,
    MENU_STATE_OPTIONS,
    MENU_STATE_SLIDE_TO_GAME,
    MENU_STATE_SLIDE_TO_GAME_IN_PROGRESS,
    MENU_STATE_GAME,
    MENU_STATE_SLIDE_TO_RESUME,
    MENU_STATE_SLIDE_TO_RESUME_IN_PROGRESS,
    MENU_STATE_RESUME,
    MENU_STATE_SLIDE_TO_QUIT,
    MENU_STATE_SLIDE_TO_QUIT_IN_PROGRESS,
    MENU_STATE_QUIT
};

#define DEFAULT_FPS 33


extern SDL_Surface *screen;
extern SDL_Window *window;

typedef struct {
    const char *text;
    int x;
    int y;
    int w;
    int h;
    Uint8 r;
    Uint8 g;
    Uint8 b;
    unsigned int image_id;
} MenuButton;

void menu_button_init(MenuButton*);

#define CONTROLLER_SETUP_BORDER 50
#define CONTROLLER_SETUP_SIZE 200

#define M_POS_BUTTON(button,mx,my) (mx>=(button).x && mx<=(button).x+(button).w && my>=(button).y && my<=(button).y+(button).h)

#define SQUARE_DISTANCE(xdiff, ydiff) (((xdiff)*(xdiff)) + ((ydiff)*(ydiff)))

#endif
