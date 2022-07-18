// Copyright 2022 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package exporter

import (
	"path/filepath"
	"regexp"
	"strconv"

	"go.skia.org/infra/go/skerr"
)

// Parse a rule into its constituent parts.
// https://docs.bazel.build/versions/main/guide.html#specifying-targets-to-build
func parseRule(rule string) (path string, target string, err error) {

	re, err := regexp.Compile(`^/([^:]+):(.+)$`)
	if err != nil {
		err = skerr.Wrap(err)
		return
	}

	parts := re.FindStringSubmatch(rule)
	if parts == nil {
		err = skerr.Fmt(`Unable to match rule "%s"`, rule)
		return
	}

	path = parts[1]
	target = parts[2]
	return
}

// Parse a file location:
// /full/path/to/BUILD.bazel:33:20
func parseLocation(location string) (path string, line int, pos int, err error) {
	re, err := regexp.Compile(`^([^:]+):([^:]+):([^:]+)$`)
	if err != nil {
		err = skerr.Wrap(err)
		return
	}
	parts := re.FindStringSubmatch(location)
	if parts == nil {
		err = skerr.Fmt(`Unable to match file location "%s"`, location)
		return
	}
	path = parts[1]
	line, err = strconv.Atoi(parts[2])
	if err != nil {
		err = skerr.Fmt(`Unable to parse line no. "%s"`, parts[2])
		return
	}
	pos, err = strconv.Atoi(parts[3])
	if err != nil {
		err = skerr.Fmt(`Unable to parse pos. "%s"`, parts[3])
		return
	}
	return
}

func getLocationDir(location string) (string, error) {
	filePath, _, _, err := parseLocation(location)
	if err != nil {
		return "", skerr.Wrap(err)
	}
	return filepath.Dir(filePath), nil
}
