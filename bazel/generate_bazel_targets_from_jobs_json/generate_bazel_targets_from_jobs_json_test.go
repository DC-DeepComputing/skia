// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

func TestGenerateBazelTargetsFromJobsJSONFileContents_Success(t *testing.T) {
	jobsJSON := `[
		{"name": "BazelBuildGeneratedTarget-AndroidUnitTest-pixel5_codec_test",
			"cq_config": {"experimental": true}
		},
		{"name": "BazelTestGeneratedTarget-AndroidUnitTest-pixel5_codec_test",
			"cq_config": {"experimental": true}
		},
		{"name": "BazelBuildGeneratedTarget-AndroidUnitTest-pixel7_pathops_debug_test"},
		{"name": "BazelTestGeneratedTarget-AndroidUnitTest-pixel7_pathops_debug_test"},
		{"name": "BazelBuildGeneratedTarget-AndroidUnitTest-pixel7_ganesh_vulkan_release_test"},
		{"name": "BazelTestGeneratedTarget-AndroidUnitTest-pixel7_ganesh_vulkan_release_test"}
	]
	`

	bzl, err := generateBazelTargetsFromJobsJSONFileContents([]byte(jobsJSON))
	require.NoError(t, err)
	assert.Equal(t, `"""GENERATED FILE, DO NOT MODIFY."""

load(
    ":android.bzl",
    "CODEC",
    "DEBUG",
    "GANESH",
    "PATHOPS",
    "PIXEL_5",
    "PIXEL_7",
    "RELEASE",
    "VULKAN",
    "skia_android_unit_test",
)

def generated_tests(name):
    """Produces a set of Bazel targets generated from CI task names in //infra/bots/jobs.json.

    Args:
        name: Unused, but required to appease Buildifier.
    """

    skia_android_unit_test(
        name = "pixel5_codec_test",
        device = PIXEL_5,
        test_group = CODEC,
    )

    skia_android_unit_test(
        name = "pixel7_ganesh_vulkan_release_test",
        device = PIXEL_7,
        test_group = GANESH,
        flavors = [
            RELEASE,
            VULKAN,
        ],
    )

    skia_android_unit_test(
        name = "pixel7_pathops_debug_test",
        device = PIXEL_7,
        test_group = PATHOPS,
        flavors = [
            DEBUG,
        ],
    )
`, bzl)
}
