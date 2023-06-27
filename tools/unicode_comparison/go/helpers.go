// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
package main

import (
    "os"
    "path/filepath"
    "strconv"
    "strings"
)

func check(e error) {
    if e != nil {
        panic(e)
    }
}

func Abs(x int) int {
    if x < 0 {
        return -x
    }
    return x
}

func min(a, b int) int {
    if a < b {
        return a
    }
    return b
}

func minIndex(a []int) int {
    if len(a) == 0 {
        return -1
    }
    result := 0
    for i, r := range a {
        if (a[result] > r) {
            result = i
        }
    }
    return result
}

func expand(path *string) {
    if strings.HasPrefix(*path, "~/") {
        home, err := os.UserHomeDir()
        check(err)
        *path = filepath.Join(home, (*path)[2:])
    }
}

func split(str string, sep string) []int {
    arr := strings.Split(str, sep)
    var result = []int{}
    for _, i := range arr {
        j, err := strconv.Atoi(i)
        if err != nil {
            return []int{}
        }
        result = append(result, j)
    }
    return result
}

func insert(a []int, index int, value int) []int {
    if len(a) == index { // nil or empty slice or after last element
        return append(a, value)
    }
    a = append(a[:index+1], a[index:]...) // index < len(a)
    a[index] = value
    return a
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