if [[ $(hostname) == ShagBook ]]; then
    until xrandr --output eDP1 --mode 1600x900; do
        xrandr --output eDP1 --mode 1600x900
    done &
    for mf in 'Middle Emulation' 'Natural Scrolling' 'Tapping'; do
        xinput set-prop "Elan Touchpad" "libinput $mf Enabled" 1 &
    done &
fi
redshift l 40:-86 &
exec bspwm
