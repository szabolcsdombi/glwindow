import glwindow
import zengl

window = glwindow.init(cursor=False)
ctx = zengl.context()

import os
img = ctx.image(window.size, 'rgba8unorm', os.urandom(window.size[0] * window.size[1] * 4))

while window.update():
    ctx.new_frame()
    if window.key_pressed('a'):
        img.write(os.urandom(window.size[0] * window.size[1] * 4))
    img.blit()
    ctx.end_frame()
