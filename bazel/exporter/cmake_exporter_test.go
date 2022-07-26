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

func TestExport_QueryReadError_ReturnsError(t *testing.T) {

	const workspaceDir = ""
	e := NewCMakeExporter(workspaceDir)
	contents := new(bytes.Buffer)
	qcmd := mocks.NewQueryCommand(t)
	qcmd.On("Read", mock.Anything).Return([]byte{}, skerr.Fmt("expected error"))
	err := e.Export(qcmd, contents, "projName")
	assert.Error(t, err)
	assert.Equal(t, 0, contents.Len())
}

func TestExport_InvalidProtobuf_ReturnsError(t *testing.T) {

	const workspaceDir = ""
	e := NewCMakeExporter(workspaceDir)
	contents := new(bytes.Buffer)
	qcmd := mocks.NewQueryCommand(t)
	qcmd.On("Read", mock.Anything).Return(make([]byte, 50), skerr.Fmt("empty data"))
	err := e.Export(qcmd, contents, "projName")
	assert.Error(t, err)
	assert.Equal(t, 0, contents.Len())
}

func TestExport_ValidProtobuf_Success(t *testing.T) {

	const textProto = `results {
		target {
		  type: RULE
		  rule {
			name: "//src/apps:hello"
			rule_class: "cc_library"
			location: "/src/apps/BUILD.bazel:8:11"
			attribute {
			  name: "copts"
			  type: STRING_LIST
			  explicitly_specified: false
			  nodep: false
			}
			attribute {
			  name: "defines"
			  type: STRING_LIST
			  explicitly_specified: false
			  nodep: false
			}
			attribute {
			  name: "includes"
			  type: STRING_LIST
			  string_list_value: "/include/dir"
			  explicitly_specified: false
			  nodep: false
			}
			attribute {
			  name: "linkopts"
			  type: STRING_LIST
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
			  name: "hdrs"
			  type: LABEL_LIST
			  string_list_value: "//src/apps:hello-world.h"
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
		}
	  }`
	qr := &analysis_v2.CqueryResult{}
	err := proto.UnmarshalText(textProto, qr)
	require.NoError(t, err)
	protoData, err := proto.Marshal(qr)
	require.NoError(t, err)

	const workspaceDir = ""
	e := NewCMakeExporter(workspaceDir)
	contents := new(bytes.Buffer)
	qcmd := mocks.NewQueryCommand(t)
	qcmd.On("Read", mock.Anything).Return(protoData, nil)
	err = e.Export(qcmd, contents, "projName")
	require.NoError(t, err)

	const expected = `# DO NOT EDIT: This file is auto-generated.
cmake_minimum_required(VERSION 3.13)

project(projName LANGUAGES C CXX)


# //src/apps:hello
add_library(src_apps_hello "")
target_sources(src_apps_hello
  PRIVATE
    # Sources:
    "/src/apps/hello-world.cpp"
    # Headers:
    "/src/apps/hello-world.h"
)
set_target_properties(src_apps_hello PROPERTIES COMPILE_FLAGS
  "-isystem external/clang_mac/include/c++/v1 -isystem external/clang_mac/symlinks/xcode/MacSDK/usr/include -isystem external/clang_mac/lib/clang/13.0.0/include -F /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks -no-canonical-prefixes -std=c++17 -Wno-psabi --target=arm64-apple-macos11"
)
set_target_properties(src_apps_hello PROPERTIES INCLUDE_DIRECTORIES
  "/include/dir;"
)
`
	buf := new(strings.Builder)
	nb, err := io.Copy(buf, contents)
	assert.Greater(t, nb, int64(0))
	assert.Equal(t, expected, buf.String())
}
