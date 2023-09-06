// Copyright 2023 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"context"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
	"go.skia.org/infra/go/exec"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
	"go.skia.org/skia/infra/bots/task_drivers/common"
	"go.skia.org/skia/infra/bots/task_drivers/testutils"
)

func TestRun_UnitTest_Success(t *testing.T) {
	commandCollector := exec.CommandCollector{}
	res := td.RunTestSteps(t, false, func(ctx context.Context) error {
		ctx = td.WithExecRunFn(ctx, commandCollector.Run)

		err := run(ctx, taskDriverArgs{
			commandPath:    "/path/to/command",
			commandArgs:    []string{"--foo", "bar", "--baz", "qux"},
			commandWorkDir: "/path/to/workdir",
		})

		assert.NoError(t, err)
		return err
	})

	require.Empty(t, res.Errors)
	require.Empty(t, res.Exceptions)

	testutils.AssertStepNamesMatchStringsOrRegexps(t, res,
		"/path/to/command --foo bar --baz qux",
	)

	require.Len(t, commandCollector.Commands(), 1)

	testutils.AssertCommand(t, commandCollector.Commands()[0],
		"/path/to/workdir",
		"/path/to/command",
		"--foo",
		"bar",
		"--baz",
		"qux",
	)
}

func TestRun_GMTest_Success(t *testing.T) {
	// Given that we have tests for common.UploadToGold(), it suffices to test a couple of
	// "happy" cases here.

	test := func(name string, tdArgs taskDriverArgs, goldctlWorkDir string, goldctlImgtestInitStepName string, goldctlImgtestInitArgsStringsOrRegexps []any) {
		t.Run(name, func(t *testing.T) {
			// Create directory with fake undeclared test outputs.
			testutils.PopulateDir(t, tdArgs.undeclaredOutputsDir, map[string]string{
				// The contents of PNG files does not matter for this test.
				"alfa.png": "fake PNG",
				"alfa.json": `{
					"md5": "a01a01a01a01a01a01a01a01a01a01a0",
					"keys": {
						"build_system": "bazel",
						"name": "alfa",
						"source_type": "gm"
					}
				}`,
				"beta.png": "fake PNG",
				"beta.json": `{
					"md5": "b02b02b02b02b02b02b02b02b02b02b0",
					"keys": {
						"build_system": "bazel",
						"name": "beta",
						"source_type": "gm"
					}
				}`,
			})

			commandCollector := exec.CommandCollector{}
			res := td.RunTestSteps(t, false, func(ctx context.Context) error {
				ctx = td.WithExecRunFn(ctx, commandCollector.Run)

				// We expect os_steps.TempDir() to be called only once to create the goldctl working directory.
				osStepsTempDirAlreadyCalled := false
				ctx = context.WithValue(ctx, os_steps.TempDirContextKey, func(string, string) (string, error) {
					if osStepsTempDirAlreadyCalled {
						require.Fail(t, "os_steps.TempDir called more than once")
					}
					osStepsTempDirAlreadyCalled = true
					return goldctlWorkDir, nil
				})

				err := run(ctx, tdArgs)

				assert.NoError(t, err)
				return err
			})

			require.Empty(t, res.Errors)
			require.Empty(t, res.Exceptions)

			testutils.AssertStepNamesMatchStringsOrRegexps(t, res,
				"/path/to/command --foo bar --baz qux",
				"Gather JSON and PNG files produced by GMs",
				"Gather \"alfa.png\"",
				"Gather \"beta.png\"",
				"Upload GM outputs to Gold",
				"Creating TempDir",
				"/path/to/goldctl auth --work-dir "+goldctlWorkDir+" --luci",
				goldctlImgtestInitStepName,
				"/path/to/goldctl imgtest add --work-dir "+goldctlWorkDir+" --test-name alfa --png-file "+tdArgs.undeclaredOutputsDir+"/alfa.png --png-digest a01a01a01a01a01a01a01a01a01a01a0 --add-test-key build_system:bazel --add-test-key name:alfa --add-test-key source_type:gm",
				"/path/to/goldctl imgtest add --work-dir "+goldctlWorkDir+" --test-name beta --png-file "+tdArgs.undeclaredOutputsDir+"/beta.png --png-digest b02b02b02b02b02b02b02b02b02b02b0 --add-test-key build_system:bazel --add-test-key name:beta --add-test-key source_type:gm",
				"/path/to/goldctl imgtest finalize --work-dir "+goldctlWorkDir,
			)

			require.Len(t, commandCollector.Commands(), 6)

			testutils.AssertCommand(t, commandCollector.Commands()[0],
				"/path/to/workdir",
				"/path/to/command",
				"--foo", "bar",
				"--baz", "qux",
			)

			testutils.AssertCommand(t, commandCollector.Commands()[1],
				testutils.IgnoreCommandDir,
				"/path/to/goldctl",
				"auth",
				"--work-dir", goldctlWorkDir,
				"--luci",
			)

			testutils.AssertCommand(t, commandCollector.Commands()[2],
				testutils.IgnoreCommandDir,
				"/path/to/goldctl",
				goldctlImgtestInitArgsStringsOrRegexps...,
			)

			testutils.AssertCommand(t, commandCollector.Commands()[3],
				testutils.IgnoreCommandDir,
				"/path/to/goldctl",
				"imgtest",
				"add",
				"--work-dir", goldctlWorkDir,
				"--test-name", "alfa",
				"--png-file", tdArgs.undeclaredOutputsDir+"/alfa.png",
				"--png-digest", "a01a01a01a01a01a01a01a01a01a01a0",
				"--add-test-key", "build_system:bazel",
				"--add-test-key", "name:alfa",
				"--add-test-key", "source_type:gm",
			)

			testutils.AssertCommand(t, commandCollector.Commands()[4],
				testutils.IgnoreCommandDir,
				"/path/to/goldctl",
				"imgtest",
				"add",
				"--work-dir", goldctlWorkDir,
				"--test-name", "beta",
				"--png-file", tdArgs.undeclaredOutputsDir+"/beta.png",
				"--png-digest", "b02b02b02b02b02b02b02b02b02b02b0",
				"--add-test-key", "build_system:bazel",
				"--add-test-key", "name:beta",
				"--add-test-key", "source_type:gm",
			)

			testutils.AssertCommand(t, commandCollector.Commands()[5],
				testutils.IgnoreCommandDir,
				"/path/to/goldctl",
				"imgtest",
				"finalize",
				"--work-dir", goldctlWorkDir,
			)
		})
	}

	goldctlWorkDir := t.TempDir()
	test(
		"post-submit task",
		taskDriverArgs{
			UploadToGoldArgs: common.UploadToGoldArgs{
				TestOnlyAllowAnyBazelLabel: true,
				BazelLabel:                 "//some/test:target",
				GoldctlPath:                "/path/to/goldctl",
				GitCommit:                  "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
			},
			commandPath:          "/path/to/command",
			commandArgs:          []string{"--foo", "bar", "--baz", "qux"},
			commandWorkDir:       "/path/to/workdir",
			isGM:                 true,
			undeclaredOutputsDir: t.TempDir(),
		},
		goldctlWorkDir,
		"/path/to/goldctl imgtest init --work-dir "+goldctlWorkDir+" --instance skia --url https://gold.skia.org --bucket skia-infra-gm --git_hash ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99 --key os:linux",
		[]any{
			"imgtest",
			"init",
			"--work-dir", goldctlWorkDir,
			"--instance", "skia",
			"--url", "https://gold.skia.org",
			"--bucket", "skia-infra-gm",
			"--git_hash", "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
			"--key", "os:linux",
		},
	)

	goldctlWorkDir = t.TempDir()
	test(
		"CL task",
		taskDriverArgs{
			UploadToGoldArgs: common.UploadToGoldArgs{
				TestOnlyAllowAnyBazelLabel: true,
				BazelLabel:                 "//some/test:target",
				GoldctlPath:                "/path/to/goldctl",
				GitCommit:                  "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
				ChangelistID:               "changelist-id",
				PatchsetOrder:              "1",
				TryjobID:                   "tryjob-id",
			},
			commandPath:          "/path/to/command",
			commandArgs:          []string{"--foo", "bar", "--baz", "qux"},
			commandWorkDir:       "/path/to/workdir",
			isGM:                 true,
			undeclaredOutputsDir: t.TempDir(),
		},
		goldctlWorkDir,
		"/path/to/goldctl imgtest init --work-dir "+goldctlWorkDir+" --instance skia --url https://gold.skia.org --bucket skia-infra-gm --git_hash ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99 --crs gerrit --cis buildbucket --changelist changelist-id --patchset 1 --jobid tryjob-id --key os:linux",
		[]any{
			"imgtest",
			"init",
			"--work-dir", goldctlWorkDir,
			"--instance", "skia",
			"--url", "https://gold.skia.org",
			"--bucket", "skia-infra-gm",
			"--git_hash", "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
			"--crs", "gerrit",
			"--cis", "buildbucket",
			"--changelist", "changelist-id",
			"--patchset", "1",
			"--jobid", "tryjob-id",
			"--key", "os:linux",
		},
	)
}
