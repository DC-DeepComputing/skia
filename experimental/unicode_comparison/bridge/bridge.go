package main

// #cgo LDFLAGS: -L. -lbridge
// #include <stdio.h>
// #include <stdlib.h>
// #include "bridge.h"
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

type Unicode struct {
	  ptr unsafe.Pointer
}

type SkString struct {
	  ptr unsafe.Pointer
}

func NewUnicode() Unicode {
	  return Unicode {
		    ptr: C.make_unicode(),
	  }
}

func (unicode Unicode) Destroy() {
    C.destroy_unicode(unicode.ptr)
}

func (str SkString) Destroy() {
    C.destroy_str(str.ptr)
}

func (unicode Unicode) Upper(str string) SkString {
    fmt.Println(str);
	  return SkString {
	      ptr: C.upper(C.CString(str)),
    }
}

func main() {
	unicode := NewUnicode()

  var upp SkString = unicode.Upper("Hello world!")
  C.print(upp.ptr)

  defer upp.Destroy()
	defer unicode.Destroy()
}
