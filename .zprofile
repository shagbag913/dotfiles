if [[ $HOST = ShagBox ]]; then
    $HOME/.bin/newout &>/dev/null
fi

if [[ -z "$SSH_CLIENT" && "$TTY" = "/dev/tty1" ]]; then
    exec startx
fi
