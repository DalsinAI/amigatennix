

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

#include "config.h"

#ifdef HAVE_PYTHON

#include <Python.h>

#include "game.h"
#include "archive.h"
#include "tennixpy.h"

static void tennixpy_get_bot_name(InputDevice *device, char *dest, int maxlen);
static PyObject* tennixpy_register_bot(PyObject* self, PyObject* bot_class);
static PyObject* tennixpy_get_ball_pos(PyObject* self, PyObject* gamestate);
static PyObject* tennixpy_get_power(PyObject* self, PyObject* args);
static PyObject* tennixpy_get_position(PyObject* self, PyObject* args);
static PyObject* tennixpy_create_module(void);

struct InputDevicePython {
    PyObject* py_bot_class;
    PyObject* py_bot;
};


/* This saves our current Python thread state */
static PyThreadState *_py_save;

static PyMethodDef TennixMethods[] = {
    {"register_bot", tennixpy_register_bot, METH_O, "Register a new bot"},
    {"get_ball_pos", tennixpy_get_ball_pos, METH_O, "Gets the ball position"},
    {"get_position", tennixpy_get_position, METH_VARARGS, "Get position of player"},
    {"get_power", tennixpy_get_power, METH_VARARGS, "Get power of player"},
    {NULL, NULL, 0, NULL} /* Sentinel */
};

static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "tennix",
        NULL,
        -1,
        TennixMethods,
        NULL,
        NULL,
        NULL,
        NULL
};

PyObject* tennixpy_create_module(void)
{
    PyObject *module = PyModule_Create(&moduledef);
    PyModule_AddIntConstant(module, "INPUT_AXIS_X", INPUT_AXIS_X);
    PyModule_AddIntConstant(module, "INPUT_AXIS_Y", INPUT_AXIS_Y);
    PyModule_AddIntConstant(module, "INPUT_KEY_HIT", INPUT_KEY_HIT);
    PyModule_AddIntConstant(module, "INPUT_KEY_TOPSPIN", INPUT_KEY_TOPSPIN);
    PyModule_AddIntConstant(module, "INPUT_KEY_SMASH", INPUT_KEY_SMASH);
    PyDict_SetItemString(PyImport_GetModuleDict(), moduledef.m_name, module);
    return module;
}

void
tennixpy_create_bot(InputDevice *device, GameState *s, int player_id)
{
    InputDevicePython *pydevice = (InputDevicePython*)device->user_data;
    PyObject *bot_class = pydevice->py_bot_class;

    PyObject* o = NULL;
    PyObject* args;
    PyObject* gamestate;

    PyEval_RestoreThread(_py_save);
    if (PyType_Check(bot_class)) {
        gamestate = PyCapsule_New(s, "GameState", NULL);
        assert(gamestate != NULL);
        args = Py_BuildValue("(Oi)", gamestate, player_id);
        assert(args != NULL);
        Py_DECREF(gamestate);
        o = PyObject_CallObject(bot_class, args);
        Py_DECREF(args);
        if (o == NULL) {
            fprintf(stderr, "cannot create instance\n");
            PyErr_Print();
        }
    } else {
        fprintf(stderr, "Typecheck failed\n");
    }
    _py_save = PyEval_SaveThread();

    pydevice->py_bot = o;
}

void tennixpy_destroy_bot(InputDevice *device)
{
    InputDevicePython *pydevice = (InputDevicePython*)device->user_data;
    PyObject *bot = pydevice->py_bot;

    PyObject* finish_func;
    PyEval_RestoreThread(_py_save);
    finish_func = PyObject_GetAttrString(bot, "finish");
    assert(finish_func != NULL);
    Py_DECREF(PyObject_CallObject(finish_func, NULL));
    Py_DECREF(finish_func);
    Py_DECREF(bot);
    _py_save = PyEval_SaveThread();
}

float tennixpy_bot_get_axis(InputDevice *device, int axis)
{
    InputDevicePython *pydevice = (InputDevicePython*)device->user_data;
    PyObject *bot = pydevice->py_bot;

    PyObject* args;
    PyObject* get_axis_func;
    PyObject* r;
    float result = 0.0;

    PyEval_RestoreThread(_py_save);

    get_axis_func = PyObject_GetAttrString(bot, "get_axis");
    assert(get_axis_func != NULL);

    args = Py_BuildValue("(i)", axis);
    r = PyObject_CallObject(get_axis_func, args);
    Py_DECREF(get_axis_func);
    Py_DECREF(args);

    if (r == NULL) {
        PyErr_Print();
    } else {
        if (PyFloat_Check(r)) {
            result = (float)PyFloat_AsDouble(r);
        } else {
            fprintf(stderr, "Unexpected value from python\n");
        }
        Py_DECREF(r);
    }

    _py_save = PyEval_SaveThread();
    return result;
}

char tennixpy_bot_get_key(InputDevice *device, int key)
{
    InputDevicePython *pydevice = (InputDevicePython*)device->user_data;
    PyObject *bot = pydevice->py_bot;

    PyObject* args;
    PyObject* get_key_func;
    PyObject* r;
    char result = 0;

    PyEval_RestoreThread(_py_save);

    get_key_func = PyObject_GetAttrString(bot, "get_key");
    assert(get_key_func != NULL);

    args = Py_BuildValue("(i)", key);
    r = PyObject_CallObject(get_key_func, args);
    Py_DECREF(get_key_func);
    Py_DECREF(args);

    if (r == NULL) {
        PyErr_Print();
    } else {
        if (r == Py_True) {
            result = 1;
        } else if (r == Py_False) {
            result = 0;
        } else {
            fprintf(stderr, "Unexpected value from python\n");
        }
        Py_DECREF(r);
    }

    _py_save = PyEval_SaveThread();
    return result;
}

static void
input_add_python_bot(PyObject* bot_class)
{
    InputDevice *device = input_add_device();

    if (device == NULL) {
        fprintf(stderr, "Warning: Cannot add any more Python bots.\n");
        // We carry a ref of bot_class, so give it up here
        Py_DECREF(bot_class);
        return;
    }

    device->type = INPUT_TYPE_AI_PYTHON;
    device->icon = GR_INPUT_AI;

    InputDevicePython *pydevice = new InputDevicePython;
    pydevice->py_bot_class = bot_class;
    pydevice->py_bot = NULL;
    device->user_data = (void*)pydevice;
    tennixpy_get_bot_name(device, device->name, INPUT_DEVICE_NAME_MAX);
}

PyObject* tennixpy_register_bot(PyObject* self, PyObject* bot_class)
{
    if (PyType_Check(bot_class)) {
        /* FIXME: check if all required attrs are here */
        input_add_python_bot(bot_class);
        Py_INCREF(bot_class);
    } else {
        return PyErr_Format(PyExc_TypeError, "This function needs a bot class to work");
    }

    Py_RETURN_NONE;
}

void tennixpy_get_bot_name(InputDevice *device, char* dest, int maxlen)
{
    InputDevicePython *pydevice = (InputDevicePython*)device->user_data;
    PyObject *bot_class = pydevice->py_bot_class;
    PyObject *name;

    name = PyObject_GetAttrString(bot_class, "name");

    if (PyUnicode_Check(name) && name != NULL) {
        strncpy(dest, PyUnicode_AsUTF8(name), maxlen-1);
    } else {
        strncpy(dest, "<unknown pybot>", maxlen-1);
    }
    dest[maxlen-1] = '\0';

    Py_XDECREF(name);
}

PyObject* tennixpy_get_ball_pos(PyObject* self, PyObject* gamestate)
{
    PyObject* r;
    GameState* s;

    if (PyCapsule_CheckExact(gamestate)) {
        s = (GameState *)PyCapsule_GetPointer(gamestate, "GameState");
        r = Py_BuildValue("(dd)", (double)(s->ball.x), (double)(s->ball.y - s->ball.z));
        return r;
    } else {
        return PyErr_Format(PyExc_TypeError, "This function needs a GameState to work on");
    }
}

PyObject* tennixpy_get_power(PyObject* self, PyObject* args)
{
    PyObject* gamestate;
    PyObject* player_id;

    GameState* s;
    int player;

    assert(PyTuple_Check(args));

    if (PyObject_Length(args) != 2) {
        return PyErr_Format(PyExc_TypeError, "This function takes exactly 2 parameters");
    }

    gamestate = PyTuple_GetItem(args, 0);
    player_id = PyTuple_GetItem(args, 1);

    if (PyLong_Check(player_id)) {
        player = (int)PyLong_AsLong(player_id);
    } else {
        return PyErr_Format(PyExc_TypeError, "Invalid player_id in second argument");
    }
    if (PyCapsule_CheckExact(gamestate)) {
        s = (GameState *)PyCapsule_GetPointer(gamestate, "GameState");
        return PyFloat_FromDouble((double)(PLAYER(s, player).power));
    } else {
        return PyErr_Format(PyExc_TypeError, "First argument is not a GameState");
    }
}

PyObject* tennixpy_get_position(PyObject* self, PyObject* args)
{
    PyObject* gamestate;
    PyObject* player_id;

    GameState* s;
    int player;

    assert(PyTuple_Check(args));

    if (PyObject_Length(args) != 2) {
        return PyErr_Format(PyExc_TypeError, "This function takes exactly 2 parameters");
    }

    gamestate = PyTuple_GetItem(args, 0);
    player_id = PyTuple_GetItem(args, 1);

    if (PyLong_Check(player_id)) {
        player = (int)PyLong_AsLong(player_id);
    } else {
        return PyErr_Format(PyExc_TypeError, "Invalid player_id in second argument");
    }
    if (PyCapsule_CheckExact(gamestate)) {
        s = (GameState *)PyCapsule_GetPointer(gamestate, "GameState");
        return Py_BuildValue("(dd)", (double)(PLAYER(s, player).x), (double)(PLAYER(s, player).y));
    } else {
        return PyErr_Format(PyExc_TypeError, "First argument is not a GameState");
    }
}

void tennixpy_init(TennixArchive& tnxar)
{
    PyObject* tennix_module;
    char* data;

    setenv("PYTHONPATH", ".", 1);
    Py_Initialize();
    tennix_module = tennixpy_create_module();

    tnxar.rewind();
    while (!tnxar.endOfFile()) {
        const char *filename = tnxar.getItemFilename();
        if (strcmp(".py", filename + strlen(filename) - 3) == 0) {
            data = tnxar.getItemBytes();
            if (PyRun_SimpleString(data) != 0) {
                PyErr_Print();
            }
            free(data);
        }
        tnxar.next();
    }

    Py_DECREF(tennix_module);
    _py_save = PyEval_SaveThread();
}

void tennixpy_unregister_bot(InputDevice *device)
{
    InputDevicePython *pydevice = (InputDevicePython*)device->user_data;
    PyEval_RestoreThread(_py_save);
    Py_XDECREF(pydevice->py_bot);
    _py_save = PyEval_SaveThread();
    delete pydevice;
}

void tennixpy_uninit()
{
    PyEval_RestoreThread(_py_save);
    /**
     * FIXME: We _should_ run Py_Finalize here, but it makes
     * Tennix crash, so we just tear it down without calling it.
     **/
    /*Py_Finalize();*/
}

#endif /* HAVE_PYTHON */
