// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"flag"
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"
	"strings"

	"go.skia.org/infra/go/common"
	"go.skia.org/skia/bazel/exporter"
	"go.skia.org/skia/bazel/exporter/interfaces"
)

func main() {
	var (
		myFlags       = common.NewMultiStringFlag("rule", nil, "Bazel rule (may be repeated).")
		outFormat     = flag.String("output_format", "", "Desired output format. One of cmake or gni.")
		cmakeFileName = flag.String("out", "CMakeLists.txt", "CMake output file")
		projName      = flag.String("proj_name", "", "CMake project name")
		checkCurrent  = flag.Bool("check_current", false, "Identify any out-of-date target files")
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
	if *checkCurrent && *outFormat != "gni" {
		fmt.Fprintf(os.Stderr, "-check_current is only supported for gni output")
		flag.PrintDefaults()
		os.Exit(1)
	}
	qr := exporter.NewBazelQueryCommand(*myFlags, workspaceDir)
	openFileFn := func(path string) (interfaces.Writer, error) {
		if *outFormat == "gni" {
			// TODO: Remove this debug code - only for development.
			if strings.HasPrefix(path, workspaceDir) {
				s := strings.TrimPrefix(path, workspaceDir)
				path = filepath.Join(workspaceDir, "gn_out", s)
				d := filepath.Dir(path)
				err = os.MkdirAll(d, 0777)
				if err != nil {
					fmt.Fprintf(os.Stderr, "Unable to make dir %q: %v\n", d, err)
				}
			}
		}
		f, err := os.Create(path)
		if err != nil {
			defer f.Close()
		}
		return f, err
	}
	readFileFunc := func(filename string) ([]byte, error) {
		return ioutil.ReadFile(filename)
	}
	var exp interfaces.Exporter = nil
	if *outFormat == "cmake" {
		exp = exporter.NewCMakeExporter(*projName, workspaceDir, *cmakeFileName, openFileFn)
	} else {
		exp = exporter.NewGNIExporter(workspaceDir, openFileFn, readFileFunc)
	}
	if *checkCurrent {
		var numOutOfDate int
		numOutOfDate, err = exp.CheckCurrent(qr)
		if err != nil {
			fmt.Fprintf(os.Stderr, "Error running %s currency check: %v\n", *outFormat, err)
		}
		if numOutOfDate > 0 {
			fmt.Fprintf(os.Stdout, "%d files are out of date\n", numOutOfDate)
			os.Exit(numOutOfDate)
		}
	} else {
		err = exp.Export(qr)
		if err != nil {
			fmt.Fprintf(os.Stderr, "Error exporting to %s: %v\n", *outFormat, err)
		}
	}
	if err != nil {
		os.Exit(4)
	}
}
