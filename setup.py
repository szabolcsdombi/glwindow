from setuptools import Extension, setup

stubs = {
    'packages': ['glwindow-stubs'],
    'package_data': {'glwindow-stubs': ['__init__.pyi']},
    'include_package_data': True,
}

ext = Extension(
    name='glwindow',
    sources=['glwindow.cpp'],
    libraries=['gdi32', 'user32', 'opengl32', 'dwmapi', 'shcore', 'powrprof'],
)

setup(
    name='glwindow',
    version='0.3.0',
    ext_modules=[ext],
    **stubs,
)
