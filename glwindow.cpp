#define UNICODE
#define _UNICODE
#define WINVER 0x0601

#include <Windows.h>
#include <ShellScalingApi.h>
#include <PowerSetting.h>
#include <Dwmapi.h>
#include <hidusage.h>
#include <GL/GL.h>

#include <Python.h>
#include <structmember.h>

#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x0001
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_FLAGS_ARB 0x2094
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x0002
#define WGL_CONTEXT_FLAG_NO_ERROR_BIT_KHR 0x0008

HMODULE opengl;
HWND hwnd;
HDC hdc;
HGLRC hrc;

int width;
int height;
int focus;
int closing;

int mouse_x;
int mouse_y;
int mouse_dx;
int mouse_dy;
int mouse_wheel;

long long frequency;
long long last_frame_timestamp;

int text_input_length;
wchar_t text_input[256];
unsigned char prev_keys[256];
unsigned char keys[256];

struct Window {
    PyObject_HEAD
    PyObject * keys;
    PyObject * mouse;
    PyObject * mouse_delta;
    PyObject * mouse_wheel;
    PyObject * frame_time;
    PyObject * size;
};

static PyTypeObject * Window_type;
static Window * window;

static void add_key(PyObject * keys, const char * key, int value) {
    PyObject * v = PyLong_FromLong(value);
    PyDict_SetItemString(keys, key, v);
    Py_DECREF(v);
}

static PyObject * build_keys() {
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
    return keys;
}

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) {
    switch (umsg) {
        case WM_CLOSE: {
            closing = 1;
            return 0;
        }
        case WM_ACTIVATE: {
            focus = !!wparam;
            if (focus) {
                RECT rect = {};
                GetWindowRect(hwnd, &rect);
                ClipCursor(&rect);
            }
            break;
        }
        case WM_INPUT: {
            RAWINPUT raw = {};
            UINT size = sizeof(raw);
            GetRawInputData((HRAWINPUT)lparam, RID_INPUT, &raw, &size, sizeof(RAWINPUTHEADER));
            if (raw.header.dwType == RIM_TYPEMOUSE && raw.data.mouse.usFlags == MOUSE_MOVE_RELATIVE) {
                mouse_dx += raw.data.mouse.lLastX;
                mouse_dy -= raw.data.mouse.lLastY;
            }
            return 0;
        }
        case WM_CHAR: {
            text_input[text_input_length++] = wparam;
            return 0;
        }
        case WM_MOUSEWHEEL: {
            mouse_wheel += GET_WHEEL_DELTA_WPARAM(wparam) / WHEEL_DELTA;
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
                closing = 1;
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

static Window * meth_init(PyObject * self, PyObject * args, PyObject * kwargs) {
    const char * keywords[] = {"cursor", "pixel_format", NULL};

    int cursor = 1;
    int pixel_format = 1;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|$pi", (char **)keywords, &cursor, &pixel_format)) {
        return NULL;
    }

    window = PyObject_New(Window, Window_type);

    HANDLE process = GetCurrentProcess();
    SetPriorityClass(process, HIGH_PRIORITY_CLASS);
    SetProcessPriorityBoost(process, false);
    PowerSetActiveScheme(NULL, &GUID_MIN_POWER_SAVINGS);
    SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);

    HINSTANCE hinst = GetModuleHandle(NULL);
    HCURSOR hcursor = (HCURSOR)LoadCursor(NULL, IDC_ARROW);
    WNDCLASSW wnd_class = {CS_OWNDC, WindowProc, 0, 0, hinst, NULL, hcursor, NULL, NULL, L"mywindow"};
    RegisterClassW(&wnd_class);

    width = GetSystemMetrics(SM_CXSCREEN);
    height = GetSystemMetrics(SM_CYSCREEN);

    window->size = Py_BuildValue("(ii)", width, height);
    window->mouse = Py_BuildValue("(ii)", 0, 0);
    window->mouse_delta = Py_BuildValue("(ii)", 0, 0);
    window->mouse_wheel = PyLong_FromLong(0);
    window->frame_time = PyFloat_FromDouble(0.0);
    window->keys = build_keys();

    hwnd = CreateWindowW(L"mywindow", L"OpenGL Window", WS_POPUP | WS_VISIBLE, 0, 0, width, height, NULL, NULL, hinst, NULL);
    if (!hwnd) {
        PyErr_BadInternalCall();
        return NULL;
    }

    hdc = GetDC(hwnd);

    PIXELFORMATDESCRIPTOR pfd = {};
    DescribePixelFormat(hdc, pixel_format, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
    SetPixelFormat(hdc, pixel_format, &pfd);

    hrc = wglCreateContext(hdc);
    wglMakeCurrent(hdc, hrc);

    HGLRC (* wglCreateContextAttribsARB)(HDC, HGLRC, const int *);
    BOOL (* wglSwapIntervalEXT)(int);

    *(PROC *)&wglCreateContextAttribsARB = wglGetProcAddress("wglCreateContextAttribsARB");
    *(PROC *)&wglSwapIntervalEXT = wglGetProcAddress("wglSwapIntervalEXT");

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hrc);

    int attribs[] = {
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB | WGL_CONTEXT_FLAG_NO_ERROR_BIT_KHR,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        0, 0,
    };

    hrc = wglCreateContextAttribsARB(hdc, NULL, attribs);

    if (!hrc) {
        hrc = wglCreateContext(hdc);
    }

    wglMakeCurrent(hdc, hrc);

    if (wglSwapIntervalEXT) {
        wglSwapIntervalEXT(1);
    }

    RAWINPUTDEVICE rid = {};
    rid.usUsagePage = HID_USAGE_PAGE_GENERIC;
    rid.usUsage = HID_USAGE_GENERIC_MOUSE;
    rid.dwFlags = RIDEV_INPUTSINK;
    rid.hwndTarget = hwnd;
    RegisterRawInputDevices(&rid, 1, sizeof(rid));

    if (!cursor) {
        ShowCursor(false);
    }

    QueryPerformanceFrequency((LARGE_INTEGER *)&frequency);
    QueryPerformanceCounter((LARGE_INTEGER *)&last_frame_timestamp);

    Py_INCREF(window);
    return window;
}

static Window * meth_get_window(PyObject * self, PyObject * args) {
    Py_INCREF(window);
    return window;
}

static PyObject * Window_meth_update(Window * self, PyObject * args, PyObject * kwargs) {
    SwapBuffers(hdc);
    DwmFlush();

    memcpy(&prev_keys, &keys, sizeof(keys));
    mouse_dx = 0;
    mouse_dy = 0;
    mouse_wheel = 0;
    text_input_length = 0;

    MSG msg = {};
    while (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (closing) {
        Py_RETURN_FALSE;
    }

    GetKeyboardState(keys);
    POINT cursor = {};
    POINT zero = {};
    GetCursorPos(&cursor);
    ClientToScreen(hwnd, &zero);
    mouse_x = cursor.x - zero.x;
    mouse_y = height - (cursor.y - zero.y) - 1;

    if (!focus) {
        memset(keys, 0, sizeof(keys));
        mouse_x = 0;
        mouse_y = 0;
        mouse_dx = 0;
        mouse_dy = 0;
    }

    long long now;
    QueryPerformanceCounter((LARGE_INTEGER *)&now);
    double frame_time = (double)(now - last_frame_timestamp) / (double)frequency;
    last_frame_timestamp = now;

    Py_DECREF(window->mouse);
    Py_DECREF(window->mouse_delta);
    Py_DECREF(window->mouse_wheel);
    Py_DECREF(window->frame_time);

    window->mouse = Py_BuildValue("(ii)", mouse_x, mouse_y);
    window->mouse_delta = Py_BuildValue("(ii)", mouse_dx, mouse_dy);
    window->mouse_wheel = PyLong_FromLong(mouse_wheel);
    window->frame_time = PyFloat_FromDouble(frame_time);

    Py_RETURN_TRUE;
}

static PyObject * Window_meth_key_pressed(Window * self, PyObject * arg) {
    PyObject * key = PyDict_GetItem(self->keys, arg);
    if (!key) {
        return NULL;
    }
    int k = PyLong_AsLong(key);
    if ((keys[k] & 0x80) && (~prev_keys[k] & 0x80)) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}


static PyObject * Window_meth_key_released(Window * self, PyObject * arg) {
    PyObject * key = PyDict_GetItem(self->keys, arg);
    if (!key) {
        return NULL;
    }
    int k = PyLong_AsLong(key);
    if ((~keys[k] & 0x80) && (prev_keys[k] & 0x80)) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
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

static PyObject * Window_meth_text_input(Window * self, PyObject * args) {
    PyObject * res = PyList_New(text_input_length);
    for (int i = 0; i < text_input_length; i++) {
        PyList_SetItem(res, i, PyLong_FromLong(text_input[i]));
    }
    return res;
}

static void default_dealloc(PyObject * self) {
    PyObject_Free(self);
}

static PyMethodDef Window_methods[] = {
    {"update", (PyCFunction)Window_meth_update, METH_VARARGS | METH_KEYWORDS},
    {"key_pressed", (PyCFunction)Window_meth_key_pressed, METH_O},
    {"key_released", (PyCFunction)Window_meth_key_released, METH_O},
    {"key_down", (PyCFunction)Window_meth_key_down, METH_O},
    {"text_input", (PyCFunction)Window_meth_text_input, METH_NOARGS},
    {},
};

static PyMemberDef Window_members[] = {
    {"size", T_OBJECT, offsetof(Window, size), READONLY},
    {"mouse", T_OBJECT, offsetof(Window, mouse), READONLY},
    {"mouse_delta", T_OBJECT, offsetof(Window, mouse_delta), READONLY},
    {"mouse_wheel", T_OBJECT, offsetof(Window, mouse_wheel), READONLY},
    {"frame_time", T_OBJECT, offsetof(Window, frame_time), READONLY},
    {},
};

static PyType_Slot Window_slots[] = {
    {Py_tp_methods, Window_methods},
    {Py_tp_members, Window_members},
    {Py_tp_dealloc, default_dealloc},
    {},
};

static PyType_Spec Window_spec = {"Window", sizeof(Window), 0, Py_TPFLAGS_DEFAULT, Window_slots};

static PyMethodDef module_methods[] = {
    {"init", (PyCFunction)meth_init, METH_VARARGS | METH_KEYWORDS},
    {"get_window", (PyCFunction)meth_get_window, METH_NOARGS},
    {},
};

static PyModuleDef module_def = {PyModuleDef_HEAD_INIT, "glwindow", NULL, -1, module_methods};

extern "C" PyObject * PyInit_glwindow() {
    PyObject * module = PyModule_Create(&module_def);
    Window_type = (PyTypeObject *)PyType_FromSpec(&Window_spec);
    PyModule_AddObject(module, "Window", (PyObject *)Window_type);
    return module;
}
