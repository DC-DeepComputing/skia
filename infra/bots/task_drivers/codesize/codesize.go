// Copyright 2022 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This task driver takes a binary (e.g. "dm") built by a Build-* task (e.g.
// "Build-Debian10-Clang-x86_64-Release"), runs Bloaty against the binary to compute code size
// statistics and uploads said statistics to https://codesize.skia.org.
package main

import (
	"context"
	"encoding/json"
	"flag"
	"fmt"
	"os"
	"path"

	"cloud.google.com/go/storage"
	"google.golang.org/api/option"

	"go.skia.org/infra/go/auth"
	"go.skia.org/infra/go/exec"
	"go.skia.org/infra/go/gcs"
	"go.skia.org/infra/go/gcs/gcsclient"
	"go.skia.org/infra/go/git"
	"go.skia.org/infra/go/httputils"
	"go.skia.org/infra/go/now"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/task_driver/go/lib/auth_steps"
	"go.skia.org/infra/task_driver/go/lib/checkout"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
	"go.skia.org/infra/task_scheduler/go/types"
)

const gcsBucketName = "skia-codesize"

// BloatyOutputMetadata contains the Bloaty version and command-line arguments used, and metadata
// about the task where Bloaty was invoked. This struct is serialized into a JSON file that is
// uploaded to GCS alongside the Bloaty output file.
//
// TODO(lovisolo): Move this struct to the buildbot repository.
type BloatyOutputMetadata struct {
	SwarmingTaskID string `json:"swarming_task_id"`
	SwarmingServer string `json:"swarming_server"`

	TaskID          string `json:"task_id"`
	TaskName        string `json:"task_name"`
	CompileTaskName string `json:"compile_task_name"`
	BinaryName      string `json:"binary_name"`

	BloatyCipdVersion string   `json:"bloaty_cipd_version"`
	BloatyArgs        []string `json:"bloaty_args"`

	PatchIssue  string `json:"patch_issue"`
	PatchServer string `json:"patch_server"`
	PatchSet    string `json:"patch_set"`
	Repo        string `json:"repo"`
	Revision    string `json:"revision"`

	Timestamp string `json:"timestamp"`
	Author    string `json:"author"`
	Subject   string `json:"subject"`
}

func main() {
	var (
		projectId         = flag.String("project_id", "", "ID of the Google Cloud project.")
		taskId            = flag.String("task_id", "", "ID of this task.")
		taskName          = flag.String("task_name", "", "Name of the task.")
		workDir           = flag.String("workdir", ".", "Working directory.")
		compileTaskName   = flag.String("compile_task_name", "", "Name of the compile task that produced the binary to analyze.")
		binaryName        = flag.String("binary_name", "", "Name of the binary to analyze (e.g. \"dm\").")
		bloatyCipdVersion = flag.String("bloaty_cipd_version", "", "Version of the \"bloaty\" CIPD package used.")
		output            = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
		local             = flag.Bool("local", true, "True if running locally (as opposed to on the bots).")

		checkoutFlags = checkout.SetupFlags(nil)
	)
	ctx := td.StartRun(projectId, taskId, taskName, output, local)
	defer td.EndRun(ctx)

	repoState, err := checkout.GetRepoState(checkoutFlags)
	if err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

	// Check out the Skia repo.
	workDirAbs, err := os_steps.Abs(ctx, *workDir)
	if err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}
	repoCheckout, err := checkout.EnsureGitCheckout(ctx, path.Join(workDirAbs, "repo"), repoState)
	if err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

	gcsClient, err := makeGcsClient(ctx, *local)
	if err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

	if err := runSteps(ctx, repoCheckout, repoState, gcsClient, os.Getenv("SWARMING_TASK_ID"), os.Getenv("SWARMING_SERVER"), *taskId, *taskName, *compileTaskName, *binaryName, *bloatyCipdVersion); err != nil {
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
func runSteps(ctx context.Context, repoCheckout *git.Checkout, repoState types.RepoState, gcsClient gcs.GCSClient, swarmingTaskId, swarmingServer, taskId, taskName, compileTaskName, binaryName, bloatyCipdVersion string) error {
	// // We need to set the user's identity before we're able to run "git rev-parse HEAD", or Git will
	// // complain.
	// if err := td.Do(ctx, td.Props("Set Git user identity"), func(ctx context.Context) error {
	// 	if _, err := repoCheckout.Git(ctx, "config", "user.email", "test@test.com"); err != nil {
	// 		return skerr.Wrap(err)
	// 	}
	// 	if _, err := repoCheckout.Git(ctx, "config", "user.name", "test"); err != nil {
	// 		return skerr.Wrap(err)
	// 	}
	// 	return nil
	// }); err != nil {
	// 	return skerr.Wrap(err)
	// }

	// // Get info about the Git commit.
	// hash, err := repoCheckout.RevParse(ctx, "HEAD")
	// if err != nil {
	// 	return skerr.Wrap(err)
	// }
	// commitInfo, err := repoCheckout.GitDir.Details(ctx, hash)
	// if err != nil {
	// 	return skerr.Wrap(err)
	// }
	// commitTimestamp, err := commitInfo.Timestamp.MarshalText()
	// if err != nil {
	// 	return skerr.Wrap(err)
	// }

	// Build metadata structure.
	metadata := &BloatyOutputMetadata{
		SwarmingTaskID:    swarmingTaskId,
		SwarmingServer:    swarmingServer,
		TaskID:            taskId,
		TaskName:          taskName,
		CompileTaskName:   compileTaskName,
		BinaryName:        binaryName,
		BloatyCipdVersion: bloatyCipdVersion,
		PatchIssue:        repoState.Issue,
		PatchServer:       repoState.Server,
		PatchSet:          repoState.Patchset,
		Repo:              repoState.Repo,
		Revision:          repoState.Revision,
		// Timestamp:         string(commitTimestamp),
		// Author:            commitInfo.Author,
		// Subject:           commitInfo.Subject,
	}

	// Run Bloaty and capture its output.
	bloatyOutput, err := runBloaty(ctx, binaryName, metadata)
	if err != nil {
		return skerr.Wrap(err)
	}

	gcsDir := computeTargetGcsDirectory(ctx, repoState, taskId, compileTaskName)

	// Upload Bloaty output to GCS.
	if err = uploadFileToGcs(ctx, gcsClient, fmt.Sprintf("%s/%s.tsv", gcsDir, binaryName), bloatyOutput); err != nil {
		return skerr.Wrap(err)
	}

	// Upload JSON metadata to GCS.
	jsonMetadata, err := json.MarshalIndent(metadata, "", "  ")
	if err != nil {
		return skerr.Wrap(err)
	}
	if err = uploadFileToGcs(ctx, gcsClient, fmt.Sprintf("%s/%s.json", gcsDir, binaryName), string(jsonMetadata[:])); err != nil {
		return skerr.Wrap(err)
	}

	return nil
}

// runBloaty runs Bloaty against the given binary and returns the Bloaty output in TSV format. It
// populates the given metadata struct with the Bloaty command-line arguments used.
func runBloaty(ctx context.Context, binaryName string, metadata *BloatyOutputMetadata) (string, error) {
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
		metadata.BloatyArgs = runCmd.Args
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
	fmt.Printf("Uploading %s with contents:\n%s\n", gcsURL, contents)
	return td.Do(ctx, td.Props(fmt.Sprintf("Upload %s", gcsURL)), func(ctx context.Context) error {
		if err := gcsClient.SetFileContents(ctx, path, gcs.FILE_WRITE_OPTS_TEXT, []byte(contents)); err != nil {
			return fmt.Errorf("Could not write task to %s: %s", gcsURL, err)
		}
		return nil
	})
}
