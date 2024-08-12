from typing import Literal

Key = Literal[
    'mouse1',
    'mouse2',
    'mouse3',
    'tab',
    'left_arrow',
    'right_arrow',
    'up_arrow',
    'down_arrow',
    'page_up',
    'page_down',
    'home',
    'end',
    'insert',
    'delete',
    'backspace',
    'space',
    'enter',
    'escape',
    'apostrophe',
    'comma',
    'minus',
    'period',
    'slash',
    'semicolon',
    'equal',
    'left_bracket',
    'backslash',
    'right_bracket',
    'grave_accent',
    'caps_lock',
    'scroll_lock',
    'num_lock',
    'print_screen',
    'pause',
    'keypad_0',
    'keypad_1',
    'keypad_2',
    'keypad_3',
    'keypad_4',
    'keypad_5',
    'keypad_6',
    'keypad_7',
    'keypad_8',
    'keypad_9',
    'keypad_decimal',
    'keypad_divide',
    'keypad_multiply',
    'keypad_subtract',
    'keypad_add',
    'left_shift',
    'left_ctrl',
    'left_alt',
    'left_super',
    'right_shift',
    'right_ctrl',
    'right_alt',
    'right_super',
    'menu',
    '0',
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    'a',
    'b',
    'c',
    'd',
    'e',
    'f',
    'g',
    'h',
    'i',
    'j',
    'k',
    'l',
    'm',
    'n',
    'o',
    'p',
    'q',
    'r',
    's',
    't',
    'u',
    'v',
    'w',
    'x',
    'y',
    'z',
    'f1',
    'f2',
    'f3',
    'f4',
    'f5',
    'f6',
    'f7',
    'f8',
    'f9',
    'f10',
    'f11',
    'f12',
    'f13',
    'f14',
    'f15',
    'f16',
    'f17',
    'f18',
    'f19',
    'f20',
    'f21',
    'f22',
    'f23',
    'f24',
]

class Window:
    size: tuple[int, int]
    mouse: tuple[int, int]
    mouse_delta: tuple[int, int]
    mouse_wheel: int
    frame_time: float

    def update(self) -> bool: ...
    def key_pressed(self, key: Key) -> bool: ...
    def key_released(self, key: Key) -> bool: ...
    def key_down(self, key: Key) -> bool: ...
    def text_input(self) -> str: ...

def init(cursor: bool = True) -> Window: ...
def get_window() -> Window: ...
