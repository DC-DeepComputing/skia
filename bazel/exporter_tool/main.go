// Copyright 2022 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"fmt"
	"os"

	"go.skia.org/bazel/exporter"
)

func main() {
	const test_file string = "skia_dump.pb"
	qr, err := exporter.NewTestQueryReader(test_file)
	if err != nil {
		fmt.Printf("Error creating query reader for protobuf file %s: %v\n", test_file, err)
		os.Exit(1)
	}
	exporter := exporter.NewCMakeExporter(qr)
	err = exporter.Export()
	if err != nil {
		fmt.Printf("Error exporting to CMake: %v\n", err)
		os.Exit(2)
	}
	fmt.Println("Done exporting.")
}
