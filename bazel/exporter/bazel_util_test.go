// Copyright 2022 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package exporter

import (
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
	"go.skia.org/skia/bazel/exporter/build_proto/build"
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
	test("PathWithHost", "@repo//path/to/dir:file.txt", "at_repo_path_to_dir_file.txt")
	test("RootTarget", "//:skia_public", "skia_public")
	test("HostTarget", "@repo//:deps", "at_repo_deps")
}

func TestGetRuleStringArrayAttribute_NoAttrib_ReturnsError(t *testing.T) {
	var rule build.Rule
	vals, err := getRuleStringArrayAttribute(&rule, "missing-attrib")
	assert.NoError(t, err)
	assert.Equal(t, []string{}, vals)
}

func TestStringSliceContains_InSlice_ReturnsTrue(t *testing.T) {
	test := func(name string, slice []string, val string) {
		t.Run(name, func(t *testing.T) {
			assert.True(t, stringSliceContains(slice, val))
		})
	}

	test("EmptyString", []string{""}, "")
	test("OneValue", []string{"car"}, "car")
	test("FirstValue", []string{"car", "parts"}, "car")
	test("LastValue", []string{"car", "parts"}, "parts")
}

func TestStringSliceContains_NotInSlice_ReturnsFalse(t *testing.T) {
	test := func(name string, slice []string, val string) {
		t.Run(name, func(t *testing.T) {
			assert.False(t, stringSliceContains(slice, val))
		})
	}

	test("EmptySlice", []string{}, "not-in-list")
	test("OneValue", []string{"car"}, "not-in-list")
	test("MultipleValues", []string{"car", "parts"}, "not-in-list")
	test("Substring", []string{"Hello"}, "Hel")
	test("DifferByCase", []string{"Hello"}, "hello")
}

func TestAppendUnique_NotPresent_Appended(t *testing.T) {

	slice := appendUnique([]string{"one"}, "two")
	assert.Equal(t, []string{"one", "two"}, slice)
}

func TestAppendUnique_Present_NotAppended(t *testing.T) {

	slice := appendUnique([]string{"one", "two"}, "two")
	assert.Equal(t, []string{"one", "two"}, slice)
}

func TestIsFileRule_InvalidRule_ReturnsError(t *testing.T) {
	assert.False(t, isFileTarget(""))
}

func TestIsFileRule_ValidFileRule_Success(t *testing.T) {
	assert.True(t, isFileTarget("//dir/path:hello.c"))
}

func TestIsFileRule_ValidNonFileRule_Success(t *testing.T) {
	assert.False(t, isFileTarget("//dir/path:hello"))
}
