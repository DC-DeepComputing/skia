// Copyright 2022 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package exporter

import (
	"fmt"
	"io"

	"go.skia.org/infra/go/skerr"
	"go.skia.org/skia/bazel/exporter/build_proto/build"
)

// CMakeWorkspace represents the entire state of a CMake project.
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

// Determine if a rule has been written.
func (w *CMakeWorkspace) isRuleWritten(ruleName string) bool {
	return stringSliceContains(w.writtenRules, ruleName)
}

// GetRule will return the CMake wrapper for a Bazel rule given the
// rule name. Will return nil if there is no corresponding rule.
func (w *CMakeWorkspace) getRule(name string) *CMakeRule {
	return w.rules[name]
}

// CreateRule will create (if necessary) a new CMake rule object
// for the given rule name.
func (w *CMakeWorkspace) createRule(rule *build.Rule) *CMakeRule {
	r := w.getRule(rule.GetName())
	if r != nil {
		return r
	}
	r = NewCMakeRule(rule)
	w.rules[rule.GetName()] = r
	w.ruleList = append(w.ruleList, r)
	return r
}

// Write the given rule to the writer.
// Will first write all dependent rules so that they appear
// in the CMake project file before the rule.
func (w *CMakeWorkspace) writeRule(writer io.Writer, r *CMakeRule) (int, error) {
	nb := 0
	if !r.hasSrcs() {
		return 0, nil
	}
	// First write all dependencies.
	for _, name := range r.deps {
		dep := w.getRule(name)
		if dep == nil {
			return 0, skerr.Fmt(`cannot find rule "%s"`, name)
		}
		n, err := w.writeRule(writer, dep)
		if err != nil {
			return nb, skerr.Wrap(err)
		}
		nb += n
	}
	if w.isRuleWritten(r.getName()) {
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
	w.writtenRules = append(w.writtenRules, r.getName())
	nb += num
	return nb, nil
}

// Write this workspace using the given writer.
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
