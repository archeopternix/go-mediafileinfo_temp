package main

import (
    "fmt"
    "avwrapper"
)

func main() {
    sp := avwrapper.NewSmartAVCodecParameters()
    if sp == nil {
        panic("failed to allocate codec parameters")
    }

    fmt.Printf("Codec: %s\n", sp.CodecName())
    fmt.Printf("Resolution: %dx%d\n", sp.Width(), sp.Height())
    fmt.Printf("Bitrate: %d\n", sp.BitRate())

    // Kein manuelles Free nötig
}
