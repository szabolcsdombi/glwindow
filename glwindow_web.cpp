#include <Python.h>
#include <structmember.h>

#define QOI_NO_STDIO
#define QOI_IMPLEMENTATION
#include <qoi.h>

#define QOA_NO_STDIO
#define QOA_IMPLEMENTATION
#include <qoa.h>

struct Window {
    PyObject_HEAD

    PyObject * size;
    PyObject * view;
    PyObject * app;
};

struct Audio {
    PyObject_HEAD
};

struct ModuleState {
    PyObject * helper;

    PyTypeObject * Window_type;
    PyTypeObject * Audio_type;

    Window * window;
    Audio * audio;
};

PyObject * meth_get_window(PyObject * self) {
    ModuleState * module_state = (ModuleState *)PyModule_GetState(self);
    return Py_XNewRef(module_state->window);
}

PyObject * meth_get_audio(PyObject * self) {
    ModuleState * module_state = (ModuleState *)PyModule_GetState(self);
    return Py_XNewRef(module_state->audio);
}

PyObject * meth_decode_qoi(PyObject * self, PyObject * arg) {
    if (!PyBytes_Check(arg)) {
        return NULL;
    }
    const char * ptr = PyBytes_AsString(arg);
    int size = (int)PyBytes_Size(arg);
    qoi_desc header = {};
    PyObject * data = PyBytes_FromStringAndSize(NULL, qoi_expected_size(ptr, size, 4));
    qoi_decode((unsigned char *)PyBytes_AsString(data), ptr, size, &header, 4);
    return Py_BuildValue("((ii)N)", header.width, header.height, data);
}

PyObject * meth_decode_qoa(PyObject * self, PyObject * arg) {
    if (!PyBytes_Check(arg)) {
        return NULL;
    }
    const unsigned char * ptr = (unsigned char *)PyBytes_AsString(arg);
    int size = (int)PyBytes_Size(arg);
    qoa_desc header = {};
    PyObject * data = PyBytes_FromStringAndSize(NULL, qoa_expected_size(ptr, size));
    qoa_decode((short *)PyBytes_AsString(data), ptr, size, &header);
    return Py_BuildValue("(iiN)", header.channels, header.samplerate, data);
}

PyObject * meth_run(PyObject * self, PyObject * args, PyObject * kwargs) {
    const char * keywords[] = {"app", NULL};

    PyObject * app;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O", (char **)keywords, &app)) {
        return NULL;
    }

    ModuleState * module_state = (ModuleState *)PyModule_GetState(self);

    module_state->window = PyObject_New(Window, module_state->Window_type);
    module_state->audio = PyObject_New(Audio, module_state->Audio_type);

    PyObject * init_result = PyObject_CallMethod(module_state->helper, "init_glwindow_web", "(OOs)", module_state->window, app, GLWINDOW_JS);
    if (!init_result) {
        return NULL;
    }

    Py_RETURN_NONE;
}

static void default_dealloc(PyObject * self) {
    PyObject_Free(self);
}

static PyMethodDef Window_methods[] = {
    {0},
};

static PyMemberDef Window_members[] = {
    {"size", T_OBJECT, offsetof(Window, size), READONLY},
    {"view", T_OBJECT, offsetof(Window, view), READONLY},

    {"_size", T_OBJECT, offsetof(Window, size), 0},
    {"_view", T_OBJECT, offsetof(Window, view), 0},
    {"_app", T_OBJECT, offsetof(Window, app), 0},
    {0},
};

static PyGetSetDef Window_getset[] = {
    // {"foo", (getter)Window_get_foo, (setter)Window_set_foo},
    {0},
};

static PyType_Slot Window_slots[] = {
    {Py_tp_methods, Window_methods},
    {Py_tp_members, Window_members},
    {Py_tp_getset, Window_getset},
    {Py_tp_dealloc, (void *)default_dealloc},
    {0},
};

static PyMethodDef Audio_methods[] = {
    // {"foo", (PyCFunction)Audio_meth_foo, METH_VARARGS | METH_KEYWORDS},
    {0},
};

static PyMemberDef Audio_members[] = {
    // {"foo", T_OBJECT, offsetof(Audio, foo), READONLY},
    {0},
};

static PyGetSetDef Audio_getset[] = {
    // {"foo", (getter)Audio_get_foo, (setter)Audio_set_foo},
    {0},
};

static PyType_Slot Audio_slots[] = {
    {Py_tp_methods, Audio_methods},
    {Py_tp_members, Audio_members},
    {Py_tp_getset, Audio_getset},
    {Py_tp_dealloc, (void *)default_dealloc},
    {0},
};


static PyType_Spec Window_spec = {"glwindow.Window", sizeof(Window), 0, Py_TPFLAGS_DEFAULT, Window_slots};
static PyType_Spec Audio_spec = {"glwindow.Audio", sizeof(Audio), 0, Py_TPFLAGS_DEFAULT, Audio_slots};

static int module_exec(PyObject * self) {
    ModuleState * module_state = (ModuleState *)PyModule_GetState(self);

    module_state->helper = PyImport_ImportModule("_glwindow");
    if (!module_state->helper) {
        return -1;
    }

    module_state->window = NULL;
    module_state->audio = NULL;

    module_state->Window_type = (PyTypeObject *)PyType_FromSpec(&Window_spec);
    module_state->Audio_type = (PyTypeObject *)PyType_FromSpec(&Audio_spec);

    PyModule_AddObject(self, "Window", Py_NewRef(module_state->Window_type));
    PyModule_AddObject(self, "Audio", Py_NewRef(module_state->Audio_type));

    PyModule_AddObject(self, "__version__", PyUnicode_FromString("0.1.0"));
    return 0;
}

static PyModuleDef_Slot module_slots[] = {
    {Py_mod_exec, (void *)module_exec},
    {0},
};

static PyMethodDef module_methods[] = {
    {"run", (PyCFunction)meth_run, METH_VARARGS | METH_KEYWORDS, NULL},
    {"get_window", (PyCFunction)meth_get_window, METH_NOARGS, NULL},
    {"get_audio", (PyCFunction)meth_get_audio, METH_NOARGS, NULL},
    {"decode_qoi", (PyCFunction)meth_decode_qoi, METH_O, NULL},
    {"decode_qoa", (PyCFunction)meth_decode_qoa, METH_O, NULL},
    {0},
};

static void module_free(PyObject * self) {
    ModuleState * state = (ModuleState *)PyModule_GetState(self);
    if (state) {
        Py_DECREF(state->helper);
        Py_DECREF(state->Window_type);
    }
}

static PyModuleDef module_def = {
    PyModuleDef_HEAD_INIT, "glwindow", NULL, sizeof(ModuleState), module_methods, module_slots, NULL, NULL, (freefunc)module_free,
};

extern "C" PyObject * PyInit_glwindow() {
    return PyModuleDef_Init(&module_def);
}
