#include <Python.h>
#include <structmember.h>

#define WINVER 0x0601
#include <Windows.h>
#include <Dwmapi.h>
#include <PowerSetting.h>
#include <GL/GL.h>

#include <AL/al.h>
#include <AL/alc.h>

#define QOI_NO_STDIO
#define QOI_IMPLEMENTATION
#include <qoi.h>

#define QOA_NO_STDIO
#define QOA_IMPLEMENTATION
#include <qoa.h>

struct Window {
    PyObject_HEAD

    PyObject * size;
    PyObject * app;
};

struct Loader {
    PyObject_HEAD
};

struct Audio {
    PyObject_HEAD

    ALCdevice * device;
    ALCcontext * context;
};

struct ModuleState {
    PyObject * helper;

    PyTypeObject * Window_type;
    PyTypeObject * Loader_type;
    PyTypeObject * Audio_type;

    Window * window;
    Loader * loader;
    Audio * audio;
};

HWND hwnd;
HDC hdc;
HGLRC hrc;

int width;
int height;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) {
    switch (umsg) {
        case WM_CLOSE: {
            PostQuitMessage(0);
            return 0;
        }
        case WM_KEYDOWN:
        case WM_KEYUP: {
            return 0;
        }
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP: {
            static bool alt = false;
            if (wparam == VK_MENU) {
                alt = umsg == WM_SYSKEYDOWN;
                return 0;
            }
            if (alt && wparam == VK_F4 && umsg == WM_SYSKEYDOWN) {
                PostQuitMessage(0);
                return 0;
            }
            return 1;
        }
        case WM_SYSCHAR: {
            return 1;
        }
    }
    return DefWindowProc(hwnd, umsg, wparam, lparam);
}

PyObject * meth_get_window(PyObject * self) {
    ModuleState * module_state = (ModuleState *)PyModule_GetState(self);
    return Py_XNewRef(module_state->window);
}

PyObject * meth_get_loader(PyObject * self) {
    ModuleState * module_state = (ModuleState *)PyModule_GetState(self);
    return Py_XNewRef(module_state->loader);
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

    Window * window = PyObject_New(Window, module_state->Window_type);
    Loader * loader = PyObject_New(Loader, module_state->Loader_type);
    Audio * audio = PyObject_New(Audio, module_state->Audio_type);

    module_state->window = window;
    module_state->loader = loader;
    module_state->audio = audio;

    window->app = Py_None;

    HANDLE process = GetCurrentProcess();
    SetPriorityClass(process, HIGH_PRIORITY_CLASS);
    SetProcessPriorityBoost(process, false);
    PowerSetActiveScheme(NULL, &GUID_MIN_POWER_SAVINGS);

    HINSTANCE hinst = GetModuleHandle(NULL);
    HCURSOR hcursor = (HCURSOR)LoadCursor(NULL, IDC_ARROW);
    WNDCLASS wnd_class = {CS_OWNDC, WindowProc, 0, 0, hinst, NULL, hcursor, NULL, NULL, "mywindow"};
    RegisterClass(&wnd_class);

    width = 1280;
    height = 720;
    window->size = Py_BuildValue("(ii)", width, height);

    int style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE;
    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);

    RECT rect = {0, 0, width, height};
    AdjustWindowRect(&rect, style, false);

    int w = rect.right - rect.left;
    int h = rect.bottom - rect.top;
    int x = (sw - w) / 2;
    int y = (sh - h) / 2;

    hwnd = CreateWindow("mywindow", "OpenGL Window", style, x, y, w, h, NULL, NULL, hinst, NULL);
    if (!hwnd) {
        PyErr_BadInternalCall();
        return NULL;
    }

    hdc = GetDC(hwnd);

    DWORD pfd_flags = PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_GENERIC_ACCELERATED;
    PIXELFORMATDESCRIPTOR pfd = {sizeof(PIXELFORMATDESCRIPTOR), 1, pfd_flags, 0, 32};

    int pixelformat = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, pixelformat, &pfd);

    HGLRC loader_hglrc = wglCreateContext(hdc);
    wglMakeCurrent(hdc, loader_hglrc);

    HGLRC (WINAPI * wglCreateContextAttribsARB)(HDC hdc, HGLRC hrc, const int * attrib_list);
    BOOL (WINAPI * wglSwapIntervalEXT)(int interval);

    *(PROC *)&wglCreateContextAttribsARB = wglGetProcAddress("wglCreateContextAttribsARB");
    *(PROC *)&wglSwapIntervalEXT = wglGetProcAddress("wglSwapIntervalEXT");

    if (!wglCreateContextAttribsARB || !wglSwapIntervalEXT) {
        PyErr_BadInternalCall();
        return NULL;
    }

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(loader_hglrc);

    const int WGL_CONTEXT_PROFILE_MASK = 0x9126;
    const int WGL_CONTEXT_CORE_PROFILE_BIT = 0x0001;
    const int WGL_CONTEXT_MAJOR_VERSION = 0x2091;
    const int WGL_CONTEXT_MINOR_VERSION = 0x2092;
    const int WGL_CONTEXT_FLAGS = 0x2094;
    const int WGL_CONTEXT_FORWARD_COMPATIBLE_BIT = 0x0002;
    const int WGL_CONTEXT_FLAG_NO_ERROR_BIT = 0x0008;

    int attribs[] = {
        WGL_CONTEXT_FLAGS, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT,
        WGL_CONTEXT_PROFILE_MASK, WGL_CONTEXT_CORE_PROFILE_BIT,
        WGL_CONTEXT_MAJOR_VERSION, 3,
        WGL_CONTEXT_MINOR_VERSION, 3,
        0, 0,
    };

    hrc = wglCreateContextAttribsARB(hdc, NULL, attribs);

    if (!hrc) {
        PyErr_BadInternalCall();
        return NULL;
    }

    wglMakeCurrent(hdc, hrc);
    wglSwapIntervalEXT(1);

    audio->device = alcOpenDevice(NULL);
    if (!audio->device) {
        PyErr_BadInternalCall();
        return NULL;
    }

    audio->context = alcCreateContext(audio->device, NULL);
    if (!audio->context) {
        PyErr_BadInternalCall();
        return NULL;
    }

    alcMakeContextCurrent(audio->context);

    PyObject * old_app = module_state->window->app;
    module_state->window->app = PyObject_CallFunction(app, NULL);
    Py_DECREF(old_app);

    while (true) {
        SwapBuffers(hdc);
        DwmFlush();

        MSG msg = {};
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                Py_RETURN_NONE;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        PyObject * res = PyObject_CallMethod(module_state->window->app, "update", NULL);
        if (!res) {
            return NULL;
        }
        Py_XDECREF(res);
    }

    Py_RETURN_NONE;
}

static PyObject * Loader_meth_load_opengl_function(PyObject * self, PyObject * arg) {
    if (!PyUnicode_CheckExact(arg)) {
        return NULL;
    }
    HMODULE module = GetModuleHandle("opengl32");
    if (!module) {
        return NULL;
    }
    PyObject * encoded = PyUnicode_AsEncodedString(arg, NULL, NULL);
    const char * name = PyBytes_AsString(encoded);
    void * proc = (void *)GetProcAddress(module, name);
    if (!proc) {
        proc = wglGetProcAddress(name);
    }
    Py_DECREF(encoded);
    return PyLong_FromVoidPtr(proc);
}

static void default_dealloc(PyObject * self) {
    PyObject_Free(self);
}

static PyMethodDef Window_methods[] = {
    {0},
};

static PyMemberDef Window_members[] = {
    {"size", T_OBJECT, offsetof(Window, size), READONLY},
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

static PyMethodDef Loader_methods[] = {
    {"load_opengl_function", (PyCFunction)Loader_meth_load_opengl_function, METH_O},
    {0},
};

static PyType_Slot Loader_slots[] = {
    {Py_tp_methods, Loader_methods},
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


static PyType_Spec Window_spec = {"Window", sizeof(Window), 0, Py_TPFLAGS_DEFAULT, Window_slots};
static PyType_Spec Loader_spec = {"Loader", sizeof(Loader), 0, Py_TPFLAGS_DEFAULT, Loader_slots};
static PyType_Spec Audio_spec = {"Audio", sizeof(Audio), 0, Py_TPFLAGS_DEFAULT, Audio_slots};

static int module_exec(PyObject * self) {
    ModuleState * module_state = (ModuleState *)PyModule_GetState(self);

    module_state->helper = PyImport_ImportModule("_glwindow");
    if (!module_state->helper) {
        return -1;
    }

    module_state->window = NULL;
    module_state->loader = NULL;
    module_state->audio = NULL;

    module_state->Window_type = (PyTypeObject *)PyType_FromSpec(&Window_spec);
    module_state->Loader_type = (PyTypeObject *)PyType_FromSpec(&Loader_spec);
    module_state->Audio_type = (PyTypeObject *)PyType_FromSpec(&Audio_spec);

    PyModule_AddObject(self, "Window", Py_NewRef(module_state->Window_type));
    PyModule_AddObject(self, "Loader", Py_NewRef(module_state->Loader_type));
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
    {"get_loader", (PyCFunction)meth_get_loader, METH_NOARGS, NULL},
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

extern PyObject * PyInit_glwindow() {
    return PyModuleDef_Init(&module_def);
}
