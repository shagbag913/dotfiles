#!/usr/bin/env python3
import psutil
import os

def secs2hours(secs):
    mm, ss = divmod(secs, 60)
    hh, mm = divmod(mm, 60)
    return ("%dhr, %dmin" % (hh, mm))

battery = psutil.sensors_battery()
battery_state = battery.power_plugged
battery_time = battery.secsleft
battery_pct = battery.percent

if battery_state == True:
    if battery_pct < 20:
        icon = 'battery-caution-charging'
    elif battery_pct < 40:
        icon = 'battery-low-charging'
    elif battery_pct < 80:
        icon = 'battery-good-charging'
    else:
        icon = 'battery-full-charging'

    remain = "Charging"
else:
    if battery_pct < 20:
        icon = 'battery-caution'
    elif battery_pct < 40:
        icon = 'battery-low'
    elif battery_pct < 80:
        icon = 'battery-good'
    else:
        icon = 'battery-full'

    remain = "%s remaining" % (secs2hours(battery_time))


icon = '/usr/share/icons/Numix/48/devices/%s.svg' % (icon)
os.system('notify-send.sh --app-name=Battery --icon=%s "%i%% charged\n%s"' % (icon, round(battery_pct), remain))
