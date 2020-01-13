package main

import (
    "fmt"
    "time"
    "strconv"
    "io/ioutil"
    "os/exec"
    "strings"
)

/* Printed bar strings */
var timeString string
var chargeString string
var bspwmStatus string

/* Other */
var lastBspwmStatus []byte

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

func testEqual(a, b []byte) bool {
    if (a == nil) != (b == nil) {
        return false
    }

    if len(a) != len(b) {
        return false
    }

    for i := range a {
        if a[i] != b[i] {
            return false
        }
    }

    return true
}

func setBspwmStatus() {
    for {
        /* TODO: talk directly to socket */
        socketStatus, _ := exec.Command("bspc", "wm", "--get-status").Output()

        /* Don't continue if socketStatus hasn't changed */
        if testEqual(socketStatus, lastBspwmStatus) == true {
            time.Sleep(100 * time.Millisecond)
            continue
        }
        lastBspwmStatus = socketStatus

        socketStatusSplit := strings.Split(string(socketStatus), ":")
        newBspwmStatus := ""

        for i := 0; i < len(socketStatusSplit); i++ {
            socketStatusEl := string(socketStatusSplit[i])
            switch socketStatusEl[:1] {
            case "F":
                fallthrough
            case "O":
                newBspwmStatus += " %{+u}  " + strconv.Itoa(i) + "  %{-u} |"
            case "o":
                newBspwmStatus += "   " + strconv.Itoa(i) + "   |"
            }
        }

        /* Remove ending spacer */
        newBspwmStatus = newBspwmStatus[:len(newBspwmStatus)-1]

        bspwmStatus = newBspwmStatus
        printBuffer()
        time.Sleep(100 * time.Millisecond)
    }
}
