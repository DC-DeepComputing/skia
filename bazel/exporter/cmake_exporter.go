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

func findRule(qr *analysis_v2.CqueryResult, name string) (*build.Rule, error) {
	for _, result := range qr.GetResults() {
		r := result.GetTarget().GetRule()
		if r.GetName() == name {
			return r, nil
		}
	}
	return nil, skerr.Fmt(`cannot find rule "%s"`, name)
}

func getRuleStringArrayAttribute(r *build.Rule, name string) ([]string, error) {
	for _, attrib := range r.Attribute {
		if attrib.GetName() != name {
			continue
		}
		if attrib.GetType() != build.Attribute_LABEL_LIST && attrib.GetType() != build.Attribute_STRING_LIST {
			return []string{}, skerr.Fmt(`srcs in rule "%s" is not a list`, r.GetName())
		}
		return attrib.GetStringListValue(), nil
	}
	return []string{}, nil
}

func getPlatformRuleCopts() ([]string, error) {
	// TODO: This is a total HACK - just as an experiment.
	// The copts (and I think linker opts) come from toolchain/download_mac_toolchain.bzl
	// and possibly toolchain/mac_toolchain_config.bzl, but don't seem to be making their
	// way into the cquery (nor query) output. Need to figure this out.
	hardCodedRules := []string{
		"-isystem",
		"external/clang_mac/include/c++/v1",
		"-isystem",
		"external/clang_mac/symlinks/xcode/MacSDK/usr/include",
		"-isystem",
		"external/clang_mac/lib/clang/13.0.0/include",
		"-F",
		"/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks",
		"-no-canonical-prefixes",
		"-std=c++17",
		"-Wno-psabi",
		"--target=arm64-apple-macos11",
		"-O1",
		"--sysroot=external/clang_mac",
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

func getRuleDeps(r *build.Rule) ([]string, error) {
	return getRuleStringArrayAttribute(r, "deps")
}

func getRuleLinkOpts(r *build.Rule) ([]string, error) {
	return getRuleStringArrayAttribute(r, "linkopts")
}

func getRuleIncludes(r *build.Rule) ([]string, error) {
	return getRuleStringArrayAttribute(r, "includes")
}

func isLocalDir(path string) bool {
	return path == "."
}

func getRuleDefines(r *build.Rule, qr *analysis_v2.CqueryResult) ([]string, error) {
	deps, err := getRuleDeps(r)
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
		// TODO: This can result in duplicate entries. Fix this.
		defines = append(defines, defs...)
	}
	fmt.Printf("defs for rule %s: %v\n", r.GetName(), defines)
	return defines, nil
}

func (e *CMakeExporter) isFileRule(target string) bool {
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
		strings.Contains(r.GetName(), "//infra")
}

func ignoreRule(r *build.Rule) bool {
	ignoreRules := []string{
		"_gazelle_runner",
		"_karma_test",
		"alias",
		"bool_flag",
		"cc_library_with_flags",
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

func (e *CMakeExporter) writeIncludeDirectories(r *build.Rule, buffer *bytes.Buffer) error {
	includes, err := getRuleIncludes(r)
	if err != nil {
		return skerr.Wrap(err)
	}
	noLocalDir := []string{}
	for _, d := range includes {
		if !isLocalDir(d) {
			noLocalDir = append(noLocalDir, d)
		}
	}
	if len(noLocalDir) == 0 {
		// No error, just nothing to write.
		return nil
	}
	str := strings.Join(noLocalDir, " ")
	_, err = fmt.Fprintf(buffer, "set_target_properties(%s PROPERTIES INCLUDE_DIRECTORIES\n  \"%s\"\n)\n", getRuleCMakeName(r.GetName()), str)
	return err
}

func (e *CMakeExporter) writeLinkFlags(r *build.Rule, buffer *bytes.Buffer) error {
	defines, err := getRuleLinkOpts(r)
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

func (e *CMakeExporter) appendStuff(r *build.Rule, qr *analysis_v2.CqueryResult, buffer *bytes.Buffer) error {
	err := e.appendCompileFlags(r, buffer)
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
	err = e.writeIncludeDirectories(r, buffer)
	if err != nil {
		return skerr.Wrap(err)
	}
	return nil
}

func (e *CMakeExporter) convertCCLibraryRule(r *build.Rule, qr *analysis_v2.CqueryResult) error {
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
	err = e.appendStuff(r, qr, &buffer)
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
		log.Fatalln("Failed to unmarshal Bazel cquery result:", err)
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
		case ignoreRule(r):
			err = e.ignoreRule(r)
		case r.GetRuleClass() == "cc_binary":
			err = e.convertCCBinaryRule(r)
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

	fmt.Printf("The Bazel query has %d targets.\n", len(e.rules))

	_, err = e.workspace.write(writer)
	if err != nil {
		return skerr.Wrap(err)
	}

	return nil
}

// Make sure CMakeExporter fulfills the Exporter interface.
var _ Exporter = (*CMakeExporter)(nil)
