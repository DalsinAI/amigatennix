

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

#ifndef __TENNIXPY_H
#define __TENNIXPY_H

#include "config.h"

#ifdef HAVE_PYTHON

#include "archive.h"
#include "input.h"

void tennixpy_create_bot(InputDevice *device, GameState *s, int player_id);
void tennixpy_destroy_bot(InputDevice *device);

void tennixpy_unregister_bot(InputDevice *device);

float tennixpy_bot_get_axis(InputDevice *device, int axis);
char tennixpy_bot_get_key(InputDevice *device, int key);

void tennixpy_init(TennixArchive& tnxar);
void tennixpy_uninit();

#endif /* HAVE_PYTHON */

#endif /* __TENNIXPY_H */
