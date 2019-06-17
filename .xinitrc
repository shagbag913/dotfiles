if [[ $(hostname) == ShagBook ]]; then
    until xrandr --output eDP1 --mode 1600x900; do
        xrandr --output eDP1 --mode 1600x900
    done &
fi
redshift l 40:-86 &
exec bspwm
