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
	"context"
	"flag"
	"fmt"
	"os"
	"path/filepath"
	"regexp"
	"strings"

	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/task_driver/go/td"
)

var (
	// Required properties for this task.
	baseDir    = flag.String("base_dir", "include/private/base", "The directory of files to check.")
	basePrefix = flag.String("base_prefix", "include/private/base", "The directory of files to check.")
	projectId  = flag.String("project_id", "", "ID of the Google Cloud project.")
	taskId     = flag.String("task_id", "", "ID of this task.")
	taskName   = flag.String("task_name", "", "Name of the task.")
	// Optional flags.
	local  = flag.Bool("local", false, "True if running locally (as opposed to on the CI/CQ)")
	output = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
)

func main() {
	// StartRun calls flag.Parse()
	ctx := td.StartRun(projectId, taskId, taskName, output, local)
	defer td.EndRun(ctx)

	if *baseDir == "" {
		td.Fatal(ctx, fmt.Errorf("--base_dir is required"))
	}

	if err := os.Chdir(*baseDir); err != nil {
		td.Fatal(ctx, err)
	}

	headers, err := filepath.Glob("*.h")
	if err != nil {
		td.Fatal(ctx, err)
	}

	if len(headers) == 0 {
		td.Fatalf(ctx, "No header files found in %s", *baseDir)
	}

	allPassed := true
	for _, hdr := range headers {
		if err := ensureProperIncludes(ctx, hdr, *basePrefix); err != nil {
			allPassed = false
		}
	}

	if !allPassed {
		td.Fatalf(ctx, "Some base files #include in appropriate files")
	}
}

// Matches non-system includes
var includeRegex = regexp.MustCompile(`^\s*#\s*include\s+"(.+)"`)

const skUserConfig = "include/config/SkUserConfig.h"

func ensureProperIncludes(ctx context.Context, hdr, basePrefix string) error {
	step := fmt.Sprintf("Validating #includes of %s", hdr)
	return td.Do(ctx, td.Props(step), func(ctx context.Context) error {
		contents, err := os.ReadFile(hdr)
		if err != nil {
			return skerr.Wrapf(err, "File: %s", hdr)
		}
		invalidIncludes := ""
		lines := strings.Split(string(contents), "\n")
		for _, line := range lines {
			if match := includeRegex.FindStringSubmatch(line); len(match) > 0 {
				file := match[1]
				if !strings.HasPrefix(file, basePrefix) && file != skUserConfig {
					invalidIncludes += file
					invalidIncludes += "\n"
				}
			}
		}
		if len(invalidIncludes) > 0 {
			return fmt.Errorf("Invalid includes for file %s:\n%s", hdr, invalidIncludes)
		}
		return nil
	})
}
