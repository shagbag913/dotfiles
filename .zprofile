if [[ $HOST = ShagBox ]]; then
    $HOME/.bin/newout &>/dev/null
elif [[ "$TTY" = "/dev/tty1" ]]; then
    exec startx
fi
