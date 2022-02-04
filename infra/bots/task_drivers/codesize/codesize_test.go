// Copyright 2022 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"context"
	"testing"
	"time"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
	"go.skia.org/infra/go/exec"
	"go.skia.org/infra/go/gcs"
	"go.skia.org/infra/go/gcs/test_gcsclient"
	"go.skia.org/infra/go/now"
	"go.skia.org/infra/go/testutils"
	"go.skia.org/infra/task_driver/go/td"
	"go.skia.org/infra/task_scheduler/go/types"
)

func TestRunSteps_Tryjob_Success(t *testing.T) {
	fakeNow := time.Date(2022, time.January, 31, 0, 0, 0, 0, time.UTC)
	swarmingTaskId := "58dccb0d6a3f0411"
	swarmingServer := "https://chromium-swarm.appspot.com"
	taskId := "CkPp9ElAaEXyYWNHpXHU"
	taskName := "CodeSize-dm-Debian10-Clang-x86_64-Release"
	compileTaskName := "Build-Debian10-Clang-x86_64-Release"
	binaryName := "dm"
	bloatyCipdVersion := "1"

	expectedBloatyFileGCSPath := "2022/01/31/tryjob/12345/3/CkPp9ElAaEXyYWNHpXHU/Build-Debian10-Clang-x86_64-Release/dm.tsv"
	expectedBloatyFileContents := "I'm a fake Bloaty output!"

	expectedJSONMetadataFileGCSPath := "2022/01/31/tryjob/12345/3/CkPp9ElAaEXyYWNHpXHU/Build-Debian10-Clang-x86_64-Release/dm.json"
	expectedJSONMetadataFileContents := `{
  "swarming_task_id": "58dccb0d6a3f0411",
  "swarming_server": "https://chromium-swarm.appspot.com",
  "task_id": "CkPp9ElAaEXyYWNHpXHU",
  "task_name": "CodeSize-dm-Debian10-Clang-x86_64-Release",
  "compile_task_name": "Build-Debian10-Clang-x86_64-Release",
  "binary_name": "dm",
  "bloaty_cipd_version": "1",
  "bloaty_args": [
    "build/dm",
    "-d",
    "compileunits,symbols",
    "-n",
    "0",
    "--tsv"
  ],
  "patch_issue": "12345",
  "patch_server": "https://skia-review.googlesource.com",
  "patch_set": "3",
  "repo": "https://skia.googlesource.com/skia.git",
  "revision": "9049e12a273435d1fd53ca7a223607599ff3b46c"
}`

	repoState := types.RepoState{
		Patch: types.Patch{
			Issue:     "12345",
			PatchRepo: "https://skia.googlesource.com/skia.git",
			Patchset:  "3",
			Server:    "https://skia-review.googlesource.com",
		},
		Repo:     "https://skia.googlesource.com/skia.git",
		Revision: "9049e12a273435d1fd53ca7a223607599ff3b46c",
	}

	// Mock Bloaty's output.
	commandCollector := exec.CommandCollector{}
	commandCollector.SetDelegateRun(func(ctx context.Context, cmd *exec.Command) error {
		if cmd.Name == "bloaty/bloaty" {
			cmd.CombinedOutput.Write([]byte(expectedBloatyFileContents))
		}
		return nil
	})

	mockGcsClient := test_gcsclient.NewMockClient()
	defer mockGcsClient.AssertExpectations(t)

	// Mock the GCS client call to upload the Bloaty output.
	mockGcsClient.On(
		"SetFileContents",
		testutils.AnyContext,
		expectedBloatyFileGCSPath,
		gcs.FILE_WRITE_OPTS_TEXT,
		[]byte(expectedBloatyFileContents),
	).Return(nil)

	// Mock the GCS client call to upload the JSON metadata file.
	mockGcsClient.On(
		"SetFileContents",
		testutils.AnyContext,
		expectedJSONMetadataFileGCSPath,
		gcs.FILE_WRITE_OPTS_TEXT,
		[]byte(expectedJSONMetadataFileContents),
	).Return(nil)

	res := td.RunTestSteps(t, false, func(ctx context.Context) error {
		ctx = now.TimeTravelingContext(fakeNow).WithContext(ctx)
		ctx = td.WithExecRunFn(ctx, commandCollector.Run)

		return runSteps(ctx, repoState, mockGcsClient, swarmingTaskId, swarmingServer, taskId, taskName, compileTaskName, binaryName, bloatyCipdVersion)
	})

	require.Empty(t, res.Errors)
	require.Empty(t, res.Exceptions)

	require.Len(t, commandCollector.Commands(), 2)

	// Assert that "ls build" was executed to list the contents of the directory with the binaries
	// built by the compile task, for debugging purposes.
	lsCmd := commandCollector.Commands()[0]
	assert.Equal(t, "ls", lsCmd.Name)
	assert.Equal(t, []string{"build"}, lsCmd.Args)

	// Assert that Bloaty was invoked with the expected arguments.
	bloatyCmd := commandCollector.Commands()[1]
	assert.Equal(t, "bloaty/bloaty", bloatyCmd.Name)
	assert.Equal(t, []string{"build/dm", "-d", "compileunits,symbols", "-n", "0", "--tsv"}, bloatyCmd.Args)

	// Assert that two files were uploaded to GCS.
	mockGcsClient.AssertNumberOfCalls(t, "SetFileContents", 2)
}
