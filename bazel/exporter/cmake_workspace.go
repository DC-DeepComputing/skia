// Copyright 2022 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package exporter

import (
	"fmt"
	"io"

	"go.skia.org/infra/go/skerr"
)

// CMakeWorkspace represents the entire state of a set of CMake project files
// that comprise a project.
type CMakeWorkspace struct {
	// Map Bazel rule names to CMakeRule istances.
	rules        map[string]*CMakeRule
	ruleList     []*CMakeRule
	writtenRules []string // All CMakeRule's written during write.
}

// NewCMakeWorkspace will create a new CMake workspace object.
func NewCMakeWorkspace() *CMakeWorkspace {
	return &CMakeWorkspace{rules: make(map[string]*CMakeRule)}
}

func (w *CMakeWorkspace) isRuleWritten(ruleName string) bool {
	return stringSliceContains(w.writtenRules, ruleName)
}

// Return all rule names that directly depend on ruleName.
func (w *CMakeWorkspace) getDependencies(ruleName string) []string {
	deps := []string{}

	for _, r := range w.rules {
		if r.hasDependency(ruleName) {
			deps = append(deps, r.name)
		}
	}

	return deps
}

// GetProject will return the CMake project for the corresponding directory path.
// Will return nil if there is no corresponding project.
func (w *CMakeWorkspace) GetRule(name string) *CMakeRule {
	return w.rules[name]
}

// CreateRule will create (if necessary) a new CMake rule object
// for the given rule name.
func (w *CMakeWorkspace) CreateRule(name string, class string) *CMakeRule {
	r := w.GetRule(name)
	if r != nil {
		return r
	}
	r = NewCMakeRule(name, class)
	w.rules[name] = r
	w.ruleList = append(w.ruleList, r)
	return r
}

func (w *CMakeWorkspace) skipRuleWrite(r *CMakeRule) bool {
	if r.isEmpty() && len(w.getDependencies(r.name)) == 0 {
		// TODO: This is temporary until some dependencies are properly converted.
		return true
	}
	if r.isHeaderOnlyLibrary() {
		// There are Bazel libraries that only contain headers - skip these.
		return true
	}

	return false
}

func (w *CMakeWorkspace) writeRule(writer io.Writer, r *CMakeRule) (int, error) {
	nb := 0
	if w.skipRuleWrite(r) {
		return 0, nil
	}
	// First write all dependencies.
	for _, name := range r.deps {
		dep := w.GetRule(name)
		if dep == nil {
			return 0, skerr.Fmt(`cannot find rule "%s"`, name)
		}
		n, err := w.writeRule(writer, dep)
		if err != nil {
			return nb, skerr.Wrap(err)
		}
		nb += n
	}
	if w.isRuleWritten(r.name) {
		return nb, nil
	}
	num, err := fmt.Fprintln(writer)
	if err != nil {
		return nb, skerr.Wrap(err)
	}
	nb += num
	num, err = r.write(writer)
	if err != nil {
		return nb, skerr.Wrap(err)
	}
	w.writtenRules = append(w.writtenRules, r.name)
	nb += num
	return nb, nil
}

func (w *CMakeWorkspace) write(writer io.Writer) (int, error) {
	w.writtenRules = []string{}
	nb := 0
	for _, r := range w.ruleList {
		num, err := w.writeRule(writer, r)
		if err != nil {
			return nb, skerr.Wrap(err)
		}
		nb += num
	}
	return nb, nil
}
