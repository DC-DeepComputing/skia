// Copyright 2022 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package exporter

import (
	"bytes"
	"fmt"
	"io"
	"log"
	"os"
	"path/filepath"
	"strings"

	"github.com/golang/protobuf/proto"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/skia/bazel/exporter/build_proto/analysis_v2"
	"go.skia.org/skia/bazel/exporter/build_proto/build"
)

type CMakeExporter struct {
	qr           QueryReader
	workspace    *CMakeWorkspace
	rules        map[string]bool
	workspaceDir string
}

// NewCMakeExporter creates an exporter that will export to the
// CMake project file format.
func NewCMakeExporter(qr QueryReader, workspaceDir string) *CMakeExporter {
	return &CMakeExporter{
		qr:           qr,
		workspace:    NewCMakeWorkspace(),
		rules:        make(map[string]bool),
		workspaceDir: workspaceDir,
	}
}

func (e CMakeExporter) isFileRule(target string) bool {
	_, _, target, err := parseRule(target)
	if err != nil {
		return false
	}
	return strings.Contains(target, ".")
}

// Should a rule be skipped during the conversion?
func skipRule(r *build.Rule) bool {
	return strings.Contains(r.GetName(), "//third_party") ||
		strings.Contains(r.GetName(), "//tool") ||
		strings.Contains(r.GetName(), "//infra") ||
		strings.Contains(r.GetName(), "//bazel")
}

// Write the list of files to the supplied buffer.
func (e *CMakeExporter) writeItems(r *CMakeRule, projectDir string, files []string, buffer *bytes.Buffer) error {
	for _, file := range files {
		if file == "" {
			// TODO: root cause why Blaze seems to have empty items.
			continue
		}
		if e.isFileRule(file) {
			_, _, target, err := parseRule(file)
			if err != nil {
				return skerr.Wrap(err)
			}
			fullPath := filepath.Join(projectDir, target)
			if _, err = os.Stat(fullPath); err != nil {
				fmt.Printf("Doesn't exist: %s\n", fullPath)
				// TODO: These missing files are related to genrule. Need to fix.
				return nil // Will be an error at some point.
			}
			fmt.Fprintf(buffer, "    \"%s\"\n", fullPath)
			r.addFile(fullPath)
		} else {
			fmt.Fprintf(buffer, "    ${%s}\n", getRuleCMakeName(file))
			err := r.addDependency(file)
			if err != nil {
				return skerr.Wrap(err)
			}
		}
	}
	return nil
}

func (e *CMakeExporter) appendItems(rule *CMakeRule, buffer *bytes.Buffer, r *build.Rule, projectDir string) error {
	for _, attrib := range r.Attribute {
		if attrib.GetName() == "srcs" {
			if attrib.GetType() != build.Attribute_LABEL_LIST {
				return skerr.Fmt(`srcs in rule "%s" is not a list`, r.GetName())
			}
			fmt.Fprintln(buffer, "    # Sources:")
			err := e.writeItems(rule, projectDir, attrib.GetStringListValue(), buffer)
			if err != nil {
				return skerr.Wrap(err)
			}
		}
		if attrib.GetName() == "hdrs" {
			if attrib.GetType() != build.Attribute_LABEL_LIST {
				return skerr.Fmt(`hdrs in rule "%s" is not a list`, r.GetName())
			}
			fmt.Fprintln(buffer, "    # Headers:")
			err := e.writeItems(rule, projectDir, attrib.GetStringListValue(), buffer)
			if err != nil {
				return skerr.Wrap(err)
			}
		}
	}
	return nil
}

func (e *CMakeExporter) convertFilegroupRule(r *build.Rule) error {
	targetName := r.GetName()
	projectDir, err := getLocationDir(r.GetLocation())
	if err != nil {
		return skerr.Wrap(err)
	}

	rule := e.workspace.CreateRule(r.GetName(), r.GetRuleClass())

	var buffer bytes.Buffer

	variableName := getRuleCMakeName(targetName)
	fmt.Fprintf(&buffer, "# %s\n", targetName)
	fmt.Fprintf(&buffer, "list(APPEND %s\n", variableName)

	err = e.appendItems(rule, &buffer, r, projectDir)
	if err != nil {
		return skerr.Wrap(err)
	}
	fmt.Fprintln(&buffer, ")")

	rule.Append(buffer.Bytes())

	return nil
}

func (e *CMakeExporter) ignoreRule(r *build.Rule) error {
	return nil
}

func (e *CMakeExporter) convertCCBinaryRule(r *build.Rule) error {
	targetName := r.GetName()
	projectDir, err := getLocationDir(r.GetLocation())
	if err != nil {
		return skerr.Wrap(err)
	}

	rule := e.workspace.CreateRule(r.GetName(), r.GetRuleClass())

	var buffer bytes.Buffer
	fmt.Fprintf(&buffer, "# %s\n", targetName)
	fmt.Fprintf(&buffer, "add_executable(%s \"\")\n", getRuleCMakeName(targetName))
	fmt.Fprintf(&buffer, "target_sources(%s\n", getRuleCMakeName(targetName))
	fmt.Fprintln(&buffer, "  PRIVATE")

	err = e.appendItems(rule, &buffer, r, projectDir)
	if err != nil {
		return skerr.Wrap(err)
	}

	fmt.Fprintln(&buffer, ")")

	rule.Append(buffer.Bytes())

	return nil
}

func (e *CMakeExporter) appendCompileFlags(r *build.Rule, buffer *bytes.Buffer) error {
	for _, attrib := range r.Attribute {
		if attrib.GetName() == "copts" {
			if attrib.GetType() != build.Attribute_STRING_LIST {
				return skerr.Fmt(`copts in rule "%s" is not a list`, r.GetName())
			}
			props := strings.Join(attrib.GetStringListValue(), " ")
			fmt.Fprintf(buffer, "set_target_properties(%s PROPERTIES COMPILE_FLAGS \"%s\")\n", getRuleCMakeName(r.GetName()), props)
			return nil
		}
	}
	return nil
}

func (e *CMakeExporter) appendCompileDefinitions(r *build.Rule, buffer *bytes.Buffer) error {
	return nil
}

func (e *CMakeExporter) appendIncludeDirectories(r *build.Rule, buffer *bytes.Buffer) error {
	return nil
}

func (e *CMakeExporter) appendLinkFlags(r *build.Rule, buffer *bytes.Buffer) error {
	return nil
}

func (e *CMakeExporter) appendStuff(r *build.Rule, buffer *bytes.Buffer) error {
	err := e.appendCompileFlags(r, buffer)
	if err != nil {
		return skerr.Wrap(err)
	}
	err = e.appendLinkFlags(r, buffer)
	if err != nil {
		return skerr.Wrap(err)
	}
	err = e.appendCompileDefinitions(r, buffer)
	if err != nil {
		return skerr.Wrap(err)
	}
	err = e.appendIncludeDirectories(r, buffer)
	if err != nil {
		return skerr.Wrap(err)
	}
	return nil
}

func (e *CMakeExporter) convertCCLibraryRule(r *build.Rule) error {
	targetName := r.GetName()

	projectDir, err := getLocationDir(r.GetLocation())
	if err != nil {
		return skerr.Wrap(err)
	}

	rule := e.workspace.CreateRule(r.GetName(), r.GetRuleClass())

	var buffer bytes.Buffer
	fmt.Fprintf(&buffer, "# %s\n", targetName)
	fmt.Fprintf(&buffer, "add_library(%s \"\")\n", getRuleCMakeName(targetName))
	fmt.Fprintf(&buffer, "target_sources(%s\n", getRuleCMakeName(targetName))
	fmt.Fprintln(&buffer, "  PRIVATE")

	err = e.appendItems(rule, &buffer, r, projectDir)
	if err != nil {
		return skerr.Wrap(err)
	}
	fmt.Fprintln(&buffer, ")")
	err = e.appendStuff(r, &buffer)
	if err != nil {
		return skerr.Wrap(err)
	}

	rule.Append(buffer.Bytes())

	return nil
}

func (e *CMakeExporter) Export(writer io.Writer) error {

	fmt.Fprintln(writer, "# This file is auto-generated.")
	fmt.Fprintln(writer, "cmake_minimum_required(VERSION 3.13)")
	fmt.Fprintln(writer)
	fmt.Fprintln(writer, "project(skia LANGUAGES C CXX)")
	fmt.Fprintln(writer)

	in, err := e.qr.Read()
	if err != nil {
		log.Fatalln("Error reading proto data:", err)
	}
	qr := &analysis_v2.CqueryResult{}
	if err := proto.Unmarshal(in, qr); err != nil {
		log.Fatalln("Failed to unmarshal Bazel query result:", err)
		return skerr.Wrap(err)
	}

	for _, result := range qr.GetResults() {
		t := result.GetTarget()
		r := t.GetRule()
		if skipRule(r) {
			continue
		}
		e.rules[r.GetName()] = true
		var err error = nil
		switch {
		case r.GetRuleClass() == "":
			err = e.ignoreRule(r)
		case r.GetRuleClass() == "_gazelle_runner":
			err = e.ignoreRule(r)
		case r.GetRuleClass() == "_karma_test":
			err = e.ignoreRule(r)
		case r.GetRuleClass() == "alias":
			err = e.ignoreRule(r)
		case r.GetRuleClass() == "bool_flag":
			err = e.ignoreRule(r)
		case r.GetRuleClass() == "cc_binary":
			err = e.convertCCBinaryRule(r)
		case r.GetRuleClass() == "cc_library":
			err = e.convertCCLibraryRule(r)
		case r.GetRuleClass() == "cc_library_with_flags":
			err = e.ignoreRule(r)
		case r.GetRuleClass() == "config_setting":
			err = e.ignoreRule(r)
		case r.GetRuleClass() == "constraint_value":
			err = e.ignoreRule(r)
		case r.GetRuleClass() == "constraint_setting":
			err = e.ignoreRule(r)
		case r.GetRuleClass() == "filegroup":
			err = e.convertFilegroupRule(r)
		case r.GetRuleClass() == "genrule":
			err = e.ignoreRule(r)
		case r.GetRuleClass() == "go_binary":
			err = e.ignoreRule(r)
		case r.GetRuleClass() == "go_library":
			err = e.ignoreRule(r)
		case r.GetRuleClass() == "platform":
			err = e.ignoreRule(r)
		case r.GetRuleClass() == "py_binary":
			err = e.ignoreRule(r)
		case r.GetRuleClass() == "py_library":
			err = e.ignoreRule(r)
		case r.GetRuleClass() == "sh_binary":
			err = e.ignoreRule(r)
		case r.GetRuleClass() == "sh_test":
			err = e.ignoreRule(r)
		case r.GetRuleClass() == "transition_rule":
			err = e.ignoreRule(r)
		case r.GetRuleClass() == "wasm_cc_binary":
			err = e.ignoreRule(r)
		case r.GetRuleClass() == "web_test":
			err = e.ignoreRule(r)
		default:
			return skerr.Fmt("unsupported Bazel rule: \"%s\"\n", r.GetRuleClass())
		}
		if err != nil {
			return skerr.Wrapf(err, "Failed to convert %s", r.GetRuleClass())
		}
	}

	fmt.Printf("The Bazel query has %d targets.\n", len(e.rules))

	_, err = e.workspace.write(writer)
	if err != nil {
		return skerr.Wrap(err)
	}

	return nil
}

// Make sure CMakeExporter fulfills the Exporter interface.
var _ Exporter = (*CMakeExporter)(nil)
