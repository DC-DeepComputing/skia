// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This executable builds and tests CanvasKit. The tests produce images (aka gms) and these
// are uploaded to Gold.
package main

import (
	"context"
	"flag"
	"fmt"
	"path/filepath"
	"strconv"

	sk_exec "go.skia.org/infra/go/exec"
	"go.skia.org/infra/task_driver/go/lib/bazel"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
)

// This value is arbitrarily selected. It is smaller than our maximum RBE pool size.
const rbeJobs = 100

var (
	// Required properties for this task.
	projectId   = flag.String("project_id", "", "ID of the Google Cloud project.")
	taskId      = flag.String("task_id", "", "ID of this task.")
	taskName    = flag.String("task_name", "", "Name of the task.")
	workdir     = flag.String("workdir", ".", "Working directory, the root directory of a full Skia checkout")
	buildConfig = flag.String("build_config", "release", "The config name (defined in //bazel/buildrc), which indicates how CanvasKit should be compiled before tests are executed.")
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
		// We want the cache to be on a bigger disk than default. The root disk, where the home
		// directory (and default Bazel cache) lives, is only 15 GB on our GCE VMs.
		CachePath: "/mnt/pd0/bazel_cache",
	}
	if err := bazel.EnsureBazelRCFile(ctx, opts); err != nil {
		td.Fatal(ctx, err)
	}

	if err := bazelTest(ctx, skiaDir, "//modules/canvaskit:canvaskit_js_tests", *buildConfig,
		"--config=linux_rbe", "--test_output=streamed", "--jobs="+strconv.Itoa(rbeJobs)); err != nil {
		td.Fatal(ctx, err)
	}

	if err := uploadDataToGold(ctx, skiaDir); err != nil {
		td.Fatal(ctx, err)
	}
}

func bazelTest(ctx context.Context, checkoutDir, label, config string, args ...string) error {
	step := fmt.Sprintf("Running Test %s with config %s and %d extra flags", label, config, len(args))
	return td.Do(ctx, td.Props(step), func(ctx context.Context) error {
		runCmd := &sk_exec.Command{
			Name: "bazelisk",
			Args: append([]string{"test",
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

func uploadDataToGold(ctx context.Context, checkoutDir string) error {
	return td.Do(ctx, td.Props("TODO(kjlubick) upload to Gold"), func(ctx context.Context) error {
		runCmd := &sk_exec.Command{
			Name: "ls",
			Args: append([]string{"-ahl", ".",
				"bazel-testlogs/modules/canvaskit/canvaskit_js_wasms/test.outputs/",
			}),
			Dir:       checkoutDir,
			LogStdout: true,
			LogStderr: true,
		}
		_, err := sk_exec.RunCommand(ctx, runCmd)
		if err != nil {
			return err
		}
		return nil
	})
}
