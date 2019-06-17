if [[ $(hostname) == ShagBook ]]; then
    while :; do if xrandr --output eDP1 --mode 1600x900; then break; fi; done &
    for mf in 'Middle Emulation' 'Natural Scrolling' 'Tapping'; do
        xinput set-prop "Elan Touchpad" "libinput $mf Enabled" 1 &
    done &
fi
redshift l 40:-86 &
exec bspwm
