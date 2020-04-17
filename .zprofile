# Colors
export COLOR_FOREGROUND="#c9b3b4"

export BSPWM_SOCKET=/tmp/bspwm_1_0-socket

if [[ $HOST = ShagBox ]]; then
    $HOME/.bin/newout &>/dev/null
elif [[ "$TTY" = "/dev/tty1" ]]; then
    exec startx
fi
