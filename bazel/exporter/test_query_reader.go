// Copyright 2022 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package exporter

import (
	"io/ioutil"

	"go.skia.org/infra/go/skerr"
)

type TestQueryReader struct {
	fileData []byte
}

// NewTestQueryReader will create a new TestQueryReader for reading
// test query results from a file.
func NewTestQueryReader(pbFile string) (*TestQueryReader, error) {
	fileData, err := ioutil.ReadFile(pbFile)
	if err != nil {
		return nil, skerr.Wrap(err)
	}
	return &TestQueryReader{fileData: fileData}, nil
}

func (q *TestQueryReader) Read() ([]byte, error) {
	return q.fileData, nil
}
