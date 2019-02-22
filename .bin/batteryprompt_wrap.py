#!/usr/bin/env python3
import batteryprompt
import os
import psutil
import sys

showlow = 'no u'
plugged = 'no u'

while True:
    battery = psutil.sensors_battery()
    battery_pct = round(battery.percent)
    battery_state = battery.power_plugged
    battery_time = battery.secsleft

    if plugged == 'no u':
        plugged = battery_state

    if showlow == 'no u':
        showlow = True

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
