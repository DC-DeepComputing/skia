// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This executable runs a Bazel(isk) build command for a single label using the provided
// config (which is assumed to be in //bazel/buildrc) and any provided Bazel args.
// This handles any setup needed to run Bazel on our CI machines before running the task, like
// setting up logs and the Bazel cache.
package main

import (
	"context"
	"flag"
	"fmt"
	"path/filepath"

	sk_exec "go.skia.org/infra/go/exec"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
)

var (
	// Required properties for this task.
	gitPath   = flag.String("git_path", "", "Location of git binary to use for diffs.")
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

	if *gitPath == "" {
		td.Fatal(ctx, fmt.Errorf("--git_path is required"))
	}

	wd, err := os_steps.Abs(ctx, *workdir)
	if err != nil {
		td.Fatal(ctx, err)
	}
	skiaPath := filepath.Join(wd, "skia")

	if err := regenerateGLInterfaces(ctx, skiaPath); err != nil{

	}

	diffBefore, err := gitDiff(ctx, *gitPath, skiaPath)
	if err != nil {
		td.Fatal(ctx, err)
	}
	fmt.Printf("Diff Before: \n%s\n", diffBefore)
}

func regenerateGLInterfaces(ctx context.Context, skiaPath string) error {
	return td.Do(ctx, "Regenerate GL Interfaces", func(ctx context.Context) error {
		runCmd := &sk_exec.Command{
			Name:       "make",
			Args:       []string{"-C", "tools/gpu/gl/interface", "generate"},
			InheritEnv: true, // Need to make sure go is on the path
			Dir:        skiaPath,
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

// gitDiff ...
func gitDiff(ctx context.Context, gitPath, skiaPath string) (string, error) {
	step := fmt.Sprintf("git diff %s", skiaPath)
	var rv string
	err := td.Do(ctx, td.Props(step), func(ctx context.Context) error {
		runCmd := &sk_exec.Command{
			Name:       gitPath,
			Args:       []string{"diff", "--no-ext-diff"},
			InheritEnv: false,
			Dir:        skiaPath,
			LogStdout:  true,
			LogStderr:  true,
		}
		var err error
		rv, err = sk_exec.RunCommand(ctx, runCmd)
		if err != nil {
			return err
		}
		return nil
	})
	return rv, err
}
