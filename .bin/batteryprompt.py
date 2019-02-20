#!/usr/bin/env python3
import psutil
import os

def secs2hours(secs):
    mm, ss = divmod(secs, 60)
    hh, mm = divmod(mm, 60)
    if hh != 0:
        return ("%dhr, %dmin" % (hh, mm))
    else:
        return ("%dmin" % mm)

battery = psutil.sensors_battery()
battery_pct = round(battery.percent)
icon = '/usr/share/icons/Numix/48/devices/'

if battery_pct < 20:
    icon += 'battery-caution'
elif battery_pct < 40:
    icon += 'battery-low'
elif battery_pct < 80:
    icon += 'battery-good'
else:
    icon += 'battery-full'

battery_state = battery.power_plugged
battery_time = battery.secsleft

if battery_state == True:
    remain = 'Charging'
    icon += '-charging'
else:
    remain = "{} remaining".format(secs2hours(battery_time))

os.system('notify-send.sh --app-name=Battery --icon={}.svg "{}% charged\n{}"'.format(icon, battery_pct, remain))
