if [[ -z "$DISPLAY" && -z "$SSH_CLIENT" ]] && ! ps -e | grep sway && command -v sway 1>/dev/null; then
    sway
fi
