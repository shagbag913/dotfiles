[[ "$HOST" = ShagBox ]] && newout # Create initial out directory
[[ -z "$SSH_CLIENT" && "$TTY" = "/dev/tty1" && "$HOST" != ShagBox ]] && exec startx
