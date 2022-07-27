// Copyright 2022 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package exporter

import (
	"bytes"
	"io"
	"strings"
	"testing"

	"github.com/golang/protobuf/proto"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
	"github.com/stretchr/testify/require"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/skia/bazel/exporter/build_proto/analysis_v2"
	"go.skia.org/skia/bazel/exporter/interfaces/mocks"
)

const testWorkspaceDir = "/path/to/workspace"

const textProto = `results {
	target {
		type: RULE
		rule {
		  name: "//src/libs:sum"
		  rule_class: "cc_library"
		  location: "/path/to/workspace/src/libs/BUILD.bazel:8:11"
		  attribute {
			name: "copts"
			type: STRING_LIST
			string_list_value: "-O2"
			explicitly_specified: true
			nodep: false
		  }
		  attribute {
			name: "defines"
			type: STRING_LIST
			string_list_value: "SUMDEF"
			explicitly_specified: false
			nodep: false
		  }
		  attribute {
			name: "includes"
			type: STRING_LIST
			string_list_value: "."
			explicitly_specified: false
			nodep: false
		  }
		  attribute {
			name: "linkopts"
			type: STRING_LIST
			string_list_value: "-L/library/dir"
			explicitly_specified: false
			nodep: false
		  }
		  attribute {
			name: "name"
			type: STRING
			string_value: "sum"
			explicitly_specified: true
			nodep: false
		  }
		  attribute {
			  name: "srcs"
			  type: LABEL_LIST
			  string_list_value: "//src/libs:sum.cpp"
			  explicitly_specified: true
			  nodep: false
		  }
		  attribute {
			name: "hdrs"
			type: LABEL_LIST
			string_list_value: "//src/libs:sum.h"
			explicitly_specified: true
			nodep: false
		  }
		  attribute {
			name: "visibility"
			type: STRING_LIST
			string_list_value: "//visibility:public"
			explicitly_specified: true
			nodep: true
		  }
		}
	  },
	}
	results {
	  target {
	  type: RULE
	  rule {
		name: "//src/apps:hello"
		rule_class: "cc_binary"
		location: "/path/to/workspace/src/apps/BUILD.bazel:8:11"
		attribute {
		  name: "copts"
		  type: STRING_LIST
		  string_list_value: "-O1"
		  explicitly_specified: true
		  nodep: false
		}
		attribute {
		  name: "defines"
		  type: STRING_LIST
		  string_list_value: "APPDEF"
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "linkopts"
		  type: STRING_LIST
		  string_list_value: "-L/app/dir"
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "name"
		  type: STRING
		  string_value: "hello"
		  explicitly_specified: true
		  nodep: false
		}
		attribute {
			name: "srcs"
			type: LABEL_LIST
			string_list_value: "//src/apps:hello-world.cpp"
			explicitly_specified: true
			nodep: false
		}
		attribute {
			name: "deps"
			type: LABEL_LIST
			string_list_value: "//src/libs:sum"
			explicitly_specified: false
			nodep: false
		}
		attribute {
		  name: "visibility"
		  type: STRING_LIST
		  string_list_value: "//visibility:public"
		  explicitly_specified: true
		  nodep: true
		}
	  }
	}
  }`

func TestExport_QueryReadError_ReturnsError(t *testing.T) {

	e := NewCMakeExporter(testWorkspaceDir)
	contents := new(bytes.Buffer)
	qcmd := mocks.NewQueryCommand(t)
	qcmd.On("Read", mock.Anything).Return([]byte{}, skerr.Fmt("expected error"))
	err := e.Export(qcmd, contents, "projName")
	assert.Error(t, err)
	assert.Equal(t, 0, contents.Len())
}

func TestExport_InvalidProtobuf_ReturnsError(t *testing.T) {

	e := NewCMakeExporter(testWorkspaceDir)
	contents := new(bytes.Buffer)
	qcmd := mocks.NewQueryCommand(t)
	qcmd.On("Read", mock.Anything).Return(make([]byte, 50), skerr.Fmt("empty data"))
	err := e.Export(qcmd, contents, "projName")
	assert.Error(t, err)
	assert.Equal(t, 0, contents.Len())
}

func TestExport_ValidProtobuf_Success(t *testing.T) {

	qr := &analysis_v2.CqueryResult{}
	err := proto.UnmarshalText(textProto, qr)
	require.NoError(t, err)
	protoData, err := proto.Marshal(qr)
	require.NoError(t, err)

	e := NewCMakeExporter(testWorkspaceDir)
	contents := new(bytes.Buffer)
	qcmd := mocks.NewQueryCommand(t)
	qcmd.On("Read", mock.Anything).Return(protoData, nil)
	err = e.Export(qcmd, contents, "projName")
	require.NoError(t, err)

	const expected = `# DO NOT EDIT: This file is auto-generated.
cmake_minimum_required(VERSION 3.13)

project(projName LANGUAGES C CXX)


# //src/libs:sum
add_library(src_libs_sum "")
target_sources(src_libs_sum
  PRIVATE
    # Sources:
    "${CMAKE_SOURCE_DIR}/src/libs/sum.cpp"
    # Headers:
    "${CMAKE_SOURCE_DIR}/src/libs/sum.h"
)
set_target_properties(src_libs_sum PROPERTIES COMPILE_FLAGS
  "-std=c++17 -Wno-psabi --target=arm64-apple-macos11 -O2"
)
set_target_properties(src_libs_sum PROPERTIES LINK_FLAGS
  "-L/library/dir"
)
set_target_properties(src_libs_sum PROPERTIES COMPILE_DEFINITIONS
  "SUMDEF"
)
set_target_properties(src_libs_sum PROPERTIES INCLUDE_DIRECTORIES
  "${CMAKE_SOURCE_DIR}/src/libs;${CMAKE_SOURCE_DIR}"
)

# //src/apps:hello
add_executable(src_apps_hello "")
target_sources(src_apps_hello
  PRIVATE
    # Sources:
    "${CMAKE_SOURCE_DIR}/src/apps/hello-world.cpp"
)
set_target_properties(src_apps_hello PROPERTIES COMPILE_FLAGS
  "-std=c++17 -Wno-psabi --target=arm64-apple-macos11 -O1"
)
set_target_properties(src_apps_hello PROPERTIES LINK_FLAGS
  "-L/app/dir"
)
set_target_properties(src_apps_hello PROPERTIES COMPILE_DEFINITIONS
  "APPDEF;SUMDEF"
)
set_target_properties(src_apps_hello PROPERTIES INCLUDE_DIRECTORIES
  "${CMAKE_SOURCE_DIR}/src/libs;${CMAKE_SOURCE_DIR}"
)
`
	buf := new(strings.Builder)
	nb, err := io.Copy(buf, contents)
	assert.Greater(t, nb, int64(0))
	assert.Equal(t, expected, buf.String())
}

func TestIsFileRule_InvalidRule_ReturnsError(t *testing.T) {
	assert.False(t, isFileRule(""))
}

func TestIsFileRule_ValidFileRule_Success(t *testing.T) {
	assert.True(t, isFileRule("//dir/path:hello.c"))
}

func TestIsFileRule_ValidNonFileRule_Success(t *testing.T) {
	assert.False(t, isFileRule("//dir/path:hello"))
}

func TestFindRule_RuleExists_Success(t *testing.T) {
	qr := &analysis_v2.CqueryResult{}
	err := proto.UnmarshalText(textProto, qr)
	require.NoError(t, err)

	r, err := findRule(qr, "//src/apps:hello")
	require.NoError(t, err)
	require.NotNil(t, r)
}

func TestFindRule_RuleDoesntExists_ReturnsError(t *testing.T) {
	qr := &analysis_v2.CqueryResult{}
	err := proto.UnmarshalText(textProto, qr)
	require.NoError(t, err)

	r, err := findRule(qr, "//path/to:nonexistent_rule")
	assert.Error(t, err)
	assert.Nil(t, r)
}

func TestFindRule_InvalidRule_ReturnsError(t *testing.T) {
	qr := &analysis_v2.CqueryResult{}
	err := proto.UnmarshalText(textProto, qr)
	require.NoError(t, err)

	r, err := findRule(qr, "")
	assert.Error(t, err)
	assert.Nil(t, r)
}

func TestGetRuleCopts_CoptsExists_Success(t *testing.T) {
	qr := &analysis_v2.CqueryResult{}
	err := proto.UnmarshalText(textProto, qr)
	require.NoError(t, err)

	r, err := findRule(qr, "//src/apps:hello")
	require.NoError(t, err)
	require.NotNil(t, r)

	copts, err := getRuleCopts(r)
	require.NoError(t, err)
	assert.Equal(t, []string{"-std=c++17", "-Wno-psabi", "--target=arm64-apple-macos11", "-O1"}, copts)
}

func TestAppendUnique_NotPresent_Appended(t *testing.T) {

	slice := appendUnique([]string{"one"}, "two")
	assert.Equal(t, []string{"one", "two"}, slice)
}

func TestAppendUnique_Present_NotAppended(t *testing.T) {

	slice := appendUnique([]string{"one", "two"}, "two")
	assert.Equal(t, []string{"one", "two"}, slice)
}
