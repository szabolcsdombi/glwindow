from setuptools import Extension, setup

ext = Extension(
    name='glwindow',
    sources=['./glwindow.cpp'],
    define_macros=[
        ('Py_LIMITED_API', 0x030A0000),
        ('PY_SSIZE_T_CLEAN', None),
    ],
    py_limited_api=True,
    include_dirs=['./include/'],
    library_dirs=['./libs/'],
    libraries=['gdi32', 'user32', 'opengl32', 'dwmapi', 'powrprof', 'openal32'],
)

setup(
    name='glwindow',
    version='0.1.0',
    ext_modules=[ext],
)
