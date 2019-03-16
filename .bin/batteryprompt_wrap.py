#!/usr/bin/env python3
import batteryprompt
import os
import psutil
import sys

showlow = 'no u'
plugged = 'no u'
donechg = 'no u'

while True:
    battery = psutil.sensors_battery()
    battery_pct = round(battery.percent)
    battery_state = battery.power_plugged
    battery_time = battery.secsleft

    if plugged == 'no u': plugged = battery_state
    if showlow == 'no u': showlow = True
    if donechg == 'no u': donechg = True

    if plugged != battery_state:
        plugged = battery_state
        batteryprompt.eventprompt(battery_pct, battery_state)

    try:
        if sys.argv[1] == 'click':
            batteryprompt.statprompt(battery_pct, battery_state, battery_time)
            quit()
    except IndexError:
        None # no argument

    if battery_pct <= 15 and battery_state != True and showlow == True:
        batteryprompt.lowprompt(battery_pct)
        showlow = False
    elif (battery_pct > 16 or battery_state == True) and showlow == False:
        showlow = True

    if battery_pct > 95 and battery_state == True and donechg == True:
        batteryprompt.sendnotif("Battery fully charged", batteryprompt.geticon(battery_pct, battery_state))
        donechg = False
    elif battery_pct < 95 and donechg == False:
        donechg = True
