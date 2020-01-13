package main

import (
    "fmt"
    "time"
    "strconv"
    "io/ioutil"
    "os/exec"
    "regexp"
)

/* Printed bar strings */
var timeString string
var chargeString string
var bspwmStatus string

/* Other */
var lastBspwmStatus string

func main() {
    /* Initialize goroutines */
    go setTimeString()
    go setChargeString()
    go setBspwmStatus()

    select {}
}

func printBuffer() {
    printBuffer := ""

    if bspwmStatus != "" {
        printBuffer += "%{l}" + bspwmStatus
    }

    if timeString != "" {
        printBuffer += "%{c}" + timeString
    }

    if chargeString != "" {
        printBuffer += "%{r}" + chargeString
    }

    if printBuffer != "" {
        printBuffer = "    " + printBuffer + "    "
        fmt.Println(printBuffer)
    }
}

func setTimeString() {
    for {
        tNow := time.Now()
        minute, hour := tNow.Minute(), tNow.Hour()

        if hour > 12 {
            hour -= 12
        }

        newTimeString := strconv.Itoa(hour) + ":"

        if minute < 10 {
            newTimeString += "0" + strconv.Itoa(minute)
        } else {
            newTimeString += strconv.Itoa(minute)
        }

        if newTimeString != timeString {
            timeString = newTimeString
            printBuffer()
        }

        time.Sleep(10 * time.Second)
    }
}

func setChargeString() {
    for {
        /* TODO: unhardcode */
        charge, err := ioutil.ReadFile("/sys/class/power_supply/BAT0/capacity")

        if err != nil {
            /* DNE, break */
            break
        }

        newChargeString := string(charge[:len(charge) - 1]) + "%"

        if newChargeString != chargeString {
            chargeString = newChargeString
            printBuffer()
        }

        time.Sleep(10 * time.Second)
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
            time.Sleep(200 * time.Millisecond)
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
        time.Sleep(200 * time.Millisecond)
    }
}
