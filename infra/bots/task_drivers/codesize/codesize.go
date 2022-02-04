// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"context"
	"flag"
	"fmt"

	"cloud.google.com/go/storage"
	"google.golang.org/api/option"

	"go.skia.org/infra/go/auth"
	"go.skia.org/infra/go/exec"
	"go.skia.org/infra/go/gcs"
	"go.skia.org/infra/go/gcs/gcsclient"
	"go.skia.org/infra/go/httputils"
	"go.skia.org/infra/go/now"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/task_driver/go/lib/auth_steps"
	"go.skia.org/infra/task_driver/go/lib/checkout"
	"go.skia.org/infra/task_driver/go/td"
	"go.skia.org/infra/task_scheduler/go/types"
)

const gcsBucketName = "skia-codesize"

func main() {
	var (
		projectId       = flag.String("project_id", "", "ID of the Google Cloud project.")
		taskId          = flag.String("task_id", "", "ID of this task.")
		taskName        = flag.String("task_name", "", "Name of the task.")
		compileTaskName = flag.String("compile_task_name", "", "Name of the compile task that produced the binary to analyze.")
		binaryName      = flag.String("binary_name", "", "Name of the binary to analyze (e.g. \"dm\").")
		output          = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
		local           = flag.Bool("local", true, "True if running locally (as opposed to on the bots).")

		checkoutFlags = checkout.SetupFlags(nil)
	)
	ctx := td.StartRun(projectId, taskId, taskName, output, local)
	defer td.EndRun(ctx)

	repoState, err := checkout.GetRepoState(checkoutFlags)
	if err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

	fmt.Printf("Repo state is:\n%+v\n", repoState) // DO NOT SUBMIT

	gcsClient, err := makeGcsClient(ctx, *local)
	if err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

	if err := runSteps(ctx, repoState, gcsClient, *taskId, *compileTaskName, *binaryName); err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}
}

// makeGcsClient makes a GCS client with the necessary permissions to upload files to the
// codesize.skia.org GCS bucket.
func makeGcsClient(ctx context.Context, local bool) (*gcsclient.StorageClient, error) {
	ts, err := auth_steps.Init(ctx, local, auth.ScopeReadWrite)
	if err != nil {
		return nil, skerr.Wrap(err)
	}
	client := httputils.DefaultClientConfig().WithTokenSource(ts).With2xxOnly().Client()
	store, err := storage.NewClient(ctx, option.WithHTTPClient(client))
	if err != nil {
		return nil, skerr.Wrapf(err, "Failed to create storage service client")
	}
	return gcsclient.New(store, gcsBucketName), nil
}

// runSteps all the steps of this task driver.
func runSteps(ctx context.Context, repoState types.RepoState, gcsClient gcs.GCSClient, taskId, compileTaskName, binaryName string) error {
	// Run Bloaty and capture its output.
	bloatyOutput, err := runBloaty(ctx, binaryName)
	if err != nil {
		return skerr.Wrap(err)
	}

	gcsDir := computeTargetGcsDirectory(ctx, repoState, taskId, compileTaskName)

	// Upload Bloaty output to GCS.
	if err = uploadFileToGcs(ctx, gcsClient, fmt.Sprintf("%s/%s.tsv", gcsDir, binaryName), bloatyOutput); err != nil {
		return skerr.Wrap(err)
	}

	// TODO(lovisolo): Upload JSON metadata file to GCS.

	return nil
}

// runBloaty runs Bloaty against the given binary and returns the Bloaty output in TSV format.
//
// TODO(lovisolo): Also return a struct with metadata (Bloaty version, arguments, etc.) that can be
//                 serialized as JSON, then upload it to GCS alongside the Bloaty TSV output file.
func runBloaty(ctx context.Context, binaryName string) (string, error) {
	err := td.Do(ctx, td.Props("List files under $PWD/build (for debugging purposes)"), func(ctx context.Context) error {
		runCmd := &exec.Command{
			Name:       "ls",
			Args:       []string{"build"},
			InheritEnv: true,
			LogStdout:  true,
			LogStderr:  true,
		}
		_, err := exec.RunCommand(ctx, runCmd)
		return err
	})
	if err != nil {
		return "", skerr.Wrap(err)
	}

	var bloatyOutput string

	err = td.Do(ctx, td.Props(fmt.Sprintf("Run Bloaty against binary %q", binaryName)), func(ctx context.Context) error {
		runCmd := &exec.Command{
			Name: "bloaty/bloaty",
			Args: []string{
				"build/" + binaryName,
				"-d",
				"compileunits,symbols",
				"-n",
				"0",
				"--tsv",
			},
			InheritEnv: true,
			LogStdout:  true,
			LogStderr:  true,
		}
		bloatyOutput, err = exec.RunCommand(ctx, runCmd)
		return err
	})
	if err != nil {
		return "", skerr.Wrap(err)
	}

	return bloatyOutput, nil
}

// computeTargetGcsDirectory computs the target GCS directory where to upload the Bloaty output file
// and JSON metadata file.
func computeTargetGcsDirectory(ctx context.Context, repoState types.RepoState, taskId, compileTaskName string) string {
	yearMonthDate := now.Now(ctx).Format("2006/01/02") // YYYY/MM/DD.
	if repoState.Issue == "" && repoState.Patchset == "" {
		// TODO(lovisolo): Implement
		panic("this task driver only supports running as a tryjob at this time")
	} else {
		// Example: 2022/01/31/tryjob/12345/3/CkPp9ElAaEXyYWNHpXHU/Build-Debian10-Clang-x86_64-Release/dm.tsv
		return fmt.Sprintf("%s/tryjob/%s/%s/%s/%s", yearMonthDate, repoState.Patch.Issue, repoState.Patch.Patchset, taskId, compileTaskName)
	}
}

// uploadFileToGcs uploads a file to the codesize.skia.org GCS bucket.
func uploadFileToGcs(ctx context.Context, gcsClient gcs.GCSClient, path, contents string) error {
	gcsURL := fmt.Sprintf("gs://%s/%s", gcsBucketName, path)
	return td.Do(ctx, td.Props(fmt.Sprintf("Upload %s", gcsURL)), func(ctx context.Context) error {
		if err := gcsClient.SetFileContents(ctx, path, gcs.FILE_WRITE_OPTS_TEXT, []byte(contents)); err != nil {
			return fmt.Errorf("Could not write task to %s: %s", gcsURL, err)
		}
		return nil
	})
}
