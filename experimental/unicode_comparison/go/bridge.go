package main

// #cgo LDFLAGS: -L. -lbridge
// #include <stdio.h>
// #include <stdlib.h>
// #include "../bridge/bridge.h"
import "C"

import (
  "fmt"
	"unsafe"
)

/*
func C.CString(goString string) *C.char
func C.GoString(cString *C.char) string
func C.GoStringN(cString *C.char, length C.int) string
*/

type CodeUnitFlags int
const (
    kNoCodeUnitFlag CodeUnitFlags = 0x00
    kPartOfWhiteSpaceBreak = 0x01
    kGraphemeStart = 0x02
    kSoftLineBreakBefore = 0x04
    kHardLineBreakBefore = 0x08
    kPartOfIntraWordBreak = 0x10
    kControl = 0x20
    kTabulation = 0x40
    kGlyphClusterStart = 0x80
)

type SkString struct {
	  ptr unsafe.Pointer
}

type IntPtr struct {
	  ptr unsafe.Pointer
}

func Initialize(text string) int {
    fmt.Println(text);
    return int(C.initialize(C.CString(text)))
}

func GetFlags(index int, flags *int) bool {
    return bool(C.getFlags(C.int(index), unsafe.Pointer(flags)))
}

func FlagsToString(flags int) string {
    result := ""
    if (flags & kGraphemeStart) != 0 {
        result += "G"
    }
    if (flags & kSoftLineBreakBefore) != 0 {
        result += "S"
    }
    if (flags & kHardLineBreakBefore) != 0 {
        result += "H"
    }
    if (flags & kPartOfWhiteSpaceBreak) != 0 {
        result += "W"
    }
    if (flags & kControl) != 0 {
        result += "C"
    }
    return result
}

func Upper(str string) SkString {
	  return SkString {
	      ptr: C.upper(C.CString(str)),
    }
}

