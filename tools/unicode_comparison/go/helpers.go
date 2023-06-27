// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
package main

import (
	  "os"
    "path/filepath"
    "strings"
)

func check(e error) {
    if e != nil {
        panic(e)
    }
}

func min(a, b int) int {
    if a < b {
        return a
    }
    return b
}

func expand_path(path *string) {
    if strings.HasPrefix(*path, "~/") {
        home, err := os.UserHomeDir()
        check(err)
        *path = filepath.Join(home, (*path)[2:])
    }
}

func write_text_file(fullFileName string, text string) {
    f, err := os.Create(fullFileName)
    check(err)
    _, err = f.WriteString(text)
    check(err)
    defer f.Close()
}