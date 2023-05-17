// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

// #cgo LDFLAGS: -L. -lbridge
// #include <stdio.h>
// #include <stdlib.h>
// #include "../bridge/bridge.h"
import "C"

import (
	"flag"
	"fmt"
  "io/ioutil"
	//"github.com/trietmn/go-wiki"
	//"golang.org/x/net/html"
	//"html/template"
	"os"
  "path/filepath"
	"strings"
	"strconv"
	//"unsafe"
)

func check(e error) {
    if e != nil {
        panic(e)
    }
}


func createEmptyFile(name string) error {
    d := []byte("")
    return os.WriteFile(name, d, 0644)
}

func walkRecursively(input string, output string) error {
    err := filepath.Walk(input,
    func(inputPath string, info os.FileInfo, err error) error {
        if err != nil {
            fmt.Println(err)
            return err
        }
        if info.IsDir() {
            outputPath := strings.Replace(inputPath, input, output, 1)
            fmt.Printf("%s -> %s\n", inputPath, outputPath)
            err := os.MkdirAll(outputPath, os.ModePerm)
            check(err)
        } else {
            outputPath := strings.Replace(inputPath, input, output, 1)
            //fmt.Printf("     %s -> %s\n", inputPath, outputPath)
            writeInfo(inputPath, outputPath)
        }
        return nil
    })
    return err
}

func writeInfo(inputPath string, outputPath string) int {

    // Read file
    fileContent, err := ioutil.ReadFile(inputPath)
    check(err)

    // Convert []byte to string
    text := string(fileContent)

    // Initialize file
    count := Initialize(text)

    // Collect info
    line := ""
    for i := 1; i < count; i++ {
        var flags int = 0
        GetFlags(i, &flags)
        line += strconv.Itoa(i)
        line += ":"
        line += FlagsToString(flags)
        line += " "
    }
    if len(line) > 0 {
        strings.TrimSuffix(line, " ")
        line += "\n"
    } else {
        return -1
    }

    fmt.Printf("%d: %s\n", count, text)

    // Write info
    outputFile, err := os.Create(outputPath)
    check(err)
    bytes, err := outputFile.WriteString(line)
    check(err)
    outputFile.Close()
    return bytes
}

func main() {
    var (
        root      = flag.String("root", "datasets", "Folder (pages will be under <Folder>/input")
        impl      = flag.String("impl", "", "ICU Implementation")
    )
    flag.Parse()
    input := *root + "/input"
    output := *root + "/" + *impl
    if *root == "" {
        fmt.Println("Must set --root")
        flag.PrintDefaults()
    } else if *impl == "" {
        fmt.Println("Must set --impl")
        flag.PrintDefaults()
    }
    C.make_unicode()
    err := walkRecursively(input, output)
	  defer C.destroy_unicode()

    if err != nil {
        fmt.Println(err)
    }
}

