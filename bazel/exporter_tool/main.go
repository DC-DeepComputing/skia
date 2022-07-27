// Copyright 2022 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"bufio"
	"flag"
	"fmt"
	"os"

	"go.skia.org/skia/bazel/exporter"
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
		cmakeFileName = flag.String("out", "CMakeLists.txt", "CMake output file")
		projName      = flag.String("proj_name", "", "CMake project name")
	)
	flag.Var(&myFlags, "rule", "Bazel rule (may be repeated).")
	flag.Parse()
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
	exporter := exporter.NewCMakeExporter(workspaceDir)
	cmakeFile, err := os.OpenFile(*cmakeFileName, os.O_CREATE|os.O_WRONLY, 0644)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error opening %s: %v", *cmakeFileName, err)
		os.Exit(2)
	}
	w := bufio.NewWriter(cmakeFile)
	err = exporter.Export(qr, w, *projName)
	if err != nil {
		fmt.Printf("Error exporting to CMake: %v\n", err)
		os.Exit(2)
	}
	w.Flush()
}
