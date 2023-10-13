import sys

from setuptools import Extension, setup

libraries = []

define_macros = [
    ('Py_LIMITED_API', 0x030A0000),
    ('PY_SSIZE_T_CLEAN', None),
]

stubs = {
    'packages': ['glwindow-stubs'],
    'package_data': {'glwindow-stubs': ['__init__.pyi']},
    'include_package_data': True,
}

if sys.platform.startswith('win'):
    libraries.extend(['gdi32', 'user32', 'opengl32', 'dwmapi', 'powrprof', 'openal32'])

else:
    define_macros.append(('USE_SDL', None))
    libraries.extend(['SDL2', 'GL', 'openal'])

ext = Extension(
    name='glwindow',
    sources=['./glwindow.cpp'],
    define_macros=define_macros,
    py_limited_api=True,
    include_dirs=['./include/'],
    library_dirs=['./libs/'],
    libraries=libraries,
)

setup(
    name='glwindow',
    version='0.1.0',
    ext_modules=[ext],
    **stubs,
)
