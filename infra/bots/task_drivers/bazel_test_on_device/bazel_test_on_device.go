// Copyright 2023 Google LLC
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
	"flag"
	"path/filepath"

	"go.skia.org/infra/go/common"
	sk_exec "go.skia.org/infra/go/exec"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
)

var (
	// Required properties for this task.
	projectId      = flag.String("project_id", "", "ID of the Google Cloud project.")
	taskId         = flag.String("task_id", "", "ID of this task.")
	taskName       = flag.String("task_name", "", "Name of the task.")
	workdir        = flag.String("workdir", ".", "Working directory, the root directory of a full Skia checkout")
	command        = flag.String("command", "", "Command to run (e.g. a shell script in a directory with CAS inputs).")
	commandWorkDir = flag.String("command_workdir", "", "Working directory for the command to run (e.g. a directory with CAS inputs).")
	commandArgs    = common.NewMultiStringFlag("command_args", nil, "Any arguments to pass to the command to run.")

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

	// if err := td.Do(ctx, td.Props("Sleep for 20 minutes"), func(ctx context.Context) error {
	// 	time.Sleep(20 * time.Minute)
	// 	return nil
	// }); err != nil {
	// 	td.Fatal(ctx, err)
	// }

	commandWorkDirAbs := filepath.Join(wd, *commandWorkDir)
	commandAbs := filepath.Join(commandWorkDirAbs, *command)

	runCmd := &sk_exec.Command{
		Name:       commandAbs,
		Args:       *commandArgs,
		InheritEnv: true, // Makes sure any CIPD-downloaded tools are available on $PATH.
		Dir:        commandWorkDirAbs,
		LogStdout:  true,
		LogStderr:  true,
	}
	_, err = sk_exec.RunCommand(ctx, runCmd)
	if err != nil {
		td.Fatal(ctx, err)
	}
}
