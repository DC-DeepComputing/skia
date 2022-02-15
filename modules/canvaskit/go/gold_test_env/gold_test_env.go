// Copyright 2022 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"fmt"
	"io/ioutil"
	"net"
	"net/http"
	"os"
	"os/signal"
	"path"
	"strconv"
	"syscall"
)

const (
	envPortFileBaseName = "port"
)

func main() {
	envDir, envReadyFile := mustGetEnvironmentVariables()

	port, listener := mustGetUnusedNetworkPort()

	beginTestManagementLogic(listener)

	mustPrepareTestEnvironment(envDir, port)

	setupTerminationLogic()

	mustSignalTestsCanBegin(envReadyFile)

	select {} // Block until the termination handler calls os.Exit
}

// mustGetEnvironmentVariables returns two file paths: a directory that can be used to communicate
// between this binary and the test binaries, and the file that needs to be created when this
// binary has finished setting things up. It panics if it cannot read the values from the
// set environment variables.
func mustGetEnvironmentVariables() (string, string) {
	// Read in build paths to the ready and port files.
	envDir := os.Getenv("ENV_DIR")
	if envDir == "" {
		panic("required environment variable ENV_DIR is unset")
	}
	envReadyFile := os.Getenv("ENV_READY_FILE")
	if envReadyFile == "" {
		panic("required environment variable ENV_READY_FILE is unset")
	}
	return envDir, envReadyFile
}

// mustGetUnusedNetworkPort returns a network port chosen by the OS (and assumed to be previously
// unused) and a listener for that port. We choose a non-deterministic port instead of a fixed port
// because multiple tests may be running in parallel.
func mustGetUnusedNetworkPort() (int, net.Listener) {
	// Listen on an unused port chosen by the OS.
	listener, err := net.Listen("tcp", ":0")
	if err != nil {
		panic(err)
	}
	port := listener.Addr().(*net.TCPAddr).Port
	fmt.Printf("Listening on port %d.\n", port)
	return port, listener
}

// beginTestManagementLogic sets up the server endpoints which allow the JS gm() tests to exfiltrate
// their PNG images by means of a POST request. TODO(kjlubick) finish implementing this
func beginTestManagementLogic(listener net.Listener) {
	// Serve a plaintext echo endpoint.
	http.HandleFunc("/test", func(w http.ResponseWriter, r *http.Request) {
		if _, err := fmt.Fprintln(w, "Hello world"); err != nil {
			panic(err)
		}
		fmt.Printf("handler saw a %s request\n", r.Method)
	})
	go func() {
		serveForever(listener)
	}()
}

// serveForever serves the given listener and blocks. If it could not start serving, it will panic.
func serveForever(listener net.Listener) {
	// If http.Serve returns, it is an error.
	if err := http.Serve(listener, nil); err != nil {
		panic(fmt.Sprintf("Finished serving due to error: %s\n", err))
	}
}

// mustPrepareTestEnvironment writes any files to the temporary test directory. This is just a file
// that indicates which port the gold tests should make POST requests to. It panics if there are
// any errors.
func mustPrepareTestEnvironment(dirTestsCanRead string, port int) {
	envPortFile := path.Join(dirTestsCanRead, envPortFileBaseName)
	if err := ioutil.WriteFile(envPortFile, []byte(strconv.Itoa(port)), 0644); err != nil {
		panic(err)
	}
}

// setupTerminationLogic creates a handler for SIGTERM which is what test_on_env will send the
// environment when the tests complete. We use that handler to finalize the upload via goldctl.
func setupTerminationLogic() {
	c := make(chan os.Signal, 1)
	go func() {
		<-c
		fmt.Println("Got termination signal, should run goldctl finalize or something")
		os.Exit(0)
	}()
	signal.Notify(c, syscall.SIGTERM)
}

// mustSignalTestsCanBegin creates the agreed upon ENV_READY_FILE which signals the test binary can
// be executed by Bazel. See test_on_env.bzl for more. It panics if the file cannot be created.
func mustSignalTestsCanBegin(envReadyFile string) {
	if err := ioutil.WriteFile(envReadyFile, []byte{}, 0644); err != nil {
		panic(err)
	}
}
