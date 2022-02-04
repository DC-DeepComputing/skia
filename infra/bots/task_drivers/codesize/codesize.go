// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"context"
	"flag"
	"fmt"
	"time"

	"cloud.google.com/go/storage"
	"google.golang.org/api/option"

	"go.skia.org/infra/go/auth"
	"go.skia.org/infra/go/exec"
	"go.skia.org/infra/go/gcs"
	"go.skia.org/infra/go/gcs/gcsclient"
	"go.skia.org/infra/go/httputils"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/task_driver/go/lib/auth_steps"
	"go.skia.org/infra/task_driver/go/lib/checkout"
	"go.skia.org/infra/task_driver/go/td"
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

	rs, err := checkout.GetRepoState(checkoutFlags)
	if err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

	gcsClient, err := makeGcsClient(ctx, *local)
	if err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

	bloatyOutput, err := runBloaty(ctx, *binaryName)
	if err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

	if rs.Issue != "" && rs.Patchset != "" {
		if err = uploadBloatyFileFromTryjob(ctx, gcsClient, rs.Issue, rs.Patchset, *taskId, *compileTaskName, *binaryName+".tsv", bloatyOutput); err != nil {
			td.Fatal(ctx, skerr.Wrap(err))
		}
	} else {
		// TODO(lovisolo): Implement.
		td.Fatalf(ctx, "This task driver should be run only as a try bot")
	}
}

func makeGcsClient(ctx context.Context, local bool) (*gcsclient.StorageClient, error) {
	// Create token source with scope for GCS access.
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

func uploadBloatyFileFromTryjob(ctx context.Context, gcsClient gcs.GCSClient, issue, patchset, taskId, compileTaskName, fileName, fileContents string) error {
	yearMonthDate := time.Now().Format("2006/01/02") // YYYY/MM/DD.

	// Path format: <YYYY>/<MM>/<DD>/tryjob/<issue>/<patchset>/<task ID>/<compile task name>/<file name>
	// Example: 2022/01/31/tryjob/12345/3/CkPp9ElAaEXyYWNHpXHU/Build-Debian10-Clang-x86_64-Release/dm.tsv
	path := fmt.Sprintf("%s/tryjob/%s/%s/%s/%s/%s", yearMonthDate, issue, patchset, taskId, compileTaskName, fileName)
	gcsURL := fmt.Sprintf("gs://%s/%s", gcsBucketName, path)

	return td.Do(ctx, td.Props(fmt.Sprintf("Upload file %s to %s", fileName, gcsURL)), func(ctx context.Context) error {
		if err := gcsClient.SetFileContents(ctx, path, gcs.FILE_WRITE_OPTS_TEXT, []byte(fileContents)); err != nil {
			return fmt.Errorf("Could not write task to %s: %s", gcsURL, err)
		}
		return nil
	})
}
