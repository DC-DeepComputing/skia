// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package exporter

import (
	"bytes"
	"io"

	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/go/util"
	"go.skia.org/skia/bazel/exporter/build_proto/build"
)

// CMakeRule represents the CMake equivalent to a Bazel rule.
type CMakeRule struct {
	contents *bytes.Buffer // Data to be written to CMake file.
	deps     []string      // Names of Bazel targets (not files) on which this rule directly depends.
	rule     *build.Rule
}

// NewCMakeRule will create a new CMake rule object.
func NewCMakeRule(r *build.Rule) *CMakeRule {
	return &CMakeRule{rule: r}
}

// Return the rule name.
func (r *CMakeRule) getName() string {
	return r.rule.GetName()
}

// Does this rule contain source dependencies?
func (r *CMakeRule) hasSrcs() bool {
	srcs, _ := getRuleStringArrayAttribute(r.rule, "srcs")
	return len(srcs) > 0
}

// Does this rule depend directly on a rule.
func (r *CMakeRule) hasDependency(ruleName string) bool {
	return util.In(ruleName, r.deps)
}

// Add a rule dependency.
func (r *CMakeRule) addDependency(ruleName string) error {
	if ruleName == "" {
		return skerr.Fmt("empty rule name")
	}
	if ruleName == r.getName() {
		return skerr.Fmt("rule cannot depend on self: %s", ruleName)
	}
	if r.hasDependency(ruleName) {
		return nil
	}
	// Trusting that circular dependencies are already fixed by Bazel.
	r.deps = append(r.deps, ruleName)
	return nil
}

// setContents will set the supplied chunk of a CMake project file to this
// rules contents.
func (r *CMakeRule) setContents(contents *bytes.Buffer) {
	r.contents = contents
}

// Write the contents for this rule using the supplied writer.
// Returns the number of bytes written and error.
func (r *CMakeRule) write(writer io.Writer) (int, error) {
	if r.contents == nil {
		return 0, skerr.Fmt("rule %s has no contents", r.getName())
	}
	return writer.Write(r.contents.Bytes())
}
