// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This program is the brains behind the adb_test Bazel rule, which allows running a test on an
// Android device via adb.

package main

import (
	"bytes"
	"flag"
	"fmt"
	"os"
	"os/exec"
	"strings"

	"go.skia.org/infra/go/skerr"
)

type Device string

const (
	PIXEL_5 = Device("pixel_5")
	PIXEL_7 = Device("pixel_7")
)

var AllDevices = []Device{
	// TODO(lovisolo): Add more devices.
	PIXEL_5,
	PIXEL_7,
}

func main() {
	deviceFlag := flag.String("device", "", `Device under test, e.g. "pixel_5".`)
	archiveFlag := flag.String("archive", "", "Tarball with the payload to upload to the device under test.")
	testRunnerFlag := flag.String("test-runner", "", "Path to the test runner inside the tarball.")
	flag.Parse()

	if *deviceFlag == "" {
		die("Flag --device is required.\n")
	}
	if *archiveFlag == "" {
		die("Flag --archive is required.\n")
	}
	if *testRunnerFlag == "" {
		die("Flag --test-runner is required.\n")
	}

	var device Device
	for _, d := range AllDevices {
		if *deviceFlag == string(d) {
			device = d
		}
	}
	if device == "" {
		die("Unknown device: %q\n", *deviceFlag)
	}

	if err := runTest(device, *archiveFlag, *testRunnerFlag); err != nil {
		die("%s\n", err)
	}
}

// runTest runs the test on device via adb.
func runTest(device Device, archive, testRunner string) error {
	// TODO(lovisolo): Add any necessary device-specific setup steps such as turning cores on/off and
	//                 setting the CPU/GPU frequencies.

	// TODO(lovisolo): Should we check that the machine is attached to the expected device type?
	//                 E.g. run "adb devices -l" and check that the output contains
	//                 "model:Pixel_5".

	// Clean up the device before running the test. Previous tests might have left the device in a
	// dirty state.
	cleanUpDevice := func(device Device) error {
		return adb("shell", "su", "root", "rm", "-rf", getArchiveExtractionDirOnDevice(device), getArchiveExtractionDirOnDevice(device))
	}
	if err := cleanUpDevice(device); err != nil {
		return skerr.Wrapf(err, "while cleaning up the device before running the test")
	}

	// Also clean up device after running the test.
	defer func() {
		if err := cleanUpDevice(device); err != nil {
			die(skerr.Wrapf(err, "while cleaning up the device after running the test").Error() + "\n")
		}
	}()

	// Upload archive to device.
	if err := adb("push", archive, getArchivePathOnDevice(device)); err != nil {
		return skerr.Wrapf(err, "while pushing archive to device")
	}

	// Extract archive.
	if err := adb("shell", "su", "root", "mkdir", "-p", getArchiveExtractionDirOnDevice(device)); err != nil {
		return skerr.Wrapf(err, "Error while creating archive extraction directory on device")
	}
	if err := adb("shell", "su", "root", "tar", "xzvf", getArchivePathOnDevice(device), "-C", getArchiveExtractionDirOnDevice(device)); err != nil {
		return skerr.Wrapf(err, "Error while extracting archive on device")
	}

	// Run test.
	stdin := fmt.Sprintf("cd %s && %s", getArchiveExtractionDirOnDevice(device), testRunner)
	if err := adbWithStdin(stdin, "shell", "su", "root"); err != nil {
		return skerr.Wrapf(err, "Error while running the test")
	}

	return nil
}

// getArchivePathOnDevice returns the path in the device's file system where the archive should be
// uploaded.
func getArchivePathOnDevice(device Device) string {
	// The /sdcard/revenge_of_the_skiabot directory is writable for non-root users, but files in
	// this directory cannot be executed. For this reason, we extract the archive in a directory
	// under /data, which allows executing files but requires root privileges.
	//
	// This might change in the future based on the device type, whether or not it's rooted, etc.
	return "/sdcard/revenge_of_the_skiabot/bazel-adb-test.tar.gz"
}

// getArchiveExtractionDirOnDevice returns the directory in the device's file system where the
// archive should be extracted.
func getArchiveExtractionDirOnDevice(device Device) string {
	// This might change in the future based on the device type, whether or not it's rooted, etc.
	return "/data/bazel-adb-test"
}

// adb runs adb with the given arguments.
func adb(args ...string) error {
	fmt.Printf("Executing %q...\n", strings.Join(append([]string{"adb"}, args...), " "))
	cmd := exec.Command("adb", args...)
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	return cmd.Run()
}

// adbWithStdin runs adb with the given arguments, and pipes the given input via standard input.
func adbWithStdin(stdin string, args ...string) error {
	fmt.Printf("Executing: %q with standard input %q...\n", strings.Join(append([]string{"adb"}, args...), " "), stdin)
	cmd := exec.Command("adb", args...)
	cmd.Stdin = bytes.NewBufferString(stdin)
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	return cmd.Run()
}

func die(msg string, a ...interface{}) {
	fmt.Fprintf(os.Stderr, msg, a...)
	os.Exit(1)
}
