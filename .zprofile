[[ -z "$DISPLAY" && -z "$SSH_CLIENT" ]] && ! ps -e | grep sway; then
    sway
fi
