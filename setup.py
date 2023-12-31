import os
import sys

from setuptools import Extension, setup

include_dirs = ['./include/']
library_dirs = ['./libs/']
sources = ['./glwindow.cpp']
libraries = []

define_macros = [
    ('Py_LIMITED_API', 0x030A0000),
]

stubs = {
    'packages': ['glwindow-stubs'],
    'package_data': {'glwindow-stubs': ['__init__.pyi']},
    'include_package_data': True,
}

if sys.platform.startswith('win'):
    libraries.extend(['gdi32', 'user32', 'opengl32', 'dwmapi', 'shcore', 'powrprof', 'openal32'])

else:
    define_macros.append(('USE_SDL', None))
    libraries.extend(['SDL2', 'openal'])

if sys.platform.startswith('darwin'):
    import subprocess

    sdl = subprocess.check_output(['brew', '--prefix', 'sdl2']).strip().decode()
    openal = subprocess.check_output(['brew', '--prefix', 'openal-soft']).strip().decode()

    include_dirs.extend([os.path.join(sdl, 'include'), os.path.join(openal, 'include')])
    library_dirs.extend([os.path.join(sdl, 'lib'), os.path.join(openal, 'lib')])

if os.getenv('PYODIDE'):
    import re

    with open('glwindow_web.js') as f:
        glwindow_js = re.sub(r'\s+', ' ', f.read(), flags=re.M)

    define_macros = [
        ('GLWINDOW_JS', f'"{glwindow_js}"'),
    ]
    sources = ['glwindow_web.cpp']
    libraries = []
    stubs = {}

ext = Extension(
    name='glwindow',
    sources=sources,
    define_macros=define_macros,
    py_limited_api=True,
    include_dirs=include_dirs,
    library_dirs=library_dirs,
    libraries=libraries,
)

setup(
    name='glwindow',
    version='0.2.0',
    ext_modules=[ext],
    py_modules=['_glwindow', 'glwnd'],
    **stubs,
)
