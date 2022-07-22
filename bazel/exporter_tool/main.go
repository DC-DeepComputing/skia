// Copyright 2022 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"bufio"
	"fmt"
	"os"
	"path"

	"go.skia.org/skia/bazel/exporter"
)

func main() {
	if len(os.Args) != 3 {
		fmt.Fprintf(os.Stderr, "Usage: %s: <proto input file> <cmake output file>\n", os.Args[0])
		os.Exit(1)
	}
	testFile := os.Args[1]
	cmakeFileName := os.Args[2]
	qr, err := exporter.NewTestQueryReader(testFile)
	if err != nil {
		fmt.Printf("Error creating query reader for protobuf file %s: %v\n", testFile, err)
		os.Exit(1)
	}
	workspaceDir := path.Dir(testFile)
	exporter := exporter.NewCMakeExporter(qr, workspaceDir)
	cmakeFile, err := os.OpenFile(cmakeFileName, os.O_CREATE|os.O_WRONLY, 0644)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error opening %s: %v", cmakeFileName, err)
		os.Exit(2)
	}
	w := bufio.NewWriter(cmakeFile)
	err = exporter.Export(w)
	if err != nil {
		fmt.Printf("Error exporting to CMake: %v\n", err)
		os.Exit(2)
	}
	w.Flush()
}
