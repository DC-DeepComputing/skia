// Copyright 2022 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package exporter

import (
	"bytes"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
	"go.skia.org/infra/go/skerr"
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
