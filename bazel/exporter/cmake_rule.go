// Copyright 2022 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package exporter

import (
	"bytes"
	"io"
	"path/filepath"
	"strings"

	"go.skia.org/infra/go/skerr"
)

// CMakeRule represents the CMake equivalent to a Bazel rule.
type CMakeRule struct {
	name     string       // The rule name as it appeared in the Bazel file.
	class    string       // The Bazel rule class (cc_library, etc.).
	contents bytes.Buffer // The contents of the CMake project file.
	deps     []string     // Bazel rule names of upstream dependencies.
	files    []string     // The files referenced by this rule.
}

// NewCMakeRule will create a new CMake rule object.
func NewCMakeRule(name string, class string) *CMakeRule {
	return &CMakeRule{name: name, class: class}
}

func isCxxSourceFile(fname string) bool {
	ext := strings.ToLower(filepath.Ext(fname))
	return ext == ".c" || ext == ".cc" || ext == ".cpp"
}

func (r *CMakeRule) hasDependency(ruleName string) bool {
	return stringSliceContains(r.deps, ruleName)
}

func (r *CMakeRule) isEmpty() bool {
	return len(r.files) == 0
}

func (r *CMakeRule) isHeaderOnlyLibrary() bool {
	if r.class != "cc_library" {
		return false
	}
	for _, f := range r.files {
		if isCxxSourceFile(f) {
			return false
		}
	}
	return true
}

func (r *CMakeRule) addFile(filePath string) {
	r.files = append(r.files, filePath)
}

func (r *CMakeRule) addDependency(ruleName string) error {
	if ruleName == "" {
		return skerr.Fmt("empty rule name")
	}
	if ruleName == r.name {
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
