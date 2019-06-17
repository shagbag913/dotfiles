if [[ -n "$SSH_CLIENT" ]] || [[ -n "$SSH_TTY" ]]; then
    export SSH=true
fi
if [[ ! $DISPLAY && $XDG_VTNR = 1 && "$SSH" != true ]] && ! ps -e | grep bspwm; then
    exec startx
fi
