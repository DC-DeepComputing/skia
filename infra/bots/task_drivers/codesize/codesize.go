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
		projectId = flag.String("project_id", "", "ID of the Google Cloud project.")
		taskId    = flag.String("task_id", "", "ID of this task.")
		taskName  = flag.String("task_name", "", "Name of the task.")
		output    = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
		local     = flag.Bool("local", true, "True if running locally (as opposed to on the bots)")

		checkoutFlags = checkout.SetupFlags(nil)
	)
	ctx := td.StartRun(projectId, taskId, taskName, output, local)
	defer td.EndRun(ctx)

	rs, err := checkout.GetRepoState(checkoutFlags)
	if err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

	// if rs.Issue == "" || rs.Patchset == "" {
	// 	td.Fatalf(ctx, "This task driver should be run only as a try bot")
	// }

	// Create token source with scope for GCS access.
	ts, err := auth_steps.Init(ctx, *local, auth.ScopeReadWrite)
	if err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}
	client := httputils.DefaultClientConfig().WithTokenSource(ts).With2xxOnly().Client()
	store, err := storage.NewClient(ctx, option.WithHTTPClient(client))
	if err != nil {
		td.Fatalf(ctx, "Failed to create storage service client: %s", err)
	}
	gcsClient := gcsclient.New(store, gcsBucketName)

	fileName := fmt.Sprintf("%s-%s.json", rs.Issue, rs.Patchset)
	gcsPath := fmt.Sprintf("gs://%s/%s", gcsBucketName, fileName)

	////////
	fmt.Println("ABOUT TO READ gs://skia-codesize/hello.txt")
	bytes, err := gcsClient.GetFileContents(ctx, "hello.txt")
	if err != nil {
		td.Fatalf(ctx, "Couldn't read gs://skia-codesize/hello.txt: %s", err)
	}
	fmt.Printf("Contents of gs://skia-codesize/hello.txt:\n%s\n", string(bytes[:]))
	////////

	// DELETE ME
	fmt.Println("SLEEPING FOR 20 MINS")
	time.Sleep(20 * time.Minute)

	if err = uploadFakeBloatyFile(ctx, rs.Issue, rs.Patchset, fileName, gcsPath, gcsClient); err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}
}

func uploadFakeBloatyFile(ctx context.Context, issue, patchset, fileName, gcsPath string, gcsClient gcs.GCSClient) error {
	ctx = td.StartStep(ctx, td.Props("Upload fake Bloaty output file"))
	defer td.EndStep(ctx)

	msg := fmt.Sprintf("Hello from issue %s, patchset %s!\n", issue, patchset)

	if err := gcsClient.SetFileContents(ctx, fileName, gcs.FILE_WRITE_OPTS_TEXT, []byte(msg)); err != nil {
		return fmt.Errorf("Could not write task to %s: %s", gcsPath, err)
	}

	return nil
}
