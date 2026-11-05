
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

#include <string.h>

#include <cstdlib>

#include "tennix.h"
#include "graphics.h"
#include "game.h"
#include "input.h"
#include "util.h"
#include "archive.h"


static InputDevice devices[MAX_INPUT_DEVICES];
static int devices_count;

#ifdef HAVE_PYTHON
#include "tennixpy.h"
#endif

void init_input(TennixArchive& tnxar)
{
    int n, x;

    SDL_JoystickEventState(SDL_ENABLE);

    /* keyboard presets */
    devices[devices_count].type = INPUT_TYPE_KEYBOARD;
    devices[devices_count].up_key = SDL_SCANCODE_W;
    devices[devices_count].down_key = SDL_SCANCODE_S;
    devices[devices_count].input_keys[INPUT_KEY_HIT]= SDL_SCANCODE_D;
    devices[devices_count].input_keys[INPUT_KEY_TOPSPIN] = SDL_SCANCODE_E;
    devices[devices_count].input_keys[INPUT_KEY_SMASH] = SDL_SCANCODE_F;
    devices[devices_count].icon = GR_INPUT_KEYBOARD_WS;
    devices[devices_count].exclusive_to_player = 1;
    strcpy(devices[devices_count].name, "Keyboard (WS/DEF)");
    devices_count++;

    /* keyboard presets */
    devices[devices_count].type = INPUT_TYPE_KEYBOARD;
    devices[devices_count].up_key = SDL_SCANCODE_O;
    devices[devices_count].down_key = SDL_SCANCODE_L;
    devices[devices_count].input_keys[INPUT_KEY_HIT]= SDL_SCANCODE_K;
    devices[devices_count].input_keys[INPUT_KEY_TOPSPIN] = SDL_SCANCODE_I;
    devices[devices_count].input_keys[INPUT_KEY_SMASH] = SDL_SCANCODE_J;
    devices[devices_count].icon = GR_INPUT_KEYBOARD_OL;
    devices[devices_count].exclusive_to_player = 2;
    strcpy(devices[devices_count].name, "Keyboard (OL/KIJ)");
    devices_count++;

    /* keyboard presets */
    devices[devices_count].type = INPUT_TYPE_KEYBOARD;
    devices[devices_count].up_key = SDL_SCANCODE_UP;
    devices[devices_count].down_key = SDL_SCANCODE_DOWN;
    devices[devices_count].input_keys[INPUT_KEY_HIT]= SDL_SCANCODE_SPACE;
    devices[devices_count].input_keys[INPUT_KEY_TOPSPIN] = SDL_SCANCODE_LCTRL;
    devices[devices_count].input_keys[INPUT_KEY_SMASH] = SDL_SCANCODE_LALT;
    devices[devices_count].icon = GR_INPUT_KEYBOARD_ARROWS;
    strcpy(devices[devices_count].name, "Keyboard (arrows)");
    devices_count++;

    /* mouse */
    devices[devices_count].type = INPUT_TYPE_MOUSE;
    devices[devices_count].input_keys[INPUT_KEY_HIT]= SDL_BUTTON(1);
    devices[devices_count].input_keys[INPUT_KEY_TOPSPIN] = SDL_BUTTON(2);
    devices[devices_count].input_keys[INPUT_KEY_SMASH] = SDL_BUTTON(3);
    devices[devices_count].icon = GR_INPUT_MOUSE;
    strcpy(devices[devices_count].name, "Mouse");
    devices_count++;

    /* network peer */
    devices[devices_count].type = INPUT_TYPE_NETWORK;
    devices[devices_count].icon = GR_INPUT_AI; /* FIXME - network icon! */
    strcpy(devices[devices_count].name, "Network player");
    devices_count++;

    /* joysticks */
    n = SDL_NumJoysticks();
    for (x=0; x<n && devices_count<MAX_INPUT_DEVICES; x++) {
        strcpy(devices[devices_count].name, SDL_JoystickNameForIndex(x));
        devices[devices_count].type = INPUT_TYPE_JOYSTICK;
        devices[devices_count].joystick = SDL_JoystickOpen(x);
        devices[devices_count].x_axis = 0;
        devices[devices_count].y_axis = 0;
        devices[devices_count].input_keys[INPUT_KEY_HIT]= 0;
        devices[devices_count].input_keys[INPUT_KEY_TOPSPIN] = 1;
        devices[devices_count].input_keys[INPUT_KEY_SMASH] = 2;
        devices[devices_count].icon = GR_INPUT_GAMEPAD;
        devices_count++;
    }

#ifdef HAVE_PYTHON
    /* This will init Python and load all available bots */
    tennixpy_init(tnxar);
#endif

}

void uninit_input()
{
    int i;
    SDL_JoystickEventState(SDL_IGNORE);

    for (i=0; i<devices_count; i++) {
        if (devices[i].type == INPUT_TYPE_JOYSTICK) {
            SDL_JoystickClose(devices[i].joystick);
#ifdef HAVE_PYTHON
        } else if (devices[i].type == INPUT_TYPE_AI_PYTHON) {
            tennixpy_unregister_bot(&(devices[i]));
#endif
        }
    }

#ifdef HAVE_PYTHON
    tennixpy_uninit();
#endif

    devices_count = 0;
}

Uint8
input_pack_axis(float v)
{
    static const float min = -1.2;
    static const float max = 1.2;
    tnx_assert(v >= min && v < max);
    return (Uint8)((1U<<7) * (v-min) / (max-min));
}

float
input_unpack_axis(Uint8 v)
{
    static const float min = -1.2;
    static const float max = 1.2;
    tnx_assert(v < (1U<<7));
    return v * (max-min) / (1U<<7) + min;
}


InputDevice* find_input_devices(unsigned int* count)
{
    *count = devices_count;
    return devices;
}

void input_device_join_game(InputDevice* device, void* gamestate, int player_id)
{
    if (device == NULL) {
        /* player is a c-style bot with no device attached */
        return;
    }
    fprintf(stderr, "Input Device %s joins the game\n", device->name);
#ifdef HAVE_PYTHON
    if (device->type == INPUT_TYPE_AI_PYTHON) {
        tennixpy_create_bot(device, (GameState*)gamestate, player_id);
    }
#endif
}

void input_device_part_game(InputDevice* device)
{
    if (device == NULL) {
        /* player is a c-style bot with no device attached */
        return;
    }
#ifdef HAVE_PYTHON
    if (device->type == INPUT_TYPE_AI_PYTHON) {
        tennixpy_destroy_bot(device);
    }
#endif
    fprintf(stderr, "Input Device %s leaves the game\n", device->name);
}

const char* input_device_get_name(InputDevice* d)
{
    return d->name;
}

float input_device_get_axis(InputDevice* d, unsigned const char axis) {
    const Uint8 *keystate;
    //Uint8 mb;
    Uint8 net_value;
    float result = 0.0;

    SDL_PumpEvents();

    if (d->type == INPUT_TYPE_KEYBOARD) {
        keystate = SDL_GetKeyboardState(NULL);
        if (axis == INPUT_AXIS_X) {
            result =  1.0*keystate[d->right_key] + -1.0*keystate[d->left_key];
        } else {
            result = 1.0*keystate[d->down_key] + -1.0*keystate[d->up_key];
        }
    } else if (d->type == INPUT_TYPE_JOYSTICK) {
        if (axis == INPUT_AXIS_X) {
            result = JOYSTICK_PERCENTIZE(SDL_JoystickGetAxis(d->joystick, d->x_axis*2));
        } else {
            result = JOYSTICK_PERCENTIZE(SDL_JoystickGetAxis(d->joystick, 1+d->y_axis*2));
        }
    } else if (d->type == INPUT_TYPE_MOUSE) {
        //mb = SDL_GetMouseState(&d->mx, &d->my);
        if (axis == INPUT_AXIS_X) {
            /* Not x-movement yet (PLAYER_MOVE_X is not defined!) */
            /*if (std::abs(d->mx - d->player_x) > PLAYER_MOVE_X) {
                if (d->mx > d->player_x)  {
                    return 1.0;
                } else if (d->mx < d->player_x) {
                    return -1.0;
                }
            }*/
        } else {
            if (std::abs(d->my - d->player_y) > PLAYER_MOVE_Y) {
                if (d->my > d->player_y)  {
                    result = 1.0;
                } else if (d->my < d->player_y) {
                    result = -1.0;
                }
            }
        }
#ifdef HAVE_PYTHON
    } else if (d->type == INPUT_TYPE_AI_PYTHON) {
        result = tennixpy_bot_get_axis(d, axis);
#endif
    } else if (d->type == INPUT_TYPE_NETWORK) {
        if (axis == INPUT_AXIS_X) {
            result = input_unpack_axis(d->net.x);
        } else if (axis == INPUT_AXIS_Y) {
            result = input_unpack_axis(d->net.y);
        }
    } else {
        tnx_assert(0/*unimplemented*/);
    }

    net_value = input_pack_axis(result);
    if (axis == INPUT_AXIS_X) {
        d->net.x = net_value;
    } else if (axis == INPUT_AXIS_Y) {
        d->net.y = net_value;
    }

    return result;
}

char input_device_get_key(InputDevice* d, unsigned const char key) {
    Uint8 mb;
    char result = 0;
    SDL_PumpEvents();

    if (d->type == INPUT_TYPE_KEYBOARD) {
        result = SDL_GetKeyboardState(NULL)[d->input_keys[key]];
    } else if (d->type == INPUT_TYPE_JOYSTICK) {
        result = SDL_JoystickGetButton(d->joystick, d->input_keys[key]);
    } else if (d->type == INPUT_TYPE_MOUSE) {
        mb = SDL_GetMouseState(NULL, NULL);
        result = (mb & d->input_keys[key]) != 0;
#ifdef HAVE_PYTHON
    } else if (d->type == INPUT_TYPE_AI_PYTHON) {
        result = tennixpy_bot_get_key(d, key);
#endif
    } else if (d->type == INPUT_TYPE_NETWORK) {
        result = (d->net.keys & (1<<key));
    } else {
        tnx_assert(0/*unimplemented*/);
    }

    /* Update the input device's NetworkInputData struct */
    if(result) {
        d->net.keys |= (1<<key);
    } else {
        d->net.keys &= ~(1<<key);
    }

    return result;
}


InputDevice *
input_add_device()
{
    tnx_assert(devices_count < MAX_INPUT_DEVICES);
    return &(devices[devices_count++]);
}

