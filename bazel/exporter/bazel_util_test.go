// Copyright 2022 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package exporter

import (
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

func TestParseRule_ValidRules_Success(t *testing.T) {

	test := func(name string, rule string, expectedRepo string, expectedPath string, expectedTarget string) {
		t.Run(name, func(t *testing.T) {
			repo, path, target, err := parseRule(rule)
			require.NoError(t, err)
			assert.Equal(t, expectedRepo, repo)
			assert.Equal(t, expectedPath, path)
			assert.Equal(t, expectedTarget, target)
		})
	}

	test("PathWithTarget", "//foo/bar:wiz", "", "/foo/bar", "wiz")
	test("PathWithTarget", "@abseil_cpp//absl/algorithm:algorithm.h", "@abseil_cpp", "/absl/algorithm", "algorithm.h")
}

func TestParseLocation_ValidInput_Success(t *testing.T) {
	path, line, pos, err := parseLocation("/path/to/file.txt:12:875")
	require.NoError(t, err)
	assert.Equal(t, "/path/to/file.txt", path)
	assert.Equal(t, 12, line)
	assert.Equal(t, 875, pos)
}

func TestGetLocationDir_ValidInput_Success(t *testing.T) {
	path, err := getLocationDir("/path/to/file.txt:12:875")
	require.NoError(t, err)
	assert.Equal(t, "/path/to", path)
}

func TestGetRuleCMakeName_ValidInput_Success(t *testing.T) {
	test := func(name string, rule string, expectedName string) {
		t.Run(name, func(t *testing.T) {
			assert.Equal(t, expectedName, getRuleCMakeName(rule))
		})
	}

	test("PathWithTarget", "//include/private/chromium:private_hdrs", "include_private_chromium_private_hdrs")
	test("PathWithTarget", "@repo//path/to/dir:file.txt", "at_repo__path_to_dir_file.txt")
}
