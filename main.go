package main

import (
	"fmt"
	"avwrapper"
)

func main() {
	p := avwrapper.NewSmartAVCodecParameters()
	if p == nil {
		panic("allocation failed")
	}

	// Zugriff auf Felder (nur lesen!)
	fmt.Println("Bitrate:", p.BitRate())
	fmt.Println("Resolution:", p.Width(), "x", p.Height())

	// Direkter Zugriff (unsafe, aber erlaubt):
	raw := p.Ptr()
	raw.bit_rate = 500000
	raw.width = 1280
	raw.height = 720

	fmt.Println("Updated Bitrate:", p.BitRate())
	fmt.Println("Updated Resolution:", p.Width(), "x", p.Height())
}
