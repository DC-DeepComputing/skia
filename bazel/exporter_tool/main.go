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

	"go.skia.org/infra/go/common"
	"go.skia.org/skia/bazel/exporter"
	intf "go.skia.org/skia/bazel/exporter/interfaces"
)

func main() {
	var (
		myFlags       = common.NewMultiStringFlag("rule", nil, "Bazel rule (may be repeated).")
		outFormat     = flag.String("output_format", "", "Desired output format. One of cmake or gni.")
		cmakeFileName = flag.String("out", "CMakeLists.txt", "CMake output file")
		projName      = flag.String("proj_name", "", "CMake project name")
	)
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
		fmt.Fprintf(os.Stderr, "Error getting pwd: %v", err)
		os.Exit(2)
	}
	qr := exporter.NewBazelQueryCommand(*myFlags, workspaceDir)
	fileOpener := func(path string) (io.Writer, error) {
		fullPath := filepath.Join(workspaceDir, path)
		f, err := os.Create(fullPath)
		if err != nil {
			defer f.Close()
		}
		return f, err
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
				fmt.Printf("### >>>>> %s\n", path)
				os.Stdout.Write(buff.Bytes())
				fmt.Printf("### <<<<< %s\n", path)
				fmt.Println()
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
