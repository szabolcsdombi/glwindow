import glwindow
import zengl

glwindow.init()

window = glwindow.get_window()
audio = glwindow.get_audio()

ctx = zengl.context(glwindow.get_loader())
image = ctx.image(window.size, 'rgba8unorm', texture=False)

print(window)
print(audio)


class App:
    def update(self):
        ctx.new_frame()
        image.clear_value = (0.5, 0.5, 0.5, 1.0)
        image.clear()
        image.blit()
        ctx.end_frame()


if __name__ == '__main__':
    glwindow.run(app=App())
