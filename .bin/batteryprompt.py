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
    subprocess.call(['notify-send.sh', '--icon=' + icon, '--app-name=BatteryPrompt', msg])

def lowprompt(battery_pct):
    icon = '/usr/share/icons/Numix/48/status/gtk-dialog-warning-panel.svg'
    sendnotif('Warning: battery low.\nCharge: {}%'.format(battery_pct), icon)

def eventprompt(battery_pct, battery_state):
    if battery_state == True:
        event = 'Charger connected'
    else:
        event = 'Charger disconnected'
    sendnotif(event, geticon(battery_pct, battery_state))

def geticon(battery_pct, battery_state):
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
        icon += '-charging'
    icon += '.svg'
    return icon

def statprompt(battery_pct, battery_state, battery_time):
    if battery_state == True:
        remain = 'Charging'
    else:
        remain = "{} remaining".format(secs2hours(battery_time))

    sendnotif("{}% charged\n{}".format(battery_pct, remain), geticon(battery_pct, battery_state))
