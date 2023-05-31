// Copyright 2023 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This program generates file //bazel/androidrc.
package main

//go:generate bazelisk run //bazel/gen_androidrc -- --android-devices-bzl ${PWD}/../devices/android_devices.bzl --output ${PWD}/../androidrc

import (
	"bytes"
	"flag"
	"os"
	"text/template"

	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/go/sklog"
	"go.starlark.net/starlark"
)

const configFileTemplate = `# Generated file. DO NOT EDIT.
{{range .}}
build:{{.}}          --platforms=//bazel/platform:{{.}} --cc_output_directory_tag={{.}}
build:{{.}}_with_rbe --config={{.}} --config=linux_rbe
{{end}}`

func main() {
	androidDevicesBzl := flag.String("android-devices-bzl", "", "Path to the android_devices.bzl file.")
	outputFile := flag.String("output", "/dev/stdout", "Output file.")
	flag.Parse()

	deviceNames, err := getAndroidDeviceNames(*androidDevicesBzl)
	if err != nil {
		sklog.Fatal(err)
	}

	tmpl, err := template.New("config").Parse(configFileTemplate)
	if err != nil {
		sklog.Fatal(err)
	}

	buf := &bytes.Buffer{}
	if err := tmpl.Execute(buf, deviceNames); err != nil {
		sklog.Fatal(err)
	}

	if err := os.WriteFile(*outputFile, buf.Bytes(), 0644); err != nil {
		sklog.Fatal(err)
	}
}

func getAndroidDeviceNames(androidDevicesBzlPath string) ([]string, error) {
	globals, err := starlark.ExecFile(&starlark.Thread{}, androidDevicesBzlPath, nil, nil)
	if err != nil {
		return nil, skerr.Wrap(err)
	}

	androidDevicesValue, ok := globals["ANDROID_DEVICES"]
	if !ok {
		return nil, skerr.Fmt("no ANDROID_DEVICES global found in %s", androidDevicesBzlPath)
	}

	androidDevices, ok := androidDevicesValue.(*starlark.Dict)
	if !ok {
		return nil, skerr.Fmt("ANDROID_DEVICES is not a dict")
	}

	var names []string
	for _, key := range androidDevices.Keys() {
		keyStr, ok := key.(starlark.String)
		if !ok {
			return nil, skerr.Fmt("ANDROID_DEVICES key %q is not a string", key)
		}
		names = append(names, keyStr.GoString())
	}
	return names, nil
}
