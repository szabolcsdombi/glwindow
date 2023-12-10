#include <Python.h>
#include <structmember.h>

#include <AL/al.h>
#include <AL/alc.h>

#define QOI_NO_STDIO
#define QOI_IMPLEMENTATION
#include <qoi.h>

#define QOA_NO_STDIO
#define QOA_IMPLEMENTATION
#include <qoa.h>

#define STB_RECT_PACK_IMPLEMENTATION
#define STBRP_STATIC
#include "stb_rect_pack.h"

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#include "stb_truetype.h"

struct Window {
    PyObject_HEAD

    PyObject * keys;
    PyObject * size;
    PyObject * view;
    PyObject * mouse;
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
    PyTypeObject * Audio_type;

    Window * window;
    Audio * audio;
};

static void add_key(PyObject * keys, const char * key, int value) {
    PyObject * v = PyLong_FromLong(value);
    PyDict_SetItemString(keys, key, v);
    Py_DECREF(v);
}

#ifndef USE_SDL

#define WINVER 0x0601
#include <Windows.h>
#include <Dwmapi.h>
#include <PowerSetting.h>
#include <ShellScalingApi.h>
#include <GL/GL.h>

HMODULE opengl;
HWND hwnd;
HDC hdc;
HGLRC hrc;

int width;
int height;
int focus;

int mouse_x;
int mouse_y;

unsigned char keys[256];

LRESULT CALLBACK WindowProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) {
    switch (umsg) {
        case WM_CLOSE: {
            PostQuitMessage(0);
            return 0;
        }
        case WM_ACTIVATE: {
            focus = !!wparam;
            break;
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

int init_window(Window * window) {
    HANDLE process = GetCurrentProcess();
    SetPriorityClass(process, HIGH_PRIORITY_CLASS);
    SetProcessPriorityBoost(process, false);
    PowerSetActiveScheme(NULL, &GUID_MIN_POWER_SAVINGS);

    SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

    opengl = GetModuleHandle("opengl32");

    HINSTANCE hinst = GetModuleHandle(NULL);
    HCURSOR hcursor = (HCURSOR)LoadCursor(NULL, IDC_ARROW);
    WNDCLASS wnd_class = {CS_OWNDC, WindowProc, 0, 0, hinst, NULL, hcursor, NULL, NULL, "mywindow"};
    RegisterClass(&wnd_class);

    width = 1280;
    height = 720;
    window->size = Py_BuildValue("(ii)", width, height);
    window->view = Py_BuildValue("(ii)", width, height);
    window->mouse = Py_BuildValue("(ii)", 0, 0);

    PyObject * keys = PyDict_New();
    add_key(keys, "mouse1", 1);
    add_key(keys, "mouse2", 2);
    add_key(keys, "mouse3", 4);
    add_key(keys, "tab", VK_TAB);
    add_key(keys, "left_arrow", VK_LEFT);
    add_key(keys, "right_arrow", VK_RIGHT);
    add_key(keys, "up_arrow", VK_UP);
    add_key(keys, "down_arrow", VK_DOWN);
    add_key(keys, "page_up", VK_PRIOR);
    add_key(keys, "page_down", VK_NEXT);
    add_key(keys, "home", VK_HOME);
    add_key(keys, "end", VK_END);
    add_key(keys, "insert", VK_INSERT);
    add_key(keys, "delete", VK_DELETE);
    add_key(keys, "backspace", VK_BACK);
    add_key(keys, "space", VK_SPACE);
    add_key(keys, "enter", VK_RETURN);
    add_key(keys, "escape", VK_ESCAPE);
    add_key(keys, "apostrophe", VK_OEM_7);
    add_key(keys, "comma", VK_OEM_COMMA);
    add_key(keys, "minus", VK_OEM_MINUS);
    add_key(keys, "period", VK_OEM_PERIOD);
    add_key(keys, "slash", VK_OEM_2);
    add_key(keys, "semicolon", VK_OEM_1);
    add_key(keys, "equal", VK_OEM_PLUS);
    add_key(keys, "left_bracket", VK_OEM_4);
    add_key(keys, "backslash", VK_OEM_5);
    add_key(keys, "right_bracket", VK_OEM_6);
    add_key(keys, "grave_accent", VK_OEM_3);
    add_key(keys, "caps_lock", VK_CAPITAL);
    add_key(keys, "scroll_lock", VK_SCROLL);
    add_key(keys, "num_lock", VK_NUMLOCK);
    add_key(keys, "print_screen", VK_SNAPSHOT);
    add_key(keys, "pause", VK_PAUSE);
    add_key(keys, "keypad_0", VK_NUMPAD0);
    add_key(keys, "keypad_1", VK_NUMPAD1);
    add_key(keys, "keypad_2", VK_NUMPAD2);
    add_key(keys, "keypad_3", VK_NUMPAD3);
    add_key(keys, "keypad_4", VK_NUMPAD4);
    add_key(keys, "keypad_5", VK_NUMPAD5);
    add_key(keys, "keypad_6", VK_NUMPAD6);
    add_key(keys, "keypad_7", VK_NUMPAD7);
    add_key(keys, "keypad_8", VK_NUMPAD8);
    add_key(keys, "keypad_9", VK_NUMPAD9);
    add_key(keys, "keypad_decimal", VK_DECIMAL);
    add_key(keys, "keypad_divide", VK_DIVIDE);
    add_key(keys, "keypad_multiply", VK_MULTIPLY);
    add_key(keys, "keypad_subtract", VK_SUBTRACT);
    add_key(keys, "keypad_add", VK_ADD);
    add_key(keys, "left_shift", VK_LSHIFT);
    add_key(keys, "left_ctrl", VK_LCONTROL);
    add_key(keys, "left_alt", VK_LMENU);
    add_key(keys, "left_super", VK_LWIN);
    add_key(keys, "right_shift", VK_RSHIFT);
    add_key(keys, "right_ctrl", VK_RCONTROL);
    add_key(keys, "right_alt", VK_RMENU);
    add_key(keys, "right_super", VK_RWIN);
    add_key(keys, "menu", VK_APPS);
    add_key(keys, "0", '0');
    add_key(keys, "1", '1');
    add_key(keys, "2", '2');
    add_key(keys, "3", '3');
    add_key(keys, "4", '4');
    add_key(keys, "5", '5');
    add_key(keys, "6", '6');
    add_key(keys, "7", '7');
    add_key(keys, "8", '8');
    add_key(keys, "9", '9');
    add_key(keys, "a", 'A');
    add_key(keys, "b", 'B');
    add_key(keys, "c", 'C');
    add_key(keys, "d", 'D');
    add_key(keys, "e", 'E');
    add_key(keys, "f", 'F');
    add_key(keys, "g", 'G');
    add_key(keys, "h", 'H');
    add_key(keys, "i", 'I');
    add_key(keys, "j", 'J');
    add_key(keys, "k", 'K');
    add_key(keys, "l", 'L');
    add_key(keys, "m", 'M');
    add_key(keys, "n", 'N');
    add_key(keys, "o", 'O');
    add_key(keys, "p", 'P');
    add_key(keys, "q", 'Q');
    add_key(keys, "r", 'R');
    add_key(keys, "s", 'S');
    add_key(keys, "t", 'T');
    add_key(keys, "u", 'U');
    add_key(keys, "v", 'V');
    add_key(keys, "w", 'W');
    add_key(keys, "x", 'X');
    add_key(keys, "y", 'Y');
    add_key(keys, "z", 'Z');
    add_key(keys, "f1", VK_F1);
    add_key(keys, "f2", VK_F2);
    add_key(keys, "f3", VK_F3);
    add_key(keys, "f4", VK_F4);
    add_key(keys, "f5", VK_F5);
    add_key(keys, "f6", VK_F6);
    add_key(keys, "f7", VK_F7);
    add_key(keys, "f8", VK_F8);
    add_key(keys, "f9", VK_F9);
    add_key(keys, "f10", VK_F10);
    add_key(keys, "f11", VK_F11);
    add_key(keys, "f12", VK_F12);
    add_key(keys, "f13", VK_F13);
    add_key(keys, "f14", VK_F14);
    add_key(keys, "f15", VK_F15);
    add_key(keys, "f16", VK_F16);
    add_key(keys, "f17", VK_F17);
    add_key(keys, "f18", VK_F18);
    add_key(keys, "f19", VK_F19);
    add_key(keys, "f20", VK_F20);
    add_key(keys, "f21", VK_F21);
    add_key(keys, "f22", VK_F22);
    add_key(keys, "f23", VK_F23);
    add_key(keys, "f24", VK_F24);
    add_key(keys, "app_back", VK_BROWSER_BACK);
    add_key(keys, "app_forward", VK_BROWSER_FORWARD);
    window->keys = keys;

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
        return -1;
    }

    hdc = GetDC(hwnd);

    DWORD pfd_flags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_GENERIC_ACCELERATED | PFD_DEPTH_DONTCARE | PFD_DOUBLEBUFFER;
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
        return -1;
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
        return -1;
    }

    wglMakeCurrent(hdc, hrc);
    wglSwapIntervalEXT(1);
    return 0;
}

int run_main_loop(Window * window) {
    while (true) {
        MSG msg = {};
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                return 0;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (focus) {
            GetKeyboardState(keys);
            POINT cursor = {};
            POINT zero = {};
            GetCursorPos(&cursor);
            ClientToScreen(hwnd, &zero);
            mouse_x = cursor.x - zero.x;
            mouse_y = height - (cursor.y - zero.y) - 1;
            Py_DECREF(window->mouse);
            window->mouse = Py_BuildValue("(ii)", mouse_x, mouse_y);
        } else {
            memset(keys, 0, sizeof(keys));
        }

        PyObject * res = PyObject_CallMethod(window->app, "update", NULL);
        if (!res) {
            return -1;
        }
        Py_XDECREF(res);

        SwapBuffers(hdc);
        DwmFlush();
    }

    return 0;
}

void * lookup_opengl_function(const char * name) {
    void * proc = (void *)GetProcAddress(opengl, name);
    if (!proc) {
        proc = (void *)wglGetProcAddress(name);
    }
    return proc;
}

#else

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <dlfcn.h>

#ifdef __APPLE__
#define LIBGL "/System/Library/Frameworks/OpenGL.framework/OpenGL"
#else
#define LIBGL "libGL.so"
#endif

void * opengl;
SDL_Window * wnd;
SDL_GLContext glctx;

int width;
int height;

int init_window(Window * window) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        PyErr_SetString(PyExc_RuntimeError, SDL_GetError());
        return -1;
    }

    opengl = dlopen(LIBGL, RTLD_LAZY);

    width = 1280;
    height = 720;
    window->size = Py_BuildValue("(ii)", width, height);

    wnd = SDL_CreateWindow("OpenGL Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (!wnd) {
        PyErr_SetString(PyExc_RuntimeError, SDL_GetError());
        return -1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    glctx = SDL_GL_CreateContext(wnd);
    if (!glctx) {
        PyErr_SetString(PyExc_RuntimeError, SDL_GetError());
        return -1;
    }

    SDL_GL_SetSwapInterval(1);
    return 0;
}

int run_main_loop(Window * window) {
    while (true) {
        SDL_Event e;
        SDL_PollEvent(&e);
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                return 0;
            }
        }

        PyObject * res = PyObject_CallMethod(window->app, "update", NULL);
        if (!res) {
            return -1;
        }
        Py_XDECREF(res);

        SDL_GL_SwapWindow(wnd);
    }

    return 0;
}

void * lookup_opengl_function(const char * name) {
    return (void *)dlsym(opengl, name);
}

#endif

int init_audio(Audio * audio) {
    audio->device = alcOpenDevice(NULL);
    if (!audio->device) {
        PyErr_BadInternalCall();
        return -1;
    }

    audio->context = alcCreateContext(audio->device, NULL);
    if (!audio->context) {
        PyErr_BadInternalCall();
        return -1;
    }

    alcMakeContextCurrent(audio->context);
    return 0;
}

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

PyObject * meth_load_font(PyObject * self, PyObject * args, PyObject * kwargs) {
    const char * keywords[] = {"size", "fonts", "font_sizes", "code_points", "oversampling", "padding", NULL};

    int width, height;
    PyObject * fonts_lst;
    PyObject * font_sizes_lst;
    PyObject * code_points_lst;
    int oversampling = 1;
    int padding = 1;

    int args_ok = PyArg_ParseTupleAndKeywords(
        args, kwargs, "(ii)OOO|ii", (char **)keywords,
        &width, &height,
        &fonts_lst,
        &font_sizes_lst,
        &code_points_lst,
        &oversampling,
        &padding
    );

    if (!args_ok) {
        return NULL;
    }

    if (!PyList_Check(fonts_lst) || !PyList_Check(code_points_lst) || !PyList_Check(font_sizes_lst)) {
        return NULL;
    }

    int num_fonts = (int)PyList_Size(fonts_lst);
    int num_glyphs = (int)PyList_Size(code_points_lst);
    int num_sizes = (int)PyList_Size(font_sizes_lst);

    int * glyphs = (int *)malloc(num_glyphs * 4);
    for (int i = 0; i < num_glyphs; ++i) {
        glyphs[i] = PyLong_AsLong(PyList_GetItem(code_points_lst, i));
    }

    float * sizes = (float *)malloc(num_sizes * 4);
    for (int i = 0; i < num_sizes; ++i) {
        sizes[i] = (float)PyFloat_AsDouble(PyList_GetItem(font_sizes_lst, i));
    }

    PyObject * pixels = PyBytes_FromStringAndSize(NULL, width * height * 4);
    PyObject * rects = PyBytes_FromStringAndSize(NULL, sizeof(stbtt_packedchar) * num_fonts * num_glyphs * num_sizes);

    stbtt_pack_context pc;
    stbtt_packedchar * cd = (stbtt_packedchar *)PyBytes_AsString(rects);
    unsigned char * atlas = (unsigned char *)PyBytes_AsString(pixels);
    stbtt_pack_range * ranges = (stbtt_pack_range *)malloc(sizeof(stbtt_pack_range) * num_sizes);

    stbtt_PackBegin(&pc, atlas, width, height, 0, padding, NULL);
    for (int i = 0; i < num_fonts; ++i) {
        unsigned char * font_data = (unsigned char *)PyBytes_AsString(PyList_GetItem(fonts_lst, i));
        stbtt_PackSetOversampling(&pc, oversampling, oversampling);
        for (int j = 0; j < num_sizes; ++j) {
            ranges[j] = {sizes[j], 0, glyphs, num_glyphs, cd + num_glyphs * (num_sizes * i + j)};
        };
        stbtt_PackFontRanges(&pc, font_data, 0, ranges, num_sizes);
    }
    stbtt_PackEnd(&pc);
    free(glyphs);
    free(sizes);
    free(ranges);

    int x = width * height;
    int y = x * 4;
    while (x--) {
        atlas[--y] = atlas[x];
        atlas[--y] = atlas[x];
        atlas[--y] = atlas[x];
        atlas[--y] = atlas[x];
    }

    return Py_BuildValue("(NN)", pixels, rects);
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

    if (init_window(module_state->window)) {
        return NULL;
    }

    if (init_audio(module_state->audio)) {
        return NULL;
    }

    PyObject * zengl = PyImport_ImportModule("zengl");
    if (!zengl) {
        return NULL;
    }

    PyObject * zengl_init = PyObject_CallMethod(zengl, "init", "(N)", self);
    if (!zengl_init) {
        return NULL;
    }

    Py_DECREF(zengl_init);
    Py_DECREF(zengl);

    module_state->window->app = PyObject_CallFunction(app, NULL);

    if (run_main_loop(module_state->window)) {
        return NULL;
    }

    Py_RETURN_NONE;
}

static PyObject * Window_meth_key_down(Window * self, PyObject * arg) {
    PyObject * key = PyDict_GetItem(self->keys, arg);
    if (!key) {
        return NULL;
    }
    int k = PyLong_AsLong(key);
    if (keys[k] & 0x80) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

static PyObject * meth_load_opengl_function(PyObject * self, PyObject * arg) {
    if (!PyUnicode_CheckExact(arg)) {
        return NULL;
    }
    PyObject * encoded = PyUnicode_AsEncodedString(arg, NULL, NULL);
    const char * name = PyBytes_AsString(encoded);
    void * proc = lookup_opengl_function(name);
    Py_DECREF(encoded);
    return PyLong_FromVoidPtr(proc);
}

static void default_dealloc(PyObject * self) {
    PyObject_Free(self);
}

static PyMethodDef Window_methods[] = {
    {"key_down", (PyCFunction)Window_meth_key_down, METH_O, NULL},
    {0},
};

static PyMemberDef Window_members[] = {
    {"size", T_OBJECT, offsetof(Window, size), READONLY},
    {"view", T_OBJECT, offsetof(Window, view), READONLY},
    {"mouse", T_OBJECT, offsetof(Window, mouse), READONLY},
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
    {"load_font", (PyCFunction)meth_load_font, METH_VARARGS | METH_KEYWORDS, NULL},
    {"load_opengl_function", (PyCFunction)meth_load_opengl_function, METH_O, NULL},
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
