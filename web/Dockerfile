FROM python:3.11.3
ENV EMSDK=/opt/emsdk EMSDK_NODE=/opt/emsdk/node/16.20.0_64bit/bin/node \
    PATH=/opt/emsdk:/opt/emsdk/upstream/emscripten:/opt/emsdk/node/16.20.0_64bit/bin:$PATH
RUN git clone https://github.com/emscripten-core/emsdk.git $EMSDK &&\
    emsdk install 3.1.45 && emsdk activate 3.1.45 && pip install pyodide-build==0.24.1 &&\
    python -c "from pyodide_build.build_env import init_environment; init_environment()"

RUN git clone https://github.com/szabolcsdombi/zengl.git /zengl && git -C /zengl checkout f7da48c
RUN pyodide build /zengl -o /web/

COPY include /glwindow/include
COPY setup.py README.md glwindow_web.js glwindow_web.cpp _glwindow.py /glwindow/
RUN pyodide build /glwindow -o /web/

COPY web/public /web
CMD python -m http.server -d /web/
