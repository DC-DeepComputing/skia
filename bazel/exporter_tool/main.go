// Copyright 2022 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"bytes"
	"flag"
	"fmt"
	"io"
	"os"
	"path/filepath"

	"go.skia.org/skia/bazel/exporter"
	intf "go.skia.org/skia/bazel/exporter/interfaces"
)

type RuleNames []string

func (i *RuleNames) String() string {
	return fmt.Sprintf("%v", *i)
}

func (i *RuleNames) Set(value string) error {
	*i = append(*i, value)
	return nil
}

func main() {
	var (
		myFlags       RuleNames
		outFormat     = flag.String("output_format", "", "Desired output format. One of cmake or gni.")
		cmakeFileName = flag.String("cmake_file", "CMakeLists.txt", "CMake output file")
		projName      = flag.String("proj_name", "", "CMake project name")
	)
	flag.Var(&myFlags, "rule", "Bazel rule (may be repeated).")
	flag.Parse()
	if *outFormat != "cmake" && *outFormat != "gni" {
		if *outFormat == "" {
			fmt.Fprintln(os.Stderr, "Output format required")
		} else {
			fmt.Fprintf(os.Stderr, "Incorrect output format: \"%s\"\n", *outFormat)
		}
		fmt.Fprintf(os.Stderr, "Usage of %s:\n", os.Args[0])
		flag.PrintDefaults()
		os.Exit(1)
	}
	if *cmakeFileName == "" || *projName == "" {
		fmt.Fprintf(os.Stderr, "Usage of %s:\n", os.Args[0])
		flag.PrintDefaults()
		os.Exit(1)
	}
	workspaceDir, err := os.Getwd()
	if err != nil {
		os.Exit(2)
	}
	qr := exporter.NewBazelQueryCommand(myFlags, workspaceDir)
	fileOpener := func(path string) (io.Writer, error) {
		fullPath := filepath.Join(workspaceDir, path)
		return os.OpenFile(fullPath, os.O_CREATE|os.O_WRONLY, 0644)
	}
	var exp intf.Exporter = nil
	if *outFormat == "cmake" {
		exp = exporter.NewCMakeExporter(*projName, workspaceDir, *cmakeFileName, fileOpener)
	} else {
		// TODO: Use chared fileOpener.
		// Currently using the map and deferred print function for debugging.
		fileWriters := make(map[string]*bytes.Buffer)
		testOpener := func(path string) (io.Writer, error) {
			w := new(bytes.Buffer)
			fileWriters[path] = w
			return w, nil
		}
		printFiles := func() {
			for path, buff := range fileWriters {
				fmt.Printf("### %s\n", path)
				os.Stdout.Write(buff.Bytes())
			}
		}
		defer printFiles()

		exp = exporter.NewGNIExporter(workspaceDir, testOpener)
	}
	err = exp.Export(qr)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error exporting to %s: %v\n", *outFormat, err)
		os.Exit(4)
	}
}
