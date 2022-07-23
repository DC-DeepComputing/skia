// Copyright 2022 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package exporter

import (
	"path/filepath"
	"regexp"
	"strconv"
	"strings"

	"go.skia.org/infra/go/skerr"
	"go.skia.org/skia/bazel/exporter/build_proto/build"
)

// Parse a rule into its constituent parts.
// https://docs.bazel.build/versions/main/guide.html#specifying-targets-to-build
func parseRule(rule string) (repo string, path string, target string, err error) {

	re, err := regexp.Compile(`^(@[^/]+)?/([^:]+):(.+)$`)
	if err != nil {
		err = skerr.Wrap(err)
		return
	}

	parts := re.FindStringSubmatch(rule)
	if parts == nil {
		err = skerr.Fmt(`Unable to match rule "%s"`, rule)
		return
	}

	repo = parts[1]
	path = parts[2]
	target = parts[3]
	return
}

// Parse a file location into its three constituent parts.
//
// A location is of the form:
//
//	/full/path/to/BUILD.bazel:33:20
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

// Return the directory containing the file in the location string.
func getLocationDir(location string) (string, error) {
	filePath, _, _, err := parseLocation(location)
	if err != nil {
		return "", skerr.Wrap(err)
	}
	return filepath.Dir(filePath), nil
}

// Create a string that uniquely identifies the Rule and can be used
// in the exported CMake file.
func getRuleCMakeName(bazelRuleName string) string {
	s := strings.TrimPrefix(bazelRuleName, "//")
	s = strings.ReplaceAll(s, "@", "at_")
	s = strings.ReplaceAll(s, "/", "_")
	s = strings.ReplaceAll(s, ":", "_")
	return s
}

func stringSliceContains(elems []string, v string) bool {
	// Should use generics (but needs Go v1.18).
	for _, s := range elems {
		if v == s {
			return true
		}
	}
	return false
}

func getRuleStringArrayAttribute(r *build.Rule, name string) ([]string, error) {
	for _, attrib := range r.Attribute {
		if attrib.GetName() != name {
			continue
		}
		if attrib.GetType() != build.Attribute_LABEL_LIST &&
			attrib.GetType() != build.Attribute_STRING_LIST {
			return []string{}, skerr.Fmt(`%s in rule "%s" is not a list`, name, r.GetName())
		}
		return attrib.GetStringListValue(), nil
	}
	return []string{}, nil
}
