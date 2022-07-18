// Copyright 2022 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package exporter

import (
	"go.skia.org/infra/go/skerr"
)

type FileQueryReader struct {
}

func NewFileQueryReader(dir string) (*FileQueryReader, error) {
	return &FileQueryReader{}, nil
}

func (q *FileQueryReader) Read() ([]byte, error) {
	return nil, skerr.Fmt("unimplemented")
}
