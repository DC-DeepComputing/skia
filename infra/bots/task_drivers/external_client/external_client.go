// Copyright 2023 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
package main

import (
	"context"
	"flag"
	"fmt"
	"path/filepath"

	sk_exec "go.skia.org/infra/go/exec"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
	"go.skia.org/skia/infra/bots/task_drivers/common"
)

var (
	// Required properties for this task.
	projectId      = flag.String("project_id", "", "ID of the Google Cloud project.")
	taskId         = flag.String("task_id", "", "ID of this task.")
	taskName       = flag.String("task_name", "", "Name of the task.")
	pathInSkia     = flag.String("path_in_skia", "example/external_client", "The directory from which to run the Bazel commands in Docker")
	workdir        = flag.String("workdir", ".", "Working directory, the root directory of a full Skia checkout")

	// Optional flags.
	local  = flag.Bool("local", false, "True if running locally (as opposed to on the CI/CQ)")
	output = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
)

func main() {
	bazelFlags := common.MakeBazelFlags(common.MakeBazelFlagsOpts{
		Label:          true,
	})

	// StartRun calls flag.Parse()
	ctx := td.StartRun(projectId, taskId, taskName, output, local)
	defer td.EndRun(ctx)

	bazelFlags.Validate(ctx)

	wd, err := os_steps.Abs(ctx, *workdir)
	if err != nil {
		td.Fatal(ctx, err)
	}
	workspaceDir := filepath.Join(wd, "skia", *pathInSkia)

	if err := runDocker(ctx, workspaceDir); err != nil {
		td.Fatal(ctx, err)
	}
}

func runDocker(ctx context.Context, checkoutDir string) error {
	step := fmt.Sprintf("Docker thing")
	return td.Do(ctx, td.Props(step), func(ctx context.Context) error {
		runCmd := &sk_exec.Command{
			Name: "docker",
			Args: []string{"version"},
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
