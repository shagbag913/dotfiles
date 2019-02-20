#!/usr/bin/env python3
import psutil
import sys
import subprocess

def secs2hours(secs):
    mm, ss = divmod(secs, 60)
    hh, mm = divmod(mm, 60)
    if hh != 0:
        return ("%dhr, %dmin" % (hh, mm))
    else:
        return ("%dmin" % mm)

def sendnotif(msg, icon):
    subprocess.call(['notify-send.sh', '--icon=' + icon, '--app-name=Battery', msg])

battery = psutil.sensors_battery()
battery_pct = round(battery.percent)

# battery low warning
try:
    if sys.argv[1] == 'low':
        icon = '/usr/share/icons/Numix/48/status/gtk-dialog-warning-panel.svg'
        sendnotif('Warning: battery low.\nCharge: {}'.format(battery_pct), icon)
        quit()
except IndexError:
    None # do nothing, no argument was specified

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

sendnotif("{}% charged\n{}".format(battery_pct, remain), icon)
