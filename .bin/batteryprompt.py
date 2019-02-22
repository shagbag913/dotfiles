#!/usr/bin/env python3
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

def lowprompt(battery_pct):
    icon = '/usr/share/icons/Numix/48/status/gtk-dialog-warning-panel.svg'
    sendnotif('Warning: battery low.\nCharge: {}%'.format(battery_pct), icon)

def statprompt(battery_pct, battery_state, battery_time):
    icon = '/usr/share/icons/Numix/48/devices/'

    if battery_pct < 20:
        icon += 'battery-caution'
    elif battery_pct < 40:
        icon += 'battery-low'
    elif battery_pct < 80:
        icon += 'battery-good'
    else:
        icon += 'battery-full'

    if battery_state == True:
        remain = 'Charging'
        icon += '-charging'
    else:
        remain = "{} remaining".format(secs2hours(battery_time))
    icon += '.svg'

    sendnotif("{}% charged\n{}".format(battery_pct, remain), icon)
