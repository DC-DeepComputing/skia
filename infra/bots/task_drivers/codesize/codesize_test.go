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
	taskId := "CkPp9ElAaEXyYWNHpXHU"
	compileTaskName := "Build-Debian10-Clang-x86_64-Release"
	binaryName := "dm"
	expectedBloatyFileGCSPath := "2022/01/31/tryjob/12345/3/CkPp9ElAaEXyYWNHpXHU/Build-Debian10-Clang-x86_64-Release/dm.tsv"
	expectedBloatyFileContents := "I'm a fake Bloaty output!"

	// DO NOT SUBMIT before replacing it with a more realistic looking struct.
	repoState := types.RepoState{
		Patch: types.Patch{
			Issue:    "12345",
			Patchset: "3",
		},
	}

	commandCollector := exec.CommandCollector{}
	commandCollector.SetDelegateRun(func(ctx context.Context, cmd *exec.Command) error {
		if cmd.Name == "bloaty/bloaty" {
			cmd.CombinedOutput.Write([]byte(expectedBloatyFileContents))
		}
		return nil
	})

	mockGcsClient := test_gcsclient.NewMockClient()
	defer mockGcsClient.AssertExpectations(t)
	mockGcsClient.On(
		"SetFileContents",
		testutils.AnyContext,
		expectedBloatyFileGCSPath,
		gcs.FILE_WRITE_OPTS_TEXT,
		[]byte(expectedBloatyFileContents),
	).Return(nil)

	res := td.RunTestSteps(t, false, func(ctx context.Context) error {
		ctx = now.TimeTravelingContext(fakeNow).WithContext(ctx)
		ctx = td.WithExecRunFn(ctx, commandCollector.Run)

		return runSteps(ctx, repoState, mockGcsClient, taskId, compileTaskName, binaryName)
	})

	require.Empty(t, res.Errors)
	require.Empty(t, res.Exceptions)

	require.Len(t, commandCollector.Commands(), 2)

	lsCmd := commandCollector.Commands()[0]
	assert.Equal(t, "ls", lsCmd.Name)
	assert.Equal(t, []string{"build"}, lsCmd.Args)

	bloatyCmd := commandCollector.Commands()[1]
	assert.Equal(t, "bloaty/bloaty", bloatyCmd.Name)
	assert.Equal(t, []string{"build/dm", "-d", "compileunits,symbols", "-n", "0", "--tsv"}, bloatyCmd.Args)

	mockGcsClient.AssertNumberOfCalls(t, "SetFileContents", 1)
}
