import sys

from setuptools import Extension, setup

include_dirs = ['./include/'],
library_dirs = ['./libs/'],
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
    libraries.extend(['SDL2', 'openal'])

if sys.platform.startswith('darwin'):
    import os
    import subprocess

    sdl = subprocess.check_output(['brew', '--prefix', 'sdl2']).strip().decode()
    openal = subprocess.check_output(['brew', '--prefix', 'openal-soft']).strip().decode()

    include_dirs.extend([os.path.join(sdl, 'include'), os.path.join(openal, 'include')])
    library_dirs.extend([os.path.join(sdl, 'lib'), os.path.join(openal, 'lib')])

ext = Extension(
    name='glwindow',
    sources=['./glwindow.cpp'],
    define_macros=define_macros,
    py_limited_api=True,
    libraries=libraries,
)

setup(
    name='glwindow',
    version='0.1.0',
    ext_modules=[ext],
    **stubs,
)
