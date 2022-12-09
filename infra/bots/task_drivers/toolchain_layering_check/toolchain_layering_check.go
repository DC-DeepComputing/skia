// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This executable tests the "layering_check" feature of our custom Bazel toolchain. It builds
// a target multiple times with different defines, which turn on a different "invalid" include
// and verifies that they fail.
package main

import (
	"context"
	"flag"
	"fmt"
	"path/filepath"

	sk_exec "go.skia.org/infra/go/exec"
	"go.skia.org/infra/task_driver/go/lib/bazel"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
)

var (
	// Required properties for this task.
	cachePath = flag.String("cache_path", "/mnt/pd0/bazel_cache", "The path where the Bazel cache should live. This should be able to hold tens of GB at least.")
	config    = flag.String("test_config", "", "A custom configuration specified in //bazel/buildrc. This configuration potentially encapsulates many features and options.")
	projectId = flag.String("project_id", "", "ID of the Google Cloud project.")
	taskId    = flag.String("task_id", "", "ID of this task.")
	taskName  = flag.String("task_name", "", "Name of the task.")
	workdir   = flag.String("workdir", ".", "Working directory, the root directory of a full Skia checkout")
	// Optional flags.
	local  = flag.Bool("local", false, "True if running locally (as opposed to on the CI/CQ)")
	output = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
)

func main() {
	// StartRun calls flag.Parse()
	ctx := td.StartRun(projectId, taskId, taskName, output, local)
	defer td.EndRun(ctx)

	wd, err := os_steps.Abs(ctx, *workdir)
	if err != nil {
		td.Fatal(ctx, err)
	}
	skiaDir := filepath.Join(wd, "skia")

	opts := bazel.BazelOptions{
		// We want the cache to be on a bigger disk than default. The root disk, where the home
		// directory (and default Bazel cache) lives, is only 15 GB on our GCE VMs.
		CachePath: *cachePath,
	}
	if err := bazel.EnsureBazelRCFile(ctx, opts); err != nil {
		td.Fatal(ctx, err)
	}

	testLib := "//experimental/bazel_test/client:client_lib"

	// This build should succeed
	if err := bazelBuild(ctx, skiaDir, testLib, *config); err != nil {
		td.Fatal(ctx, err)
	}

	// All of these should fail
	expectedFailures := []string{
		"--copt=-DHEADER_INCLUDES_TRANSITIVE_HEADER=1",
		"--copt=-DHEADER_INCLUDES_PRIVATE_HEADER=1",
		"--copt=-DSOURCE_INCLUDES_TRANSITIVE_HEADER=1",
		"--copt=-DSOURCE_INCLUDES_PRIVATE_HEADER=1",
	}
	for _, define := range expectedFailures {
		if err := bazelBuild(ctx, skiaDir, testLib, *config, define); err == nil {
			// Yes, the condition above is intentionally == nil because we are expecting a failure.
			td.Fatal(ctx, fmt.Errorf("Expected failure but didn't get one with %s", define))
		}
	}
}

// bazelBuild builds the target referenced by the given absolute label passing the provided
// config and any additional args to the build command. Instead of calling Bazel directly, we use
// Bazelisk to make sure we use the right version of Bazel, as defined in the .bazelversion file
// at the Skia root.
func bazelBuild(ctx context.Context, checkoutDir, label, config string, args ...string) error {
	step := fmt.Sprintf("Build %s with extra flags %s", label, args)
	return td.Do(ctx, td.Props(step), func(ctx context.Context) error {
		runCmd := &sk_exec.Command{
			Name: "bazelisk",
			Args: append([]string{"build",
				label,
				"--config=" + config, // Should be defined in //bazel/buildrc
			}, args...),
			InheritEnv: true, // Makes sure bazelisk is on PATH
			Dir:        checkoutDir,
			LogStdout:  true,
			LogStderr:  true,
		}
		_, err := sk_exec.RunCommand(ctx, runCmd)
		if err != nil {
			return err
		}
		return nil
	})
}
