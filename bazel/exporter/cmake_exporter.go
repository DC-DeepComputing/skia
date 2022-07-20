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

	"github.com/bazelbuild/buildtools/build_proto"
	"github.com/golang/protobuf/proto"
	"go.skia.org/infra/go/skerr"
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
func skipRule(r *build_proto.Rule) bool {
	return strings.Contains(r.GetName(), "//third_party") ||
		strings.Contains(r.GetName(), "//tool") ||
		strings.Contains(r.GetName(), "//infra") ||
		strings.Contains(r.GetName(), "//bazel")
}

// Write the list of files to the supplied buffer.
func (e *CMakeExporter) writeItems(r *CMakeRule, projectDir string, files []string, buffer *bytes.Buffer) error {
	for _, file := range files {
		if file == "" {
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
			buffer.WriteString(fmt.Sprintf("    \"%s\"\n", fullPath))
			r.addFile(fullPath)
		} else {
			buffer.WriteString(fmt.Sprintf("    ${%s}\n", getRuleCMakeName(file)))
			err := r.addDependency(file)
			if err != nil {
				return skerr.Wrap(err)
			}
		}
	}
	return nil
}

func (e *CMakeExporter) appendItems(rule *CMakeRule, buffer *bytes.Buffer, r *build_proto.Rule, projectDir string) error {
	srcs := []string{}
	hdrs := []string{}
	for _, attrib := range r.Attribute {
		if attrib.GetName() == "srcs" && attrib.GetType() == build_proto.Attribute_LABEL_LIST {
			srcs = append(srcs, attrib.GetStringListValue()...)
		}
		if attrib.GetName() == "hdrs" && attrib.GetType() == build_proto.Attribute_LABEL_LIST {
			hdrs = append(hdrs, attrib.GetStringListValue()...)
		}
	}
	if len(srcs) > 0 {
		buffer.WriteString("    # Sources:\n")
		err := e.writeItems(rule, projectDir, srcs, buffer)
		if err != nil {
			return skerr.Wrap(err)
		}
	}
	if len(hdrs) > 0 {
		buffer.WriteString("    # Headers:\n")
		err := e.writeItems(rule, projectDir, hdrs, buffer)
		if err != nil {
			return skerr.Wrap(err)
		}
	}
	return nil
}

func (e *CMakeExporter) convertFilegroupRule(r *build_proto.Rule) error {
	targetName := r.GetName()
	projectDir, err := getLocationDir(r.GetLocation())
	if err != nil {
		return skerr.Wrap(err)
	}

	rule := e.workspace.CreateRule(r.GetName(), r.GetRuleClass())

	var buffer bytes.Buffer

	variableName := getRuleCMakeName(targetName)
	buffer.WriteString(fmt.Sprintf("# %s\n", targetName))
	buffer.WriteString(fmt.Sprintf("list(APPEND %s\n", variableName))

	err = e.appendItems(rule, &buffer, r, projectDir)
	if err != nil {
		return skerr.Wrap(err)
	}
	buffer.WriteString(")\n")

	rule.Append(buffer.Bytes())

	return nil
}

func (e *CMakeExporter) ignoreRule(r *build_proto.Rule) error {
	return nil
}

func (e *CMakeExporter) convertCCBinaryRule(r *build_proto.Rule) error {
	targetName := r.GetName()
	projectDir, err := getLocationDir(r.GetLocation())
	if err != nil {
		return skerr.Wrap(err)
	}

	rule := e.workspace.CreateRule(r.GetName(), r.GetRuleClass())

	var buffer bytes.Buffer
	buffer.WriteString(fmt.Sprintf("# %s\n", targetName))
	buffer.WriteString(fmt.Sprintf("add_executable(%s \"\")\n", getRuleCMakeName(targetName)))
	buffer.WriteString(fmt.Sprintf("target_sources(%s\n", getRuleCMakeName(targetName)))
	buffer.WriteString("  PRIVATE\n")

	err = e.appendItems(rule, &buffer, r, projectDir)
	if err != nil {
		return skerr.Wrap(err)
	}

	buffer.WriteString(")\n")

	rule.Append(buffer.Bytes())

	return nil
}

func (e *CMakeExporter) convertCCLibraryRule(r *build_proto.Rule) error {
	targetName := r.GetName()

	projectDir, err := getLocationDir(r.GetLocation())
	if err != nil {
		return skerr.Wrap(err)
	}

	rule := e.workspace.CreateRule(r.GetName(), r.GetRuleClass())

	var buffer bytes.Buffer
	buffer.WriteString(fmt.Sprintf("# %s\n", targetName))
	buffer.WriteString(fmt.Sprintf("add_library(%s \"\")\n", getRuleCMakeName(targetName)))
	buffer.WriteString(fmt.Sprintf("target_sources(%s\n", getRuleCMakeName(targetName)))
	buffer.WriteString("  PRIVATE\n")

	err = e.appendItems(rule, &buffer, r, projectDir)
	if err != nil {
		return skerr.Wrap(err)
	}
	buffer.WriteString(")\n")

	rule.Append(buffer.Bytes())

	return nil
}

func (e *CMakeExporter) Export(writer io.Writer) error {

	fmt.Fprintln(writer, "# This file is auto-generated.")
	fmt.Fprintln(writer, "cmake_minimum_required(VERSION 3.13)")
	fmt.Fprintln(writer)
	fmt.Fprintln(writer, "project(skia)")
	fmt.Fprintln(writer)

	in, err := e.qr.Read()
	if err != nil {
		log.Fatalln("Error reading proto data:", err)
	}
	qr := &build_proto.QueryResult{}
	if err := proto.Unmarshal(in, qr); err != nil {
		log.Fatalln("Failed to unmarshal Bazel query result:", err)
		return skerr.Wrap(err)
	}

	for _, t := range qr.Target {
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
