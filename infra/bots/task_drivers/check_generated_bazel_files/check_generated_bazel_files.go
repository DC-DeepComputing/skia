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
	"errors"
	"flag"
	"fmt"
	"path/filepath"
	"strconv"

	sk_exec "go.skia.org/infra/go/exec"
	"go.skia.org/infra/task_driver/go/lib/bazel"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
)

// This value is arbitrarily selected.
const rbeJobs = 100

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
		// We choose a path that is not in the home directory (which is the default) so it is easier
		// to monitor/clean up if we are running low on disk space.
		CachePath: "/mnt/pd0/bazel_cache",
	}
	if err := bazel.EnsureBazelRCFile(ctx, opts); err != nil {
		td.Fatal(ctx, err)
	}

	if err := ensureNoGeneratedDiffs(ctx, skiaDir); err != nil {
		td.Fatal(ctx, err)
	}

	if err := smokeTestBuild(ctx, skiaDir); err != nil {
		td.Fatal(ctx, err)
	}
}

func ensureNoGeneratedDiffs(ctx context.Context, checkoutDir string) error {
	return td.Do(ctx, td.Props("Regenerate Bazel files"), func(ctx context.Context) error {
		genCmd := &sk_exec.Command{
			Name:       "make",
			Args:       []string{"generate"},
			InheritEnv: true, // Makes sure bazelisk is on PATH
			Dir:        filepath.Join(checkoutDir, "bazel"),
			LogStdout:  true,
			LogStderr:  true,
		}
		_, err := sk_exec.RunCommand(ctx, genCmd)
		if err != nil {
			return err
		}
		diffCmd := &sk_exec.Command{
			Name:       "git",
			Args:       []string{"diff", "--no-ext-diff"},
			InheritEnv: true, // Makes sure git is on PATH
			Dir:        checkoutDir,
			LogStdout:  true,
			LogStderr:  true,
		}
		diff, err := sk_exec.RunCommand(ctx, diffCmd)
		if err != nil {
			return err
		}
		if diff != "" {
			fmt.Println("Unexpected diffs after generating Bazel files")
			fmt.Println(diff)
			fmt.Println("You need to run make generate from //bazel.")
			fmt.Println("You may also need to manually add your new files to srcs")
			fmt.Println("in the BUILD.bazel of the parent of your added files.")
			return errors.New("unexpected generated files")
		}
		return nil
	})
}

// smokeTestBuild builds and links something with Bazelisk to make sure the generated BUILD.bazel
// files and the manually curated lists of files continue to work.
func smokeTestBuild(ctx context.Context, checkoutDir string) error {
	return td.Do(ctx, td.Props("Smoke test compile+link"), func(ctx context.Context) error {
		runCmd := &sk_exec.Command{
			Name: "bazelisk",
			Args: []string{"build",
				"--config=linux-rbe", // Compile using RBE
				"--jobs=" + strconv.Itoa(rbeJobs),
				"//example:hello_world_gl", // This compiles and links, so is a good smoke test
			},
			InheritEnv: true, // Makes sure bazelisk is on PATH
			Dir:        checkoutDir,
			LogStdout:  true,
			LogStderr:  true,
		}
		_, err := sk_exec.RunCommand(ctx, runCmd)
		if err != nil {
			fmt.Println("===============================================")
			fmt.Println("Bazel smoke test failed!")
			fmt.Println("You may need to manually update the :srcs target in the appropriate")
			fmt.Println("BUILD.bazel file to include files/targets you added or delete ones")
			fmt.Println("corresponding to deleted files.")
			return err
		}
		return nil
	})
}
