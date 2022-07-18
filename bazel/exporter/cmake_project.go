// Copyright 2022 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package exporter

import (
	"bytes"
)

// CMakeWorkspace represents the entire state of a set of CMake project files
// that comprise a project.
type CMakeProject struct {
	Dir      string       // The workspace relative directory path.
	contents bytes.Buffer // The contents of the CMake project file.
}

// Append will append the supplied chunk of a CMake projet file to this
// projects contents.
func (w *CMakeProject) Append(data []byte) {
	if w.contents.Len() != 0 {
		w.contents.WriteString("\n\n")
	}
	w.contents.Write(data)
}

// GetContents will return the entire project contents as byte array.
func (w *CMakeProject) GetContents() []byte {
	return w.contents.Bytes()
}

func (w *CMakeProject) GetString() string {
	return w.contents.String()
}
