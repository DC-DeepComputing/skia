// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This program reads the contents of the //infra/bots/jobs.json file and generates Bazel targets
// from BazelBuildGeneratedTarget-* and BazelTestGeneratedTarget-* tasks.

//go:generate bazelisk run //bazel/generate_bazel_targets_from_jobs_json --config=linux_rbe -- --jobs_json $PWD/../../infra/bots/jobs.json --output $PWD/../../tests/generated_tests.bzl

package main

import (
	"bytes"
	"encoding/json"
	"flag"
	"fmt"
	"os"
	"regexp"
	"strings"
	"text/template"

	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/go/sklog"
)

// taskKindToBazelRule maps a BazelBuildGeneratedTarget-* and BazelTestGeneratedTarget-* task kind
// to the Bazel rule used to generate its corresponding target.
var taskKindToBazelRule = map[string]string{
	"AndroidUnitTest": "skia_android_test",
}

// labelDeviceToBazelConstant maps a device in a Bazel label for a generated target (e.g. "pixel5"
// in "pixel5_codec_test") to a constant defined in a .bzl file that represents the device.
var labelDeviceToBazelConstant = map[string]string{
	"pixel5": "PIXEL_5",
	"pixel7": "PIXEL_7",
}

// labelTestGroupToBazelConstant maps a test group in a Bazel label for a generated target (e.g.
// "codec" in "pixel5_codec_test") to a constant defined in a .bzl file that represents the test
// group.
var labelTestGroupToBazelConstant = map[string]string{
	"codec":   "CODEC",
	"ganesh":  "GANESH",
	"pathops": "PATHOPS",
}

var (
	// buildTaskRegexp captures a "bazel build" task for a generated Bazel target.
	buildTaskRegexp = regexp.MustCompile(`^BazelBuildGeneratedTarget-(?P<kind>[a-zA-Z0-9_]+)-(?P<label>[a-zA-Z0-9_]+)$`)

	// testTaskRegexp captures a "bazel test" task for a generated Bazel target.
	testTaskRegexp = regexp.MustCompile(`^BazelTestGeneratedTarget-(?P<kind>[a-zA-Z0-9_]+)-(?P<label>[a-zA-Z0-9_]+)$`)
)

// parseAndValidateJobsJSONFile parses and validates the jobs.json file and returns a map from the
// relevant test job names to their kind and label parts.
func parseAndValidateJobsJSONFile(jobsJSONFileContents []byte) (map[string][2]string, error) {
	var entries []struct {
		Name string `json:"name"`
	}
	if err := json.Unmarshal(jobsJSONFileContents, &entries); err != nil {
		return nil, skerr.Wrap(err)
	}

	var tasks []string
	for _, entry := range entries {
		tasks = append(tasks, entry.Name)
	}

	// These maps map task names to their kind and label.
	buildTasks := map[string][2]string{}
	testTasks := map[string][2]string{}

	// Populate the above maps.
	for _, taskName := range tasks {
		buildTaskMatch := buildTaskRegexp.FindStringSubmatch(taskName)
		testTaskMatch := testTaskRegexp.FindStringSubmatch(taskName)

		if buildTaskMatch != nil && testTaskMatch != nil {
			// Should never happen.
			panic(fmt.Sprintf("Job %s is both a build and a test task. This is a bug.", taskName))
		}

		populateMap := func(regexp *regexp.Regexp, match []string, taskMap map[string][2]string) {
			taskMap[taskName] = [2]string{
				match[regexp.SubexpIndex("kind")],
				match[regexp.SubexpIndex("label")],
			}
		}

		if buildTaskMatch != nil {
			populateMap(buildTaskRegexp, buildTaskMatch, buildTasks)
		}

		if testTaskMatch != nil {
			populateMap(testTaskRegexp, testTaskMatch, testTasks)
		}
	}

	// Verify that each build task has a corresponding test task, and viceversa.
	for buildTaskName := range buildTasks {
		testTaskName := "BazelTest" + strings.TrimPrefix(buildTaskName, "BazelBuild")
		if _, ok := testTasks[testTaskName]; !ok {
			return nil, skerr.Fmt("build task %q does not have a corresponding test task %q", buildTaskName, testTaskName)
		}

	}
	for testTaskName := range testTasks {
		buildTaskName := "BazelBuild" + strings.TrimPrefix(testTaskName, "BazelTest")
		if _, ok := buildTasks[buildTaskName]; !ok {
			return nil, skerr.Fmt("test task %q does not have a corresponding build task %q", testTaskName, buildTaskName)
		}
	}

	return testTasks, nil
}

// generatedBazelTarget represents a generated Bazel target.
type generatedBazelTarget struct {
	// Rule name, e.g. "skia_android_test".
	Rule string

	// Name of the target, e.g. "pixel5_codec_test".
	Name string

	// Device is a Bazel constant for the Device under test, e.g. PIXEL_5.
	Device string

	// TestGroup is a Bazel constant for the group of tests to include, e.g. CODEC.
	TestGroup string
}

// bazelLabelRegexp captures a generated Bazel label.
var bazelLabelRegexp = regexp.MustCompile(`(?P<device>[a-z0-9]+)_(?P<testgroup>[a-z0-9]+)_test`)

// makeBazelTarget validates the given kind and label, and builds a generatedBazelTarget.
func makeBazelTarget(taskName, kind, label string) (generatedBazelTarget, error) {
	// Infer Bazel rule from task kind.
	rule, ok := taskKindToBazelRule[kind]
	if !ok {
		return generatedBazelTarget{}, skerr.Fmt("unknown kind %q for task %q", kind, taskName)
	}

	// Extract label parts.
	match := bazelLabelRegexp.FindStringSubmatch(label)
	if match == nil {
		return generatedBazelTarget{}, skerr.Fmt("Bazel label %q for task %q does not follow the expected format", label, taskName)
	}
	devicePart := match[bazelLabelRegexp.SubexpIndex("device")]
	testGroupPart := match[bazelLabelRegexp.SubexpIndex("testgroup")]

	// Get Bazel constants.
	deviceConstant, ok := labelDeviceToBazelConstant[devicePart]
	if !ok {
		return generatedBazelTarget{}, skerr.Fmt("unknown device %q for task %q", devicePart, taskName)
	}
	testGroupConstant, ok := labelTestGroupToBazelConstant[testGroupPart]
	if !ok {
		return generatedBazelTarget{}, skerr.Fmt("unknown test group %q for task %q", testGroupPart, taskName)
	}

	return generatedBazelTarget{
		Rule:      rule,
		Name:      label,
		Device:    deviceConstant,
		TestGroup: testGroupConstant,
	}, nil
}

const bzlFileTemplate = `"""GENERATED FILE, DO NOT MODIFY."""

load(
    ":tests.bzl",
    "CODEC",
    "GANESH",
    "PATHOPS",
    "PIXEL_5",
    "PIXEL_7",
    "skia_android_test",
)

def generated_tests(name):
    """Produces the generated Bazel test targets.

    Args:
        name: Unused, but required to appease Buildifier.
    """
{{range .}}
    {{.Rule}}(
        name = "{{.Name}}",
        device = {{.Device}},
        test_group = {{.TestGroup}},
    )
{{end}}`

// generateBazelTargetsFromJobsJSONFileContents takes the contents of the jobs.json file and
// returns the contents of a .bzl file with the Bazel targets needed by the
// BazelBuildGeneratedTarget-* and BazelTestGeneratedTarget-* tasks in the jobs.json file.
func generateBazelTargetsFromJobsJSONFileContents(jobsJSONFileContents []byte) (string, error) {
	testTasks, err := parseAndValidateJobsJSONFile(jobsJSONFileContents)
	if err != nil {
		return "", skerr.Wrap(err)
	}

	var targets []generatedBazelTarget
	for taskName, parts := range testTasks {
		kind := parts[0]
		label := parts[1]
		target, err := makeBazelTarget(taskName, kind, label)
		if err != nil {
			return "", skerr.Wrap(err)
		}
		targets = append(targets, target)
	}

	tmpl, err := template.New("template").Parse(bzlFileTemplate)
	if err != nil {
		return "", skerr.Wrap(err)
	}

	buf := &bytes.Buffer{}
	if err := tmpl.Execute(buf, targets); err != nil {
		return "", skerr.Wrap(err)
	}

	return buf.String(), nil
}

func main() {
	jobsJSON := flag.String("jobs_json", "", "Path to the jobs.json file.")
	outputPath := flag.String("output", "", "Output file. Defaults to the standard output. Optional.")
	flag.Parse()

	if *jobsJSON == "" {
		sklog.Fatal("Flag --jobs_json is required.")
	}

	jobsJSONFileContents, err := os.ReadFile(*jobsJSON)
	if err != nil {
		sklog.Fatalf("While reading file %q: %s", *jobsJSON, err)
	}

	bazelTargetsFileContents, err := generateBazelTargetsFromJobsJSONFileContents(jobsJSONFileContents)
	if err != nil {
		sklog.Fatalf("While generating Bazel targets: %s", err)
	}

	if *outputPath != "" {
		if err := os.WriteFile(*outputPath, []byte(bazelTargetsFileContents), 0644); err != nil {
			sklog.Fatalf("While writing file %q: %s", *outputPath, err)
		}
	} else {
		// We use fmt.Printf rather than fmt.Println to avoid printing a spurious \n at the end.
		fmt.Printf("%s", bazelTargetsFileContents)
	}
}
