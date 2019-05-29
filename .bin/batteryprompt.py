#!/usr/bin/env python3
import batteryprompt
import os
import psutil
import sys
import sys
import subprocess

def secs2hours(secs):
    ''' Converts seconds to hh, mm format '''
    mm, ss = divmod(secs, 60)
    hh, mm = divmod(mm, 60)
    if hh != 0:
        return ("%dhr, %dmin" % (hh, mm))
    else:
        return ("%dmin" % mm)

def sendnotif(msg, icon):
    ''' Send notification that lasts for 3 seconds and shows input icon '''
    subprocess.call(['notify-send.sh', '--expire-time=3000', '--icon=' + icon, '--app-name=BatteryPrompt', msg])

def eventprompt(battery_pct, battery_chging):
    ''' Show message for charger being {,dis}connected '''
    if battery_chging == True:
        event = 'Charger connected'
    else:
        event = 'Charger disconnected'
    sendnotif(event, geticon(battery_pct, battery_chging))

def geticon(battery_pct, battery_chging):
    ''' Find correct battery icon depending on input percent/charging state '''
    icon = '/usr/share/icons/Numix/48/devices/'
    if battery_pct < 20:
        icon += 'battery-caution'
    elif battery_pct < 40:
        icon += 'battery-low'
    elif battery_pct < 80:
        icon += 'battery-good'
    else:
        icon += 'battery-full'
    if battery_chging == True:
        icon += '-charging'
    icon += '.svg'
    return icon

def statprompt(battery_pct, battery_chging, battery_time):
    ''' Header that shows charging status and battery percentage '''
    if battery_chging == True:
        remain = 'Charging'
    else:
        remain = "{} remaining".format(secs2hours(battery_time))

    sendnotif("{}% charged\n{}".format(battery_pct, remain), geticon(battery_pct, battery_chging))

# Vars checked and set during events in the while loop
plugged = None
showlow = None
donechg = None

while True:
    battery = psutil.sensors_battery()
    battery_pct = round(battery.percent)
    battery_chging = battery.power_plugged
    battery_time = battery.secsleft

    if plugged is None: plugged = battery_chging
    if showlow is None: showlow = True
    if donechg is None: donechg = True

    if plugged != battery_chging:
        plugged = battery_chging
        eventprompt(battery_pct, battery_chging)

    try:
        if sys.argv[1] == 'click':
            statprompt(battery_pct, battery_chging, battery_time)
            quit()
    except IndexError:
        None # no argument

    if battery_pct <= 15 and battery_chging != True and showlow == True:
        sendnotif('Warning: battery low.\nCharge: {}%'.format(battery_pct),
                geticon(battery_pct, False))
        showlow = False
    elif (battery_pct > 16 or battery_chging == True) and showlow == False:
        showlow = True

    if battery_pct > 95 and battery_chging == True and donechg == True:
        sendnotif("Battery fully charged", geticon(battery_pct, True))
        donechg = False
    elif battery_pct < 95 and donechg == False:
        donechg = True
