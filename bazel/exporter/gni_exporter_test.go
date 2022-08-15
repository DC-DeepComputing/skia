// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package exporter

import (
	"bytes"
	"os"
	"path/filepath"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
	"github.com/stretchr/testify/require"
	"go.skia.org/skia/bazel/exporter/build_proto/analysis_v2"
	"go.skia.org/skia/bazel/exporter/interfaces/mocks"
	"google.golang.org/protobuf/encoding/prototext"
)

// These test query results are generated with the following command:
//
//	bazel cquery --noimplicit_deps \
//	   'kind("rule", deps(//src/core:core_srcs) + deps(//src/opts:private_hdrs))' \
//	   --output textproto
//
// Then the output data is manually pruned to include only the first three files.
const publicSrcsTextProto = `results {
	target {
	  type: RULE
	  rule {
		name: "//src/core:core_srcs"
		rule_class: "filegroup"
		location: "/path/to/skia/src/src/core/BUILD.bazel:397:20"
		attribute {
		  name: "$config_dependencies"
		  type: LABEL_LIST
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: ":action_listener"
		  type: LABEL_LIST
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "applicable_licenses"
		  type: LABEL_LIST
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "aspect_hints"
		  type: LABEL_LIST
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "compatible_with"
		  type: LABEL_LIST
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "data"
		  type: LABEL_LIST
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "deprecation"
		  type: STRING
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "distribs"
		  type: DISTRIBUTION_SET
		  string_list_value: "INTERNAL"
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "features"
		  type: STRING_LIST
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "generator_function"
		  type: STRING
		  string_value: "split_srcs_and_hdrs"
		  explicitly_specified: true
		  nodep: false
		}
		attribute {
		  name: "generator_location"
		  type: STRING
		  string_value: "src/core/BUILD.bazel:397:20"
		  explicitly_specified: true
		  nodep: false
		}
		attribute {
		  name: "generator_name"
		  type: STRING
		  string_value: "core"
		  explicitly_specified: true
		  nodep: false
		}
		attribute {
		  name: "licenses"
		  type: LICENSE
		  license {
			license_type: "NOTICE"
		  }
		  explicitly_specified: false
		}
		attribute {
		  name: "name"
		  type: STRING
		  string_value: "core_srcs"
		  explicitly_specified: true
		  nodep: false
		}
		attribute {
		  name: "output_group"
		  type: STRING
		  string_value: ""
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "output_licenses"
		  type: LICENSE
		  license {
			license_type: "NONE"
		  }
		  explicitly_specified: false
		}
		attribute {
		  name: "path"
		  type: STRING
		  string_value: ""
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "restricted_to"
		  type: LABEL_LIST
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "srcs"
		  type: LABEL_LIST
		  string_list_value: "//src/core:SkAAClip.cpp"
		  string_list_value: "//src/core:SkATrace.cpp"
		  string_list_value: "//src/core:SkAlphaRuns.cpp"
		  explicitly_specified: true
		  nodep: false
		}
		attribute {
		  name: "tags"
		  type: STRING_LIST
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "target_compatible_with"
		  type: LABEL_LIST
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "testonly"
		  type: BOOLEAN
		  int_value: 0
		  string_value: "false"
		  explicitly_specified: false
		  boolean_value: false
		}
		attribute {
		  name: "transitive_configs"
		  type: STRING_LIST
		  explicitly_specified: false
		  nodep: true
		}
		attribute {
		  name: "visibility"
		  type: STRING_LIST
		  explicitly_specified: false
		  nodep: true
		}
		rule_input: "//src/core:SkAAClip.cpp"
		rule_input: "//src/core:SkATrace.cpp"
		rule_input: "//src/core:SkAlphaRuns.cpp"
	  }
	}
	configuration {
	  checksum: "d16aa11033851c6aac7f80d42f69ce16f44935bba14f1c71f7cfc07b0d4d60b2"
	}
  }
  results {
	target {
	  type: RULE
	  rule {
		name: "//src/opts:private_hdrs"
		rule_class: "filegroup"
		location: "/path/to/skia/src/src/opts/BUILD.bazel:26:10"
		attribute {
		  name: "$config_dependencies"
		  type: LABEL_LIST
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: ":action_listener"
		  type: LABEL_LIST
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "applicable_licenses"
		  type: LABEL_LIST
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "aspect_hints"
		  type: LABEL_LIST
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "compatible_with"
		  type: LABEL_LIST
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "data"
		  type: LABEL_LIST
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "deprecation"
		  type: STRING
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "distribs"
		  type: DISTRIBUTION_SET
		  string_list_value: "INTERNAL"
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "features"
		  type: STRING_LIST
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "generator_function"
		  type: STRING
		  string_value: ""
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "generator_location"
		  type: STRING
		  string_value: ""
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "generator_name"
		  type: STRING
		  string_value: ""
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "licenses"
		  type: LICENSE
		  license {
			license_type: "NOTICE"
		  }
		  explicitly_specified: false
		}
		attribute {
		  name: "name"
		  type: STRING
		  string_value: "private_hdrs"
		  explicitly_specified: true
		  nodep: false
		}
		attribute {
		  name: "output_group"
		  type: STRING
		  string_value: ""
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "output_licenses"
		  type: LICENSE
		  license {
			license_type: "NONE"
		  }
		  explicitly_specified: false
		}
		attribute {
		  name: "path"
		  type: STRING
		  string_value: ""
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "restricted_to"
		  type: LABEL_LIST
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "srcs"
		  type: LABEL_LIST
		  string_list_value: "//src/opts:SkBitmapProcState_opts.h"
		  string_list_value: "//src/opts:SkBlitMask_opts.h"
		  string_list_value: "//src/opts:SkBlitRow_opts.h"
		  explicitly_specified: true
		  nodep: false
		}
		attribute {
		  name: "tags"
		  type: STRING_LIST
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "target_compatible_with"
		  type: LABEL_LIST
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "testonly"
		  type: BOOLEAN
		  int_value: 0
		  string_value: "false"
		  explicitly_specified: false
		  boolean_value: false
		}
		attribute {
		  name: "transitive_configs"
		  type: STRING_LIST
		  explicitly_specified: false
		  nodep: true
		}
		attribute {
		  name: "visibility"
		  type: STRING_LIST
		  string_list_value: "//src:__pkg__"
		  explicitly_specified: true
		  nodep: true
		}
		rule_input: "//src/opts:SkBitmapProcState_opts.h"
		rule_input: "//src/opts:SkBlitMask_opts.h"
		rule_input: "//src/opts:SkBlitRow_opts.h"
	  }
	}
	configuration {
	  checksum: "d16aa11033851c6aac7f80d42f69ce16f44935bba14f1c71f7cfc07b0d4d60b2"
	}
  }
`

// The expected gn/core.gni file contents for publicSrcsTextProto.
// This expected result is handmade.
const publicSrcsExpectedGNI = `# DO NOT EDIT: This is a generated file.

_src = get_path_info("../src", "abspath")

# //src/core:core_srcs
skia_core_sources = [
  "$_src/core/SkAAClip.cpp",
  "$_src/core/SkATrace.cpp",
  "$_src/core/SkAlphaRuns.cpp",
  "$_src/opts/SkBitmapProcState_opts.h",
  "$_src/opts/SkBlitMask_opts.h",
  "$_src/opts/SkBlitRow_opts.h",
]

skia_core_sources += skia_pathops_sources
skia_core_sources += skia_skpicture_sources

skia_core_public += skia_pathops_public
skia_core_public += skia_skpicture_public

skia_core_defines = []  #  Used to be used by Chromium, but no longer.
`

var fileListWriteOrder = map[string][]string{
	"gn/core.gni": {
		"skia_core_sources",
	},
}

var testExporterParams = GNIExporterParams{
	WorkspaceDir: testWorkspaceDir,
	GNIFileVars:  fileListWriteOrder,
}

func TestGNIExporterExport_ValidInput_Success(t *testing.T) {
	protoData, err := textProtoToProtobuf(publicSrcsTextProto)
	require.NoError(t, err)

	fs := mocks.NewFileSystem(t)
	var contents bytes.Buffer
	fs.On("OpenFile", mock.Anything).Once().Run(func(args mock.Arguments) {
		path := args.String(0)
		assert.True(t, filepath.IsAbs(path))
		assert.Equal(t, filepath.Join(testWorkspaceDir, "gn/core.gni"), path)
	}).Return(&contents, nil).Once()
	e := NewGNIExporter(testExporterParams, fs)
	qcmd := mocks.NewQueryCommand(t)
	qcmd.On("Read", mock.Anything).Return(protoData, nil).Once()
	err = e.Export(qcmd)
	require.NoError(t, err)

	assert.Equal(t, publicSrcsExpectedGNI, contents.String())
}

func TestGNIExporterCheckCurrent_CurrentData_ReturnsZero(t *testing.T) {
	protoData, err := textProtoToProtobuf(publicSrcsTextProto)
	require.NoError(t, err)

	fs := mocks.NewFileSystem(t)
	fs.On("ReadFile", mock.Anything).Run(func(args mock.Arguments) {
		path := args.String(0)
		assert.True(t, filepath.IsAbs(path))
		assert.Equal(t, filepath.Join(testWorkspaceDir, "gn/core.gni"), path)
	}).Return([]byte(publicSrcsExpectedGNI), nil)
	e := NewGNIExporter(testExporterParams, fs)
	qcmd := mocks.NewQueryCommand(t)
	qcmd.On("Read", mock.Anything).Return(protoData, nil).Once()
	var errBuff bytes.Buffer
	numOutOfDate, err := e.CheckCurrent(qcmd, &errBuff)
	os.Stdout.Write(errBuff.Bytes())
	assert.NoError(t, err)
	assert.Zero(t, numOutOfDate)
}

func TestMakeRelativeFilePathForGNI_MatchingRootDir_Success(t *testing.T) {
	var testData = []struct {
		input    string
		expected string
	}{
		{"//src/core:file.cpp", "$_src/core/file.cpp"},
		{"//include/core:file.h", "$_include/core/file.h"},
		{"//modules/mod:file.cpp", "$_modules/mod/file.cpp"},
	}

	for _, tt := range testData {
		path, err := makeRelativeFilePathForGNI(tt.input)
		require.NoError(t, err)
		assert.Equal(t, tt.expected, path)
	}
}

func TestMakeRelativeFilePathForGNI_IndalidInput_ReturnsError(t *testing.T) {
	var testData = []string{
		"",
		"//valid/rule/incorrect/root/dir:file.cpp",
	}
	for _, tt := range testData {
		_, err := makeRelativeFilePathForGNI(tt)
		assert.Error(t, err)
	}
}

func TestGetRuleGNIVariableName_ValidRuleName_Success(t *testing.T) {
	qr := analysis_v2.CqueryResult{}
	err := prototext.Unmarshal([]byte(publicSrcsTextProto), &qr)
	require.NoError(t, err)
	r := findRule(&qr, "//src/core:core_srcs")
	require.NotNil(t, r)
	name, err := getRuleGNIVariableName(r)
	require.NoError(t, err)
	assert.Equal(t, "skia_core_sources", name)
}

func TestIsTargetCppHeaderFile_MatchExpecting(t *testing.T) {
	var testData = []struct {
		input    string
		isHeader bool
	}{
		{"//include/core:file.h", true},
		{"//include/core:file.H", true},
		{"//include/core:file.Hpp", true},
		{"//src/core:file.cpp", false},
		{"//src/core:file.HH", false},
		{"", false},
	}

	for _, tt := range testData {
		assert.Equal(t, tt.isHeader, isTargetCppHeaderFile(tt.input))
	}
}

func TestShouldSkipRule_ShouldNotSkip_ReturnFalse(t *testing.T) {
	qr := analysis_v2.CqueryResult{}
	err := prototext.Unmarshal([]byte(publicSrcsTextProto), &qr)
	require.NoError(t, err)
	r := findRule(&qr, "//src/core:core_srcs")
	require.NotNil(t, r)
	assert.False(t, shouldSkipRule(r))
}

func TestAbsToWorkspacePath_AbsPath_ReturnExpected(t *testing.T) {
	var testData = []struct {
		input    string
		expected string
	}{
		{filepath.Join(testWorkspaceDir, "foo/bar.txt"), "foo/bar.txt"},
		{filepath.Join(testWorkspaceDir, "foo/bar"), "foo/bar"},
		{filepath.Join(testWorkspaceDir, "root.txt"), "root.txt"},
		{testWorkspaceDir, ""},
		{testWorkspaceDir + "/", ""},
	}
	fs := mocks.NewFileSystem(t)
	e := NewGNIExporter(testExporterParams, fs)
	require.NotNil(t, e)
	for _, tt := range testData {
		assert.Equal(t, tt.expected, e.absToWorkspacePath(tt.input))
	}
}

func TestAbsToWorkspacePath_WorkspaceRelPath_ReturnExpected(t *testing.T) {
	var testData = []struct {
		input    string
		expected string
	}{
		{"foo/bar.txt", filepath.Join(testWorkspaceDir, "foo/bar.txt")},
		{"foo/bar", filepath.Join(testWorkspaceDir, "foo/bar")},
		{"root.txt", filepath.Join(testWorkspaceDir, "root.txt")},
		{"", testWorkspaceDir},
	}
	fs := mocks.NewFileSystem(t)
	e := NewGNIExporter(testExporterParams, fs)
	require.NotNil(t, e)
	for _, tt := range testData {
		assert.Equal(t, tt.expected, e.workspaceToAbsPath(tt.input))
	}
}

func TestGetFileData_SamePath_ReturnSamePtr(t *testing.T) {
	e := NewGNIExporter(testExporterParams, mocks.NewFileSystem(t))
	require.NotNil(t, e)
	first := e.getFileData("gn/anything.gni")
	second := e.getFileData("gn/anything.gni")
	assert.Equal(t, first, second)
}

func TestGetFileData_DifferentPath_ReturnDifferentPtr(t *testing.T) {
	e := NewGNIExporter(testExporterParams, mocks.NewFileSystem(t))
	require.NotNil(t, e)
	first := e.getFileData("gn/anything.gni")
	second := e.getFileData("gn/another.gni")
	assert.NotEqual(t, first, second)
}

func TestGetGNILineVariable_VariousLines_ReturnsExpected(t *testing.T) {
	var testData = []struct {
		input    string
		expected string
	}{
		// Lines with variables
		{`foo = [ "something" ]`, "foo"},
		{`foo=[ "something" ]`, "foo"},
		{`foo =[ "something" ]`, "foo"},
		// Lines w/o variables
		{` foo = [ "something" ]`, ""},
		{`foo = bar`, ""},
		{`[ "something" ]`, ""},
		{`# foo = [ "something" ]`, ""},
		{`=[ "something" ]`, ""},
		{``, ""},
	}

	for _, tt := range testData {
		assert.Equal(t, tt.expected, getGNILineVariable(tt.input))
	}
}
