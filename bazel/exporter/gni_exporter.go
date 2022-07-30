// Copyright 2022 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package exporter

import (
	"fmt"
	"io"
	"path/filepath"
	"strings"

	"go.skia.org/infra/go/skerr"
	"go.skia.org/skia/bazel/exporter/build_proto/analysis_v2"
	"go.skia.org/skia/bazel/exporter/build_proto/build"
	intf "go.skia.org/skia/bazel/exporter/interfaces"
	"google.golang.org/protobuf/proto"
)

type GNIExporter struct {
	workspaceDir string               // The Bazel workspace path.
	openFile     intf.FileOpenFunc    // Function to open a file for writing.
	gnFileMap    map[string]io.Writer // Map *.gni filenames to file writers.
}

// NewGNIExporter creates an exporter that will export to GN's (*.gni) files.
func NewGNIExporter(workspaceDir string, openFile intf.FileOpenFunc) *GNIExporter {
	return &GNIExporter{
		workspaceDir: workspaceDir,
		openFile:     openFile,
		gnFileMap:    make(map[string]io.Writer),
	}
}

// Retrieve all rule attributes which are file targets.
func getRuleFiles(r *build.Rule, attrName string) ([]string, error) {
	items, err := getRuleStringArrayAttribute(r, attrName)
	if err != nil {
		return []string{}, skerr.Wrap(err)
	}

	files := []string{}
	for _, item := range items {
		if isFileTarget(item) {
			files = append(files, item)
		}
	}
	return files, nil
}

// Write the *.gni file header.
func writeGNFileHeader(writer io.Writer) {
	fmt.Fprintln(writer, "# DO NOT EDIT: This is a generated file.")
	fmt.Fprintln(writer)
	fmt.Fprintln(writer, `_src = get_path_info("../src", "abspath")`)
	fmt.Fprintln(writer, `_include = get_path_info("../include", "abspath")`)
	fmt.Fprintln(writer)
}

func (e *GNIExporter) getRuleGNFilePath(ruleLocationDir string) string {

	wsRelativePath := strings.TrimPrefix(ruleLocationDir[len(e.workspaceDir):], "/")

	const defaultGNI = "gn/XXX.gni"
	dirMap := map[string]string{
		"include/android":          defaultGNI,
		"include/codec":            defaultGNI,
		"include/config":           defaultGNI,
		"include/core":             "gn/core.gni",
		"include/effects":          "gn/effects.gni",
		"include/encode":           defaultGNI,
		"include/pathops":          "gn/core.gni",
		"include/ports":            defaultGNI,
		"include/private":          defaultGNI,
		"include/private/chromium": "gn/core.gni",
		"include/utils":            "gn/utils.gni",
		"modules/skcms":            "modules/skcms/skcms.gni",
		"src/android":              defaultGNI,
		"src/codec":                "gn/sksl.gni",
		"src/core":                 "gn/core.gni",
		"src/effects":              "gn/effects.gni",
		"src/effects/imagefilters": "gn/effects_imagefilters.gni",
		"src/image":                "gn/core.gni",
		"src/images":               "gn/core.gni",
		"src/opts":                 "gn/opts.gni",
		"src/pathops":              "gn/core.gni",
		"src/ports":                "gn/sksl.gni",
		"src/sfnt":                 defaultGNI,
		"src/shaders":              "gn/core.gni",
		"src/shaders/gradients":    "gn/effects.gni",
		"src/text":                 "gn/core.gni",
		"src/utils":                "gn/utils.gni",
	}
	gniBaseName, ok := dirMap[wsRelativePath]
	if !ok {
		gniBaseName = defaultGNI
	}
	return filepath.Join(e.workspaceDir, gniBaseName)
}

// Obtain a writer to use for the given Bazel rule. The .gni configuration
// will be written using this writer.
func (e *GNIExporter) getRuleWriter(r *build.Rule) (io.Writer, error) {
	dir, err := getLocationDir(r.GetLocation())
	if err != nil {
		return nil, skerr.Wrap(err)
	}
	gnPath := e.getRuleGNFilePath(dir)
	w, ok := e.gnFileMap[gnPath]
	if ok {
		return w, nil
	}

	w, err = e.openFile(gnPath)
	if err != nil {
		return nil, skerr.Wrap(err)
	}
	writeGNFileHeader(w)

	e.gnFileMap[gnPath] = w
	return w, nil
}

// Export the *.gni configuration to the given rule to the appropriate
// output file.
func (e *GNIExporter) exportRule(r *build.Rule) error {
	srcs, err := getRuleFiles(r, "srcs")
	if err != nil {
		return skerr.Wrap(err)
	}
	hdrs, err := getRuleFiles(r, "hdrs")
	if err != nil {
		return skerr.Wrap(err)
	}
	if len(srcs) == 0 && len(hdrs) == 0 {
		return nil
	}

	writer, err := e.getRuleWriter(r)
	if err != nil {
		return skerr.Wrap(err)
	}

	fmt.Fprintf(writer, "# %s\n", r.GetName())
	fmt.Fprintf(writer, "%s = [\n", getRuleCMakeName(r.GetName()))
	for _, target := range srcs {
		_, path, target, err := parseRule(target)
		if err != nil {
			return skerr.Wrap(err)
		}
		relativePath := filepath.Join(strings.TrimPrefix(path, "/"), target)
		fmt.Fprintf(writer, "  \"$_src/%s\",\n", relativePath)
	}
	for _, target := range hdrs {
		_, path, target, err := parseRule(target)
		if err != nil {
			return skerr.Wrap(err)
		}
		relativePath := filepath.Join(strings.TrimPrefix(path, "/"), target)
		fmt.Fprintf(writer, "  \"$_include/%s\",\n", relativePath)
	}
	fmt.Fprintln(writer, "]")
	fmt.Fprintln(writer)
	return nil
}

// Export will convert all rules defined in the Bazel query
// command result to *.gni files within the workspace.
func (e *GNIExporter) Export(qcmd intf.QueryCommand) error {

	in, err := qcmd.Read()
	if err != nil {
		return skerr.Wrapf(err, "error reading bazel cquery data")
	}
	qr := &analysis_v2.CqueryResult{}
	if err := proto.Unmarshal(in, qr); err != nil {
		return skerr.Wrapf(err, "failed to unmarshal query result")
	}

	for _, result := range qr.GetResults() {
		t := result.GetTarget()
		r := t.GetRule()
		err := e.exportRule(r)
		if err != nil {
			return skerr.Fmt("error exporting rule %s", r.GetName())
		}
	}

	return nil
}

// Make sure GNIExporter fulfills the Exporter interface.
var _ intf.Exporter = (*GNIExporter)(nil)
