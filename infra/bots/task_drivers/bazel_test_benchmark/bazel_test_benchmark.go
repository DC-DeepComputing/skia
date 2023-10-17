// Copyright 2023 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This task driver runs a single Bazel test target representing one or more benchmarks, or a Bazel
// test suite consisting of multiple such targets, using the provided config (which is assumed to
// be in //bazel/buildrc). Benchmark results are uploaded to a GCS bucket for ingestion by Perf.
// This task driver handles any setup steps needed to run Bazel on our CI machines before running
// the task, such as setting up logs and the Bazel cache.

package main

import (
	"context"
	"flag"
	"fmt"
	"path/filepath"
	"sort"

	"cloud.google.com/go/storage"
	"go.skia.org/infra/go/auth"
	sk_exec "go.skia.org/infra/go/exec"
	"go.skia.org/infra/go/gcs/gcsclient"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/task_driver/go/lib/auth_steps"
	"go.skia.org/infra/task_driver/go/lib/bazel"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
	"go.skia.org/skia/infra/bots/task_drivers/common"
	"google.golang.org/api/option"
)

var (
	// Required properties for this task.
	//
	// We want the cache to be on a bigger disk than default. The root disk, where the home directory
	// (and default Bazel cache) lives, is only 15 GB on our GCE VMs.
	projectId = flag.String("project_id", "", "ID of the Google Cloud project.")
	taskId    = flag.String("task_id", "", "ID of this task.")
	taskName  = flag.String("task_name", "", "Name of the task.")
	workdir   = flag.String("workdir", ".", "Working directory.")

	// goldctl data.
	gitCommit = flag.String("git_commit", "", "The git hash to which the data should be associated.")
	// changelistID     = flag.String("changelist_id", "", "Should be non-empty only when run on the CQ.")
	// patchsetOrderStr = flag.String("patchset_order", "", "Should be non-zero only when run on the CQ.")
	// tryjobID         = flag.String("tryjob_id", "", "Should be non-zero only when run on the CQ.")

	// Optional flags.
	local  = flag.Bool("local", false, "True if running locally (as opposed to on the CI/CQ)")
	output = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
)

const perfGCSBucketName = "skia-perf"

func main() {
	bazelFlags := common.MakeBazelFlags(common.MakeBazelFlagsOpts{
		Label:  true,
		Config: true,
	})

	// StartRun calls flag.Parse().
	ctx := td.StartRun(projectId, taskId, taskName, output, local)
	defer td.EndRun(ctx)

	bazelFlags.Validate(ctx)

	wd, err := os_steps.Abs(ctx, *workdir)
	if err != nil {
		td.Fatal(ctx, err)
	}

	opts := bazel.BazelOptions{
		CachePath: *bazelFlags.CacheDir,
	}
	if err := bazel.EnsureBazelRCFile(ctx, opts); err != nil {
		td.Fatal(ctx, err)
	}

	// Make an HTTP client with the required permissions to hit GCS, Gerrit and Gitiles.
	httpClient, _, err := auth_steps.InitHttpClient(ctx, *local, auth.ScopeReadWrite, auth.ScopeUserinfoEmail)
	if err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

	// Make a GCS client with the required permissions to upload to the perf.skia.org GCS bucket.
	store, err := storage.NewClient(ctx, option.WithHTTPClient(httpClient))
	if err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

	gcsClient := gcsclient.New(store, perfGCSBucketName)

	if err := run(ctx, *bazelFlags.CacheDir, taskDriverArgs{
		UploadToPerfArgs: common.UploadToPerfArgs{
			GCSClient: gcsClient,
			GitCommit: *gitCommit,
			TaskName:  *taskName,
			TaskID:    *taskId,
		},
		checkoutDir: filepath.Join(wd, "skia"),
		bazelLabel:  *bazelFlags.Label,
		bazelConfig: *bazelFlags.Config,
	}); err != nil {
		td.Fatal(ctx, err)
	}
}

// taskDriverArgs gathers the inputs to this task driver, and decouples the task driver's
// entry-point function from the command line flags, which facilitates writing unit tests.
type taskDriverArgs struct {
	common.UploadToPerfArgs
	checkoutDir string
	bazelLabel  string
	bazelConfig string
}

// run is the entrypoint of this task driver.
func run(ctx context.Context, bazelCacheDir string, tdArgs taskDriverArgs) error {
	outputsZipPath, err := common.ValidateLabelAndReturnOutputsZipPath(tdArgs.checkoutDir, tdArgs.bazelLabel)
	if err != nil {
		return skerr.Wrap(err)
	}

	if err := bazelTest(ctx, tdArgs.checkoutDir, tdArgs.bazelLabel, tdArgs.bazelConfig, computeKeyFlag()); err != nil {
		return skerr.Wrap(err)
	}

	// TODO(lovisolo): Upload to perf.
	if err := td.Do(ctx, td.Props(fmt.Sprintf("TODO: Extract JSON file from %s and upload it to Perf", outputsZipPath)), func(ctx context.Context) error { return nil }); err != nil {
		return skerr.Wrap(err)
	}

	// TODO(lovisolo): DO NOT SUBMIT - Skip if it's a tryjob.
	if err := common.UploadToPerf(ctx, tdArgs.UploadToPerfArgs, outputsZipPath); err != nil {
		return skerr.Wrap(err)
	}

	if !*local {
		if err := common.BazelCleanIfLowDiskSpace(ctx, bazelCacheDir, tdArgs.checkoutDir, "bazelisk"); err != nil {
			return skerr.Wrap(err)
		}
	}

	return nil
}

// computeKeyFlag returns the --key flag as expected by BazelBenchmarkTestRunner.cpp with the
// information that should be determined by the task driver, such as OS/architecture, device under
// test, etc.
func computeKeyFlag() []string {
	keyValuePairs := common.ComputeTaskSpecificGoldAndPerfKeyValuePairs()

	// Sort keys for determinism.
	var keys []string
	for key, _ := range keyValuePairs {
		keys = append(keys, key)
	}
	sort.Strings(keys)

	flag := []string{"--key"}
	for _, key := range keys {
		flag = append(flag, key, keyValuePairs[key])
	}

	return flag
}

// bazelTest runs the test referenced by the given fully qualified Bazel label under the given
// config.
func bazelTest(ctx context.Context, checkoutDir, label, config string, testArgs []string) error {
	args := []string{"test",
		label,
		"--config=" + config, // Should be defined in //bazel/buildrc.
		"--test_output=errors",
		"--jobs=100",
	}
	for _, testArg := range testArgs {
		args = append(args, "--test_arg="+testArg)
	}

	return td.Do(ctx, td.Props(fmt.Sprintf("Test %s with config %s", label, config)), func(ctx context.Context) error {
		runCmd := &sk_exec.Command{
			Name:       "bazelisk",
			Args:       args,
			InheritEnv: true, // Makes sure bazelisk is on PATH.
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
