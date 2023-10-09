import glwindow
import zengl


class App:
    def __init__(self):
        self.wnd = glwindow.get_window()
        self.audio = glwindow.get_audio()
        self.ctx = zengl.context(glwindow.get_loader())
        self.image = self.ctx.image(self.wnd.size, 'rgba8unorm', texture=False)

    def update(self):
        self.ctx.new_frame()
        self.image.clear()
        self.image.blit()
        self.ctx.end_frame()


if __name__ == '__main__':
    glwindow.run(app=App)
