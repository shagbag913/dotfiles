# setup trackpad and screen resolution
if [[ $HOSTNAME = ShagBook ]]; then
    until xrandr --output eDP1 --mode 1600x900; do !!; done &
    for mf in 'Middle Emulation' 'Natural Scrolling' 'Tapping'; do
        xinput set-prop "Elan Touchpad" "libinput $mf Enabled" 1 &
    done &
    python3 /home/$USER/.bin/batteryprompt.py &
else
    ngrok tcp 22 &
fi
dunst &
redshift l 40:-86 &
exec i3
