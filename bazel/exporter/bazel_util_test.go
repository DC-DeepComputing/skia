// Copyright 2022 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package exporter

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestParseRule_ValidRules_Success(t *testing.T) {

	test := func(name string, rule string, expectedPath string, expectedTarget string) {
		t.Run(name, func(t *testing.T) {
			path, target, err := parseRule(rule)
			assert.NoError(t, err)
			assert.Equal(t, expectedPath, path)
			assert.Equal(t, expectedTarget, target)
		})
	}

	test("PathWithTarget", "//foo/bar:wiz", "/foo/bar", "wiz")
}

func TestParseLocation_ValidInput_Success(t *testing.T) {
	path, line, pos, err := parseLocation("/path/to/file.txt:12:875")
	assert.NoError(t, err)
	assert.Equal(t, "/path/to/file.txt", path)
	assert.Equal(t, 12, line)
	assert.Equal(t, 875, pos)
}

func TestGetLocationDir_ValidInput_Success(t *testing.T) {
	path, err := getLocationDir("/path/to/file.txt:12:875")
	assert.NoError(t, err)
	assert.Equal(t, "/path/to", path)
}
