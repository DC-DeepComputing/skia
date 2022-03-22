// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This executable checks that we can build certain Bazel targets using --features skia_enforce_iwyu
// which validates that a subset of files are properly declaring all includes that they use.
package main

import (
	"context"
	"flag"
	"path/filepath"

	sk_exec "go.skia.org/infra/go/exec"
	"go.skia.org/infra/task_driver/go/lib/bazel"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
)

var (
	// Required properties for this task.
	projectId = flag.String("project_id", "", "ID of the Google Cloud project.")
	taskId    = flag.String("task_id", "", "ID of this task.")
	taskName  = flag.String("task_name", "", "Name of the task.")
	workdir   = flag.String("workdir", ".", "Working directory, the root directory of a full Skia checkout")
	// Optional flags.
	local  = flag.Bool("local", false, "True if running locally (as opposed to on the CI/CQ)")
	output = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
)

func main() {
	ctx := td.StartRun(projectId, taskId, taskName, output, local)
	defer td.EndRun(ctx)

	wd, err := os_steps.Abs(ctx, *workdir)
	if err != nil {
		td.Fatal(ctx, err)
	}
	skiaDir := filepath.Join(wd, "skia")

	opts := bazel.BazelOptions{
		CachePath: "/mnt/pd0/bazel_cache",
	}
	if err := bazel.EnsureBazelRCFile(ctx, opts); err != nil {
		td.Fatal(ctx, err)
	}

	if err := regenerateBazelFiles(ctx, skiaDir); err != nil {
		td.Fatal(ctx, err)
	}

	targets := []string{"//example:hello_world_gl", "//:skia_core", "//src/svg/..."}
	for _, t := range targets {
		if err := bazelCheckIncludes(ctx, skiaDir, t); err != nil {
			td.Fatal(ctx, err)
		}
	}

	if err := bazelCheckIncludes(ctx, skiaDir, "//src/svg/...",
		"--gpu_backend=gl_backend", "--include_decoder=jpeg_decode_codec"); err != nil {
		td.Fatal(ctx, err)
	}
}

func regenerateBazelFiles(ctx context.Context, checkoutDir string) error {
	return td.Do(ctx, td.Props("regenerate BUILD.bazel files"), func(ctx context.Context) error {
		runCmd := &sk_exec.Command{
			Name:       "make",
			Args:       []string{"generate"},
			InheritEnv: true, // Makes sure bazelisk is on PATH
			Dir:        filepath.Join(checkoutDir, "bazel"),
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

// bazelCheckIncludes builds the given label with the feature enabled that treats improper include
// declarations as a compilation error.
func bazelCheckIncludes(ctx context.Context, checkoutDir, label string, opts ...string) error {
	return td.Do(ctx, td.Props("Build and check includes for "+label), func(ctx context.Context) error {
		runCmd := &sk_exec.Command{
			Name: "bazelisk",
			Args: append([]string{"build",
				"--config=clang",          // Use the hermetic toolchain for the Linux host
				"--sandbox_base=/dev/shm", // Use tempfs (i.e. ramdisk) to improve compile speed
				"--features=skia_enforce_iwyu",
				label,
			}, opts...),
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
