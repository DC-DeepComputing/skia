// Copyright 2022 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package exporter

import (
	"bytes"
	"fmt"
	"log"
	"strings"

	"github.com/bazelbuild/buildtools/build_proto"
	"github.com/golang/protobuf/proto"
	"go.skia.org/infra/go/skerr"
)

type CMakeExporter struct {
	qr        QueryReader
	workspace *CMakeWorkspace
	rules     map[string]bool
}

// NewCMakeExporter creates an exporter that will export to the
// CMake project file format.
func NewCMakeExporter(qr QueryReader) *CMakeExporter {
	return &CMakeExporter{
		qr:        qr,
		workspace: NewCMakeWorkspace(),
		rules:     make(map[string]bool),
	}
}

func (e CMakeExporter) isRule(target string) bool {
	_, ok := e.rules[target]
	return ok
}

// Should a rule be skipped during the conversion?
func skipRule(r *build_proto.Rule) bool {
	return strings.Contains(r.GetName(), "//third_party") ||
		strings.Contains(r.GetName(), "//tool") ||
		strings.Contains(r.GetName(), "//infra") ||
		strings.Contains(r.GetName(), "//bazel")
}

func (e *CMakeExporter) appendFiles(files []string, buffer *bytes.Buffer) error {
	for _, file := range files {
		if e.isRule(file) {
			buffer.WriteString(fmt.Sprintf("    %s\n", file))
		} else {
			_, target, err := parseRule(file)
			if err != nil {
				return skerr.Wrap(err)
			}
			buffer.WriteString(fmt.Sprintf("    %s\n", target))
		}
	}
	return nil
}

func (e *CMakeExporter) convertFilegroupRule(r *build_proto.Rule) error {
	targetName := r.GetName()

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

	var buffer bytes.Buffer
	buffer.WriteString(fmt.Sprintf("source_group(%s\n", targetName))

	if len(srcs) > 0 {
		buffer.WriteString("    # Sources:\n")
		err := e.appendFiles(srcs, &buffer)
		if err != nil {
			return skerr.Wrap(err)
		}
	}
	if len(hdrs) > 0 {
		buffer.WriteString("    # Headers:")
		err := e.appendFiles(hdrs, &buffer)
		if err != nil {
			return skerr.Wrap(err)
		}
	}
	buffer.WriteString(")")

	dirPath, err := getLocationDir(r.GetLocation())
	if err != nil {
		return skerr.Wrap(err)
	}
	proj := e.workspace.CreateProject(dirPath)
	proj.Append(buffer.Bytes())

	return nil
}

func (e *CMakeExporter) convertCCLibraryRule(r *build_proto.Rule) error {
	targetName := r.GetName()

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

	var buffer bytes.Buffer
	buffer.WriteString(fmt.Sprintf("add_library(%s, \"\")\n", targetName))
	buffer.WriteString(fmt.Sprintf("target_sources(%s\n", targetName))
	buffer.WriteString("  PRIVATE\n")

	if len(srcs) > 0 {
		buffer.WriteString("    # Sources:\n")
		err := e.appendFiles(srcs, &buffer)
		if err != nil {
			return skerr.Wrap(err)
		}
	}
	if len(hdrs) > 0 {
		buffer.WriteString("    # Headers:\n")
		err := e.appendFiles(hdrs, &buffer)
		if err != nil {
			return skerr.Wrap(err)
		}
	}
	buffer.WriteString(")")

	dirPath, err := getLocationDir(r.GetLocation())
	if err != nil {
		return skerr.Wrap(err)
	}
	proj := e.workspace.CreateProject(dirPath)
	proj.Append(buffer.Bytes())

	return nil
}

func (e *CMakeExporter) printWorkspace() {
	for dir, workspace := range e.workspace.dirProjects {
		fmt.Printf("Dir: %s\n", dir)
		fmt.Println(workspace.GetString())
		fmt.Println()
	}
}

func (e *CMakeExporter) Export() error {

	in, err := e.qr.Read()
	if err != nil {
		log.Fatalln("Error reading pb data:", err)
	}
	qr := &build_proto.QueryResult{}
	if err := proto.Unmarshal(in, qr); err != nil {
		log.Fatalln("Failed to parse Bazel query results:", err)
	}

	for _, t := range qr.Target {
		r := t.GetRule()
		if skipRule(r) {
			continue
		}
		e.rules[r.GetName()] = true
		var err error = nil
		switch {
		case r.GetRuleClass() == "cc_library":
			err = e.convertCCLibraryRule(r)
		case r.GetRuleClass() == "filegroup":
			err = e.convertFilegroupRule(r)
		}
		if err != nil {
			return skerr.Wrapf(err, "Failed to convert %s", r.GetRuleClass())
		}
	}

	e.printWorkspace()
	fmt.Printf("The Bazel query has %d targets.\n", len(e.rules))
	return nil
}
