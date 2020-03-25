package main

import (
    "fmt"
    "time"
    "strconv"
    "io/ioutil"
    "os"
    "os/exec"
    "regexp"
)

// CONFIGURATION FLAGS

/* Whether to use 12 or 24 hour time */
var use12HourTime bool = true

/* Enable charging animation */
var animateChargeGlyphWhenCharging = true

// END CONFIGURATION FLAGS

/* Printed bar strings */
var timeString string
var chargeString string
var bspwmStatus string
var netStatus string

/* Other */
var lastBspwmStatus string

func main() {
    /* Initialize goroutines */
    go setTimeString()
    go setChargeString()
    go setBspwmStatus()
    go setNetStatus()

    /* Block main thread and let goroutines do everything */
    select { }
}

func printBuffer() {
    printBuffer := ""
    rightBuffer := "%{r}"

    if bspwmStatus != "" {
        printBuffer += "%{l}" + bspwmStatus
    }

    if timeString != "" {
        printBuffer += "%{c}" + timeString
    }

    if netStatus != "" {
        rightBuffer += netStatus + "   |   "
    }

    if chargeString != "" {
        rightBuffer += chargeString + "   |   "
    }

    if rightBuffer != "%{r}" {
        printBuffer += rightBuffer[:len(rightBuffer)-4]
    }

    if printBuffer != "" {
        printBuffer = printBuffer
        fmt.Println(printBuffer)
    }
}

func setTimeString() {
    for {
        tNow := time.Now()
        minute, hour := tNow.Minute(), tNow.Hour()

        /* 12 hour time */
        if use12HourTime == true {
            if hour > 12 {
                hour -= 12
            } else if hour == 0 {
                hour = 12
            }
        }

        newTimeString := strconv.Itoa(hour) + ":"
        if minute < 10 {
            newTimeString += "0"
        }
        newTimeString += strconv.Itoa(minute)

        if newTimeString != timeString {
            timeString = newTimeString
            printBuffer()
        }

        time.Sleep(2 * time.Second)
    }
}

func getBatteryPercentGlyphIndex(batteryPercentage int, overrideIndex int) int {
    if overrideIndex >= 0 {
        return overrideIndex
    }

    if batteryPercentage >= 90 {
        return 4
    } else if batteryPercentage >= 75 {
        return 3
    } else if batteryPercentage >= 50 {
        return 2
    } else if batteryPercentage >= 25 {
        return 1
    } else {
        return 0
    }
}

func getBatteryPercentWithGlyph(batteryPercentage int, overrideIndex int) string {
    batteryGlyphs := []string{"", "", "", "", ""}
    glyphIndex := getBatteryPercentGlyphIndex(batteryPercentage, overrideIndex)

    return batteryGlyphs[glyphIndex] + " " + strconv.Itoa(batteryPercentage) + "%"
}

func isCharging() bool {
    status, err := ioutil.ReadFile("/sys/class/power_supply/BAT0/status")
    if err != nil {
        return false
    }

    if string(status[:len(status) - 1]) == "Discharging" {
        return false
    }

    return true
}

func setChargeString() {
    chargingIndexCounter := -1
    for {
        /* TODO: unhardcode */
        charge, err := ioutil.ReadFile("/sys/class/power_supply/BAT0/capacity")
        if err != nil {
            break
        }
        chargeInt, _ := strconv.Atoi(string(charge[:len(charge)-1]))

        animate := isCharging() && animateChargeGlyphWhenCharging
        if animate == false {
            /* Reset index counter */
            chargingIndexCounter = -1
        } else {
            if chargingIndexCounter == 4 || chargingIndexCounter < 0 {
                chargingIndexCounter = getBatteryPercentGlyphIndex(chargeInt, -1)
            } else {
                chargingIndexCounter++
            }
        }

        newChargeString := getBatteryPercentWithGlyph(chargeInt, chargingIndexCounter)

        if newChargeString != chargeString {
            chargeString = newChargeString
            printBuffer()
        }

        time.Sleep(2 * time.Second)
    }
}

func setBspwmStatus() {
    reg := regexp.MustCompile("[^oOfF]*")

    for {
        /* TODO: talk directly to socket */
        socketStatus, _ := exec.Command("bspc", "wm", "--get-status").Output()
        socketStatusFormatted := reg.ReplaceAllString(string(socketStatus), "")

        /* Don't continue if socketStatus hasn't changed */
        if socketStatusFormatted == lastBspwmStatus {
            time.Sleep(150 * time.Millisecond)
            continue
        }
        lastBspwmStatus = socketStatusFormatted

        newBspwmStatus := ""

        for i := 0; i < len(socketStatusFormatted); i++ {
            switch socketStatusFormatted[i] {
            case 'F':
                fallthrough
            case 'O':
                newBspwmStatus += " %{+u}  " + strconv.Itoa(i+1) + "  %{-u} |"
            case 'o':
                newBspwmStatus += "   " + strconv.Itoa(i+1) + "   |"
            }
        }

        /* Remove ending spacer */
        newBspwmStatus = newBspwmStatus[:len(newBspwmStatus)-1]

        bspwmStatus = newBspwmStatus
        printBuffer()
        time.Sleep(150 * time.Millisecond)
    }
}

func setNetStatus() {
    var newNetStatus string
    for {
        baseDir := "/sys/class/net/"
        netDirs, err := ioutil.ReadDir(baseDir)
        if err != nil {
            fmt.Println(err.Error())
            break
        }

        newNetStatus = ""

        for _, netDir := range netDirs {
            if netDir.Name() == "lo" {
                continue
            }

            file, err := os.Open(baseDir + netDir.Name() + "/operstate")
            if err != nil {
                fmt.Println(err.Error())
                continue
            }

            state := make([]byte, 4)

            count, err := file.Read(state)
            if err != nil {
                fmt.Println(err.Error())
                continue
            }

            if count == 3 {
                if netDir.Name()[:2] == "wl" {
                    newNetStatus += "   "
                } else {
                    newNetStatus += "   "
                }
            }

            file.Close()
        }

        if newNetStatus != netStatus {
            netStatus = newNetStatus[:len(newNetStatus)-3]
            printBuffer()
        }
        time.Sleep(time.Second * 5)
    }
}
