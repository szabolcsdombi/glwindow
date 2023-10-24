def init_glwindow_web(window, app_class, glwindow_js):
    import js
    import pyodide
    import pyodide_js
    import zengl

    setup_function = js.eval(glwindow_js)
    wnd = setup_function(pyodide_js)

    zengl.init(wnd.gl)

    window._size = int(wnd.width), int(wnd.height)
    window._view = window._size

    app = app_class()
    window._app = app

    @pyodide.ffi.create_proxy
    def update(view_width, view_height):
        window._view = int(view_width), int(view_height)
        app.update()

    wnd.setup_render(update)
