# Colors
export COLOR_FOREGROUND="#c9b3b4"

if [[ $HOST = ShagBox ]]; then
    $HOME/.bin/newout &>/dev/null
elif [[ "$TTY" = "/dev/tty1" ]]; then
    exec startx
fi
