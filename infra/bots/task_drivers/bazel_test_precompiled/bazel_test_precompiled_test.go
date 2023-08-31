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
	"go.skia.org/infra/task_driver/go/td"
	"go.skia.org/skia/infra/bots/task_drivers/testutils"
)

func TestRun_Success(t *testing.T) {
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
