// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

// #cgo LDFLAGS: -L. -lbridge_icu
// #include <stdio.h>
// #include <stdlib.h>
// #include "../cpp/bridge.h"
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
	"strconv"
	"strings"
	//"unsafe"
)

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

func writeInfo(inputPath string, outputPath string) error {

	// Read file
	fileContent, err := ioutil.ReadFile(inputPath)
	check(err)

	// Convert []byte to string
	text := string(fileContent)

	// Initialize file
	//fmt.Printf("File: %s\n", inputPath)
	time := Initialize(text)

	outputFile, err := os.Create(outputPath)
	check(err)

	// Collect info
	line := fmt.Sprintf("%.2f", time)
	_, err = outputFile.WriteString(line + "\n0.0\n")
	check(err)

	graphemes := ""
	softBreaks := ""
	hardBreaks := ""
	whitespaces := ""
	words := ""
	controls := ""

	graphemesCount := 0
	softBreaksCount := 0
	hardBreaksCount := 0
	whitespacesCount := 0
	wordsCount := 0
	controlsCount := 0
	for i := 0; i <= len(fileContent); i++ {
		var flags int = 0
		GetFlags(i, &flags)
		str := FlagsToString(flags)
		pos := strconv.Itoa(i)
		if strings.Contains(str, "G") {
			graphemes += " "
			graphemes += pos
			graphemesCount += 1
		}
		if strings.Contains(str, "S") {
			softBreaks += " "
			softBreaks += pos
			softBreaksCount += 1
		}
		if strings.Contains(str, "H") {
			hardBreaks += " "
			hardBreaks += pos
			hardBreaksCount += 1
		}
		if strings.Contains(str, "W") {
			whitespaces += " "
			whitespaces += pos
			whitespacesCount += 1
		}
		if strings.Contains(str, "D") {
			words += " "
			words += pos
			wordsCount += 1
		}
		if strings.Contains(str, "C") {
			controls += " "
			controls += pos
			controlsCount += 1
		}
	}

	_, err = outputFile.WriteString(strconv.Itoa(graphemesCount) + graphemes + "\n")
	check(err)

	_, err = outputFile.WriteString(strconv.Itoa(softBreaksCount) + softBreaks + "\n")
	check(err)

	_, err = outputFile.WriteString(strconv.Itoa(hardBreaksCount) + hardBreaks + "\n")
	check(err)

	_, err = outputFile.WriteString(strconv.Itoa(whitespacesCount) + whitespaces + "\n")
	check(err)

	_, err = outputFile.WriteString(strconv.Itoa(wordsCount) + words + "\n")
	check(err)

	_, err = outputFile.WriteString(strconv.Itoa(controlsCount) + controls + "\n")
	check(err)

	outputFile.Close()

	return nil
}

func main() {
	var (
		root = flag.String("root", "~/datasets", "Folder (pages will be under <Folder>/input")
		impl = flag.String("impl", "", "Unicode Implementation")
	)
	flag.Parse()

	expand(root)
	input := *root + "/input"
	output := *root + "/output/" + *impl
	if *root == "" {
		fmt.Println("Must set --root")
		flag.PrintDefaults()
	} else if *impl == "" {
		fmt.Println("Must set --impl")
		flag.PrintDefaults()
	}

	if !C.make_unicode() {
		return
	}
	err := walkRecursively(input, output)
	defer C.destroy_unicode()

	if err != nil {
		fmt.Println(err)
	}
}
