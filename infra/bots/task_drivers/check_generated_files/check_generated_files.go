// Copyright 2022 Google LLC
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
	"path/filepath"

	"go.skia.org/infra/go/common"
	sk_exec "go.skia.org/infra/go/exec"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
)

var (
	// Required properties for this task.
	bazelArgs = common.NewMultiStringFlag("bazel_arg", nil, "Additional arguments that should be forwarded directly to the Bazel invocation.")
	gitPath   = flag.String("git_path", "", "Location of git binary to use for diffs.")
	projectId = flag.String("project_id", "", "ID of the Google Cloud project.")
	taskId    = flag.String("task_id", "", "ID of this task.")
	taskName  = flag.String("task_name", "", "Name of the task.")
	workdir   = flag.String("workdir", ".", "Working directory, the root directory of a full Skia checkout")
	// Optional flags.
	local  = flag.Bool("local", false, "True if running locally (as opposed to on the CI/CQ)")
	output = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
)

func main() {
	// StartRun calls flag.Parse()
	ctx := td.StartRun(projectId, taskId, taskName, output, local)
	defer td.EndRun(ctx)

	if *gitPath == "" {
		td.Fatal(ctx, fmt.Errorf("--git_path is required"))
	}

	wd, err := os_steps.Abs(ctx, *workdir)
	if err != nil {
		td.Fatal(ctx, err)
	}
	absGit, err := os_steps.Abs(ctx, *gitPath)
	if err != nil {
		td.Fatal(ctx, err)
	}
	if _, err := os_steps.Stat(ctx, absGit); err != nil {
		fmt.Printf("Cannot stat git binary %s\n", absGit)
		td.Fatal(ctx, err)
	}
	skiaPath := filepath.Join(wd, "skia")

	// When running on the CI, there is not a git checkout here, so we make a temp one.
	if !*local {
		if err := gitInit(ctx, absGit, skiaPath); err != nil {
			td.Fatal(ctx, err)
		}
	}

	if err := bazelRun(ctx, skiaPath, "//tools/gpu/gl/interface:generate_gl_interfaces", *bazelArgs...); err != nil {
		td.Fatal(ctx, err)
	}

	skslTests := []string{
		"compile_glsl_tests",
		"compile_glsl_nosettings_tests",
		"compile_metal_tests",
		"compile_skvm_tests",
		"compile_stage_tests",
		"compile_spirv_tests",
		"compile_wgsl_tests",
	}
	for _, label := range skslTests {
		if err := bazelRun(ctx, skiaPath, "//tools/skslc:"+label, *bazelArgs...); err != nil {
			td.Fatal(ctx, err)
		}
	}

	// FIXME(kjlubick): Also generate GPU driver workarounds once https://skia-review.googlesource.com/c/skia/+/600436 lands

	if err := bazelRun(ctx, skiaPath, "//tools/sksl-minify:minify_srcs", *bazelArgs...); err != nil {
		td.Fatal(ctx, err)
	}

	if err := bazelRun(ctx, skiaPath, "//tools/sksl-minify:minify_tests", *bazelArgs...); err != nil {
		td.Fatal(ctx, err)
	}

	diffAfter, err := gitDiff(ctx, absGit, skiaPath)
	if err != nil {
		td.Fatal(ctx, err)
	}
	if diffAfter != "" {
	    td.Fatal(ctx, fmt.Errorf("Non empty diff: %s", diffAfter))
	}
}

func bazelRun(ctx context.Context, skiaPath, label string, args ...string) error {
	return td.Do(ctx, td.Props("bazel run "+label), func(ctx context.Context) error {
		runCmd := &sk_exec.Command{
			Name:       "bazelisk",
			Args:       append([]string{"run", label}, args...),
			InheritEnv: true, // Need to make sure bazelisk is on the path
			Dir:        skiaPath,
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

// gitInit
func gitInit(ctx context.Context, gitPath, skiaPath string) error {
	step := fmt.Sprintf("Setting git baseline in %s", skiaPath)
	err := td.Do(ctx, td.Props(step), func(ctx context.Context) error {
		initCmd := &sk_exec.Command{
			Name:       gitPath,
			Args:       []string{"init"},
			InheritEnv: false,
			Dir:        skiaPath,
			LogStdout:  true,
			LogStderr:  true,
		}
		if _, err := sk_exec.RunCommand(ctx, initCmd); err != nil {
			return err
		}
		addCmd := &sk_exec.Command{
			Name:       gitPath,
			Args:       []string{"add", "."},
			InheritEnv: false,
			Dir:        skiaPath,
			LogStdout:  true,
			LogStderr:  true,
		}
		if _, err := sk_exec.RunCommand(ctx, addCmd); err != nil {
			return err
		}
		commitCmd := &sk_exec.Command{
			Name:       gitPath,
			Args:       []string{"commit", "-m", "baseline commit"},
			InheritEnv: false,
			Dir:        skiaPath,
			LogStdout:  true,
			LogStderr:  true,
		}
		if _, err := sk_exec.RunCommand(ctx, commitCmd); err != nil {
			return err
		}
		return nil
	})
	return err
}

// gitDiff ...
func gitDiff(ctx context.Context, gitPath, skiaPath string) (string, error) {
	step := fmt.Sprintf("git diff %s", skiaPath)
	var rv string
	err := td.Do(ctx, td.Props(step), func(ctx context.Context) error {
		runCmd := &sk_exec.Command{
			Name:       gitPath,
			Args:       []string{"diff", "--no-ext-diff"},
			InheritEnv: false,
			Dir:        skiaPath,
			LogStdout:  true,
			LogStderr:  true,
		}
		var err error
		rv, err = sk_exec.RunCommand(ctx, runCmd)
		if err != nil {
			return err
		}
		fmt.Printf("Diff Return value: \n%s\n", rv)
		return nil
	})
	return rv, err
}
