SERVER_COLOR = "blue"
CLIENT_COLOR = "yellow"
ACTION_COLOR = "green"
ERROR_COLOR = "red"
HIGH_LIGHT = "magenta"
DATA_COLOR = "cyan"

colors = {
    'black': '\033[30m',
    'red': '\033[31m',
    'green': '\033[32m',
    'yellow': '\033[33m',
    'blue': '\033[34m',
    'purple': '\033[35m',
    'cyan': '\033[36m',
    'white': '\033[37m',
    'dark_black': '\033[90m',
    'dark_red': '\033[91m',
    'dark_green': '\033[92m',
    'dark_yellow': '\033[93m',
    'dark_blue': '\033[94m',
    'dark_purple': '\033[95m',
    'dark_cyan': '\033[96m',
    'dark_white': '\033[97m',
    'reset': '\033[0m'
}


def log(message, color):
    if color in colors:
        colored_message = f"{colors[color]}{message}{colors['reset']}"
        print(colored_message)

    else:
        print(message)
