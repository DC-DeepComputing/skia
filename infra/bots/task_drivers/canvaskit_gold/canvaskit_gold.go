// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This executable builds and tests CanvasKit. The tests produce images (aka gms) and these
// are uploaded to Gold.
// It requires unzip to be installed (which Bazel already requires).
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
	projectId  = flag.String("project_id", "", "ID of the Google Cloud project.")
	taskId     = flag.String("task_id", "", "ID of this task.")
	taskName   = flag.String("task_name", "", "Name of the task.")
	workdir    = flag.String("workdir", ".", "Working directory, the root directory of a full Skia checkout")
	testConfig = flag.String("test_config", "release", "The config name (defined in //bazel/buildrc), which indicates how CanvasKit should be compiled and tested.")
	cross      = flag.String("cross", "", "[not yet supported] For use with cross-compiling.")
	// goldctl data
	goldctlPath   = flag.String("goldctl_path", "", "The path to the golctl binary on disk.")
	gitCommit     = flag.String("git_commit", "", "The git hash to which the data should be associated. This will be used when changelist_id and patchset_order are not set to report data to Gold that belongs on the primary branch.")
	changelistID  = flag.String("changelist_id", "", "Should be non-empty only when run on the CQ.")
	patchsetOrder = flag.Int("patchset_order", 0, "Should be non-zero only when run on the CQ.")
	tryjobID      = flag.String("tryjob_id", "", "Should be non-zero only when run on the CQ.")

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
	if *cross != "" {
		fmt.Println("Saw --cross, but don't know what to do with that yet.")
	}

	if err := bazelTest(ctx, skiaDir, "//modules/canvaskit:canvaskit_js_tests", *testConfig,
		"--config=linux_rbe", "--test_output=streamed", "--jobs="+strconv.Itoa(rbeJobs)); err != nil {
		td.Fatal(ctx, err)
	}

	conf := goldctlConfig{
		goldctlPath:   *goldctlPath,
		gitCommit:     *gitCommit,
		changelistID:  *changelistID,
		patchsetOrder: *patchsetOrder,
		tryjobID:      *tryjobID,
		corpus:        "canvaskit",
		keys: map[string]string{
			"arch":          "wasm32", // https://github.com/bazelbuild/platforms/blob/da5541f26b7de1dc8e04c075c99df5351742a4a2/cpu/BUILD#L109
			"configuration": *testConfig,
			// TODO(kjlubick) These should be deduced from testConfig
			"browser":          "Chrome",
			"compilation_mode": "Release",
			"cpu_or_gpu":       "GPU",
			"cpu_or_gpu_value": "WebGL",
		},
	}
	if err := uploadDataToGold(ctx, skiaDir, conf); err != nil {
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

type goldctlConfig struct {
	goldctlPath   string
	gitCommit     string
	changelistID  string
	patchsetOrder int
	tryjobID      string
	corpus        string
	keys          map[string]string
}

func uploadDataToGold(ctx context.Context, checkoutDir string, cfg goldctlConfig) error {
	return td.Do(ctx, td.Props("Upload to Gold"), func(ctx context.Context) error {
		zipExtractDir, err := os_steps.TempDir(ctx, "", "gold_outputs")
		if err != nil {
			return err
		}
		if err := extractZip(ctx, filepath.Join(checkoutDir, "bazel-testlogs", "modules", "canvaskit",
			"canvaskit_js_tests", "test.outputs", "outputs.zip"), zipExtractDir); err != nil {
			return err
		}

		// goldctl init
		runCmd := &sk_exec.Command{
			Name:      cfg.goldctlPath,
			Args:      []string{"-help"},
			Dir:       checkoutDir,
			LogStdout: true,
			LogStderr: true,
		}
		_, err = sk_exec.RunCommand(ctx, runCmd)
		if err != nil {
			return err
		}
		return nil

		// loop through all images
		//   goldctl add

		// goldctl finalize

	})
}

func extractZip(ctx context.Context, zipPath, targetDir string) error {
	runCmd := &sk_exec.Command{
		Name:      "unzip",
		Args:      []string{zipPath, "-d", targetDir},
		LogStdout: true,
		LogStderr: true,
	}
	_, err := sk_exec.RunCommand(ctx, runCmd)
	if err != nil {
		return err
	}
	// FIXME(kjlubick) remove this debugging step.
	runCmd = &sk_exec.Command{
		Name:      "ls",
		Args:      []string{"-ahl", targetDir},
		LogStdout: true,
		LogStderr: true,
	}
	_, err = sk_exec.RunCommand(ctx, runCmd)
	if err != nil {
		return err
	}
	return nil
}
