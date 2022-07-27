// Copyright 2022 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package exporter

import (
	"bytes"
	"fmt"
	"io"
	"log"
	"path/filepath"
	"strings"

	"github.com/golang/protobuf/proto"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/skia/bazel/exporter/build_proto/analysis_v2"
	"go.skia.org/skia/bazel/exporter/build_proto/build"
	intf "go.skia.org/skia/bazel/exporter/interfaces"
)

type CMakeExporter struct {
	workspace    *CMakeWorkspace
	workspaceDir string
}

// NewCMakeExporter creates an exporter that will export to the
// CMake project file format.
func NewCMakeExporter(workspaceDir string) *CMakeExporter {
	return &CMakeExporter{
		workspace:    NewCMakeWorkspace(),
		workspaceDir: workspaceDir,
	}
}

func isFileRule(target string) bool {
	_, _, target, err := parseRule(target)
	if err != nil {
		return false
	}
	return strings.Contains(target, ".")
}

func findRule(qr *analysis_v2.CqueryResult, name string) (*build.Rule, error) {
	for _, result := range qr.GetResults() {
		r := result.GetTarget().GetRule()
		if r.GetName() == name {
			return r, nil
		}
	}
	return nil, skerr.Fmt(`cannot find rule "%s"`, name)
}

func getPlatformRuleCopts() ([]string, error) {
	// TODO: Retrieve this data from Bazel.
	// These are defined by the toolchain. These values are not returned in
	// either query or cquery responses.
	hardCodedRules := []string{
		// These items are from _make_default_flags() in mac_toolchain_config.bzl.
		"-isystem",
		"external/clang_mac/include/c++/v1",
		"-isystem",
		"external/clang_mac/symlinks/xcode/MacSDK/usr/include",
		"-isystem",
		"external/clang_mac/lib/clang/13.0.0/include",
		"-F",
		"/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks",
		"-no-canonical-prefixes",
		// Also from _make_default_flags.
		"-std=c++17",
		"-Wno-psabi",

		// From _make_target_specific_flags in mac_toolchain_config.bzl.
		"--target=arm64-apple-macos11",
	}
	return hardCodedRules, nil
}

func getRuleCopts(r *build.Rule) ([]string, error) {
	copts, err := getRuleStringArrayAttribute(r, "copts")
	if err != nil {
		return []string{}, skerr.Wrap(err)
	}
	popts, err := getPlatformRuleCopts()
	if err != nil {
		return []string{}, skerr.Wrap(err)
	}
	return append(popts, copts...), nil
}

func appendUnique(slice []string, elems ...string) []string {
	for _, elem := range elems {
		if !stringSliceContains(slice, elem) {
			slice = append(slice, elem)
		}
	}
	return slice
}

func getRuleIncludes(r *build.Rule, qr *analysis_v2.CqueryResult) ([]string, error) {
	deps, err := getRuleStringArrayAttribute(r, "deps")
	if err != nil {
		return []string{}, skerr.Wrap(err)
	}
	includes, err := getRuleStringArrayAttribute(r, "includes")
	if err != nil {
		return []string{}, skerr.Wrap(err)
	}
	ruleDir, err := getLocationDir(r.GetLocation())
	for idx, inc := range includes {
		if inc == "." {
			if err != nil {
				return []string{}, skerr.Wrap(err)
			}
			includes[idx] = ruleDir
		}
	}
	for _, d := range deps {
		dr, err := findRule(qr, d)
		if err != nil {
			return []string{}, skerr.Wrap(err)
		}
		incs, err := getRuleIncludes(dr, qr)
		if err != nil {
			return []string{}, skerr.Wrap(err)
		}
		includes = appendUnique(includes, incs...)
	}
	return includes, nil
}

func getRuleDefines(r *build.Rule, qr *analysis_v2.CqueryResult) ([]string, error) {
	deps, err := getRuleStringArrayAttribute(r, "deps")
	if err != nil {
		return []string{}, skerr.Wrap(err)
	}
	defines, err := getRuleStringArrayAttribute(r, "defines")
	if err != nil {
		return []string{}, skerr.Wrap(err)
	}
	for _, d := range deps {
		dr, err := findRule(qr, d)
		if err != nil {
			return []string{}, skerr.Wrap(err)
		}
		defs, err := getRuleDefines(dr, qr)
		if err != nil {
			return []string{}, skerr.Wrap(err)
		}
		defines = appendUnique(defines, defs...)
	}
	return defines, nil
}

// Write the list of items (which may be rules or files) to the supplied buffer.
func (e *CMakeExporter) writeItems(r *CMakeRule, projectDir string, items []string, buffer *bytes.Buffer) error {
	for _, item := range items {
		if isFileRule(item) {
			_, _, target, err := parseRule(item)
			if err != nil {
				return skerr.Wrap(err)
			}
			fmt.Fprintf(buffer, "    \"%s\"\n", filepath.Join(projectDir, target))
		} else {
			fmt.Fprintf(buffer, "    ${%s}\n", getRuleCMakeName(item))
			err := r.addDependency(item)
			if err != nil {
				return skerr.Wrap(err)
			}
		}
	}
	return nil
}

// Write the "srcs" and "hdrs" rule attributes to the supplied buffer.
func (e *CMakeExporter) writeSrcsAndHdrs(rule *CMakeRule, buffer *bytes.Buffer, r *build.Rule) error {
	ruleDir, err := getLocationDir(r.GetLocation())
	if err != nil {
		return skerr.Wrap(err)
	}
	for _, attrib := range r.Attribute {
		if attrib.GetName() == "srcs" {
			if attrib.GetType() != build.Attribute_LABEL_LIST {
				return skerr.Fmt(`srcs in rule "%s" is not a list`, r.GetName())
			}
			fmt.Fprintln(buffer, "    # Sources:")
			err := e.writeItems(rule, ruleDir, attrib.GetStringListValue(), buffer)
			if err != nil {
				return skerr.Wrap(err)
			}
		}
		if attrib.GetName() == "hdrs" {
			if attrib.GetType() != build.Attribute_LABEL_LIST {
				return skerr.Fmt(`hdrs in rule "%s" is not a list`, r.GetName())
			}
			fmt.Fprintln(buffer, "    # Headers:")
			err := e.writeItems(rule, ruleDir, attrib.GetStringListValue(), buffer)
			if err != nil {
				return skerr.Wrap(err)
			}
		}
	}
	return nil
}

// Write the target COMPILE_FLAGS property to the supplied buffer (if there are any copts).
func (e *CMakeExporter) writeCompileFlags(r *build.Rule, buffer *bytes.Buffer) error {
	copts, err := getRuleCopts(r)
	if err != nil {
		return skerr.Wrap(err)
	}
	if len(copts) == 0 {
		// No error, just nothing to write.
		return nil
	}
	str := strings.Join(copts, " ")
	_, err = fmt.Fprintf(buffer, "set_target_properties(%s PROPERTIES COMPILE_FLAGS\n  \"%s\"\n)\n", getRuleCMakeName(r.GetName()), str)
	return err
}

// Write the target COMPILE_DEFINITIONS property to the supplied buffer (if there are any defines).
func (e *CMakeExporter) writeCompileDefinitions(r *build.Rule, qr *analysis_v2.CqueryResult, buffer *bytes.Buffer) error {
	defines, err := getRuleDefines(r, qr)
	if err != nil {
		return skerr.Wrap(err)
	}
	if len(defines) == 0 {
		// No error, just nothing to write.
		return nil
	}
	str := strings.Join(defines, ";")
	_, err = fmt.Fprintf(buffer, "set_target_properties(%s PROPERTIES COMPILE_DEFINITIONS\n  \"%s\"\n)\n", getRuleCMakeName(r.GetName()), str)
	return err
}

// Write the target INCLUDE_DIRECTORIES property to the supplied buffer (if there are any).
func (e *CMakeExporter) writeIncludeDirectories(r *build.Rule, qr *analysis_v2.CqueryResult, buffer *bytes.Buffer) error {
	includes, err := getRuleIncludes(r, qr)
	if err != nil {
		return skerr.Wrap(err)
	}
	includes = appendUnique(includes, e.workspaceDir)
	str := strings.Join(includes, ";")
	_, err = fmt.Fprintf(buffer, "set_target_properties(%s PROPERTIES INCLUDE_DIRECTORIES\n  \"%s\"\n)\n", getRuleCMakeName(r.GetName()), str)
	return err
}

// Write the target LINK_FLAGS property to the supplied buffer (if there are any linkopts).
func (e *CMakeExporter) writeLinkFlags(r *build.Rule, buffer *bytes.Buffer) error {
	defines, err := getRuleStringArrayAttribute(r, "linkopts")
	if err != nil {
		return skerr.Wrap(err)
	}
	if len(defines) == 0 {
		// No error, just nothing to write.
		return nil
	}
	str := strings.Join(defines, " ")
	_, err = fmt.Fprintf(buffer, "set_target_properties(%s PROPERTIES LINK_FLAGS\n  \"%s\"\n)\n", getRuleCMakeName(r.GetName()), str)
	return err
}

// Write all target properties to the supplied buffer.
func (e *CMakeExporter) writeProperties(r *build.Rule, qr *analysis_v2.CqueryResult, buffer *bytes.Buffer) error {
	err := e.writeCompileFlags(r, buffer)
	if err != nil {
		return skerr.Wrap(err)
	}
	err = e.writeLinkFlags(r, buffer)
	if err != nil {
		return skerr.Wrap(err)
	}
	err = e.writeCompileDefinitions(r, qr, buffer)
	if err != nil {
		return skerr.Wrap(err)
	}
	err = e.writeIncludeDirectories(r, qr, buffer)
	if err != nil {
		return skerr.Wrap(err)
	}
	return nil
}

// Convert the filegroup rule to the CMake equivalent.
func (e *CMakeExporter) convertFilegroupRule(r *build.Rule) error {

	rule := e.workspace.createRule(r)

	contents := new(bytes.Buffer)

	targetName := r.GetName()
	variableName := getRuleCMakeName(targetName)
	fmt.Fprintf(contents, "# %s\n", targetName)
	fmt.Fprintf(contents, "list(APPEND %s\n", variableName)

	err := e.writeSrcsAndHdrs(rule, contents, r)
	if err != nil {
		return skerr.Wrap(err)
	}
	fmt.Fprintln(contents, ")")
	rule.setContents(contents)

	return nil
}

// Convert the cc_binary rule to the CMake equivalent.
func (e *CMakeExporter) convertCCBinaryRule(r *build.Rule, qr *analysis_v2.CqueryResult) error {

	rule := e.workspace.createRule(r)

	targetName := r.GetName()
	contents := new(bytes.Buffer)
	fmt.Fprintf(contents, "# %s\n", targetName)
	fmt.Fprintf(contents, "add_executable(%s \"\")\n", getRuleCMakeName(targetName))
	fmt.Fprintf(contents, "target_sources(%s\n", getRuleCMakeName(targetName))
	fmt.Fprintln(contents, "  PRIVATE")

	err := e.writeSrcsAndHdrs(rule, contents, r)
	if err != nil {
		return skerr.Wrap(err)
	}

	fmt.Fprintln(contents, ")")
	err = e.writeProperties(r, qr, contents)
	if err != nil {
		return skerr.Wrap(err)
	}
	rule.setContents(contents)

	return nil
}

// Convert the cc_library rule to the CMake equivalent.
func (e *CMakeExporter) convertCCLibraryRule(r *build.Rule, qr *analysis_v2.CqueryResult) error {

	rule := e.workspace.createRule(r)

	targetName := r.GetName()
	contents := new(bytes.Buffer)
	fmt.Fprintf(contents, "# %s\n", targetName)
	fmt.Fprintf(contents, "add_library(%s \"\")\n", getRuleCMakeName(targetName))
	fmt.Fprintf(contents, "target_sources(%s\n", getRuleCMakeName(targetName))
	fmt.Fprintln(contents, "  PRIVATE")

	err := e.writeSrcsAndHdrs(rule, contents, r)
	if err != nil {
		return skerr.Wrap(err)
	}
	fmt.Fprintln(contents, ")")
	err = e.writeProperties(r, qr, contents)
	if err != nil {
		return skerr.Wrap(err)
	}

	rule.setContents(contents)

	return nil
}

func ignoreRule(r *build.Rule) bool {
	ignoreRules := []string{
		"_gazelle_runner",
		"_karma_test",
		"alias",
		"bool_flag",
		"cc_library_with_flags",
		"compiler_flag",
		"config_setting",
		"constraint_setting",
		"constraint_value",
		"genrule",
		"go_binary",
		"go_library",
		"multi_string_flag",
		"platform",
		"py_binary",
		"py_library",
		"sh_binary",
		"sh_test",
		"string_flag",
		"transition_rule",
		"wasm_cc_binary",
		"web_test",
	}

	return stringSliceContains(ignoreRules, r.GetRuleClass())
}

// Export will convert the input Bazel cquery output, provided by the
// supplied QueryCommand parameter, to CMake. The equivalent
// CMake project definition will be written using the supplied writer.
func (e *CMakeExporter) Export(qcmd intf.QueryCommand, writer io.Writer, projName string) error {

	in, err := qcmd.Read()
	if err != nil {
		return skerr.Wrapf(err, "error reading bazel cquery data")
	}
	qr := &analysis_v2.CqueryResult{}
	if err := proto.Unmarshal(in, qr); err != nil {
		log.Fatalln("Failed to unmarshal Bazel cquery result:", err)
		return skerr.Wrap(err)
	}

	fmt.Fprintln(writer, "# DO NOT EDIT: This file is auto-generated.")
	fmt.Fprintln(writer, "cmake_minimum_required(VERSION 3.13)")
	fmt.Fprintln(writer)
	fmt.Fprintf(writer, "project(%s LANGUAGES C CXX)\n", projName)
	fmt.Fprintln(writer)

	for _, result := range qr.GetResults() {
		t := result.GetTarget()
		r := t.GetRule()
		var err error = nil
		switch {
		case ignoreRule(r):
			continue
		case r.GetRuleClass() == "cc_binary":
			err = e.convertCCBinaryRule(r, qr)
		case r.GetRuleClass() == "cc_library":
			err = e.convertCCLibraryRule(r, qr)
		case r.GetRuleClass() == "filegroup":
			err = e.convertFilegroupRule(r)
		default:
			return skerr.Fmt("unsupported Bazel rule: \"%s\"\n", r.GetRuleClass())
		}
		if err != nil {
			return skerr.Wrapf(err, "Failed to convert %s", r.GetRuleClass())
		}
	}

	_, err = e.workspace.write(writer)
	if err != nil {
		return skerr.Wrap(err)
	}

	return nil
}

// Make sure CMakeExporter fulfills the Exporter interface.
var _ intf.Exporter = (*CMakeExporter)(nil)
