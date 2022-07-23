// Copyright 2022 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package exporter

import (
	"bytes"
	"io"

	"go.skia.org/infra/go/skerr"
	"go.skia.org/skia/bazel/exporter/build_proto/build"
)

// CMakeRule represents the CMake equivalent to a Bazel rule.
type CMakeRule struct {
	contents bytes.Buffer // Data to be written to CMake file.
	deps     []string     // Names of Bazel targets (not files) on which this rule directly depends.
	rule     *build.Rule
}

// NewCMakeRule will create a new CMake rule object.
func NewCMakeRule(r *build.Rule) *CMakeRule {
	return &CMakeRule{rule: r}
}

func (r *CMakeRule) GetName() string {
	return r.rule.GetName()
}

func (r *CMakeRule) hasSrcs() bool {
	srcs, _ := getRuleStringArrayAttribute(r.rule, "srcs")
	return len(srcs) > 0
}

func (r *CMakeRule) hasDependency(ruleName string) bool {
	return stringSliceContains(r.deps, ruleName)
}

func (r *CMakeRule) addDependency(ruleName string) error {
	if ruleName == "" {
		return skerr.Fmt("empty rule name")
	}
	if ruleName == r.GetName() {
		return skerr.Fmt("rule cannot depend on self: %s", ruleName)
	}
	if r.hasDependency(ruleName) {
		return skerr.Fmt("rule dependency already exists: %s", ruleName)
	}
	// Trusting that circular dependencies are already fixed by Bazel.
	r.deps = append(r.deps, ruleName)
	return nil
}

// Append will append the supplied chunk of a CMake project file to this
// rules contents.
func (r *CMakeRule) Append(data []byte) {
	if r.contents.Len() != 0 {
		r.contents.WriteString("\n\n")
	}
	r.contents.Write(data)
}

func (r *CMakeRule) write(writer io.Writer) (int, error) {
	return writer.Write(r.contents.Bytes())
}
