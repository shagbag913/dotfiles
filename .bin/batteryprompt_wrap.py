#!/usr/bin/env python3
import batteryprompt
import os
import psutil
import sys

battery = psutil.sensors_battery()
plugged = battery.power_plugged

while True:
    battery = psutil.sensors_battery()
    battery_pct = round(battery.percent)
    battery_state = battery.power_plugged
    battery_time = battery.secsleft

    if plugged != battery_state:
        plugged = battery_state
        batteryprompt.eventprompt(battery_pct, battery_state)

    try:
        if sys.argv[1] == 'click':
            batteryprompt.statprompt(battery_pct, battery_state, battery_time)
            quit()
    except IndexError:
        None # no argument

    if battery_pct <= 15 and battery_state != True and not os.path.exists('/tmp/batterylock'):
        batteryprompt.lowprompt(battery_pct)
        open('/tmp/batterylock', 'a').close()
    elif (battery_pct > 16 or battery_state == True) and os.path.exists('/tmp/batterylock'):
        os.remove('/tmp/batterylock')
