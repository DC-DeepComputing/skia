// Copyright 2022 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package exporter

import "io"

// FileOpenFunc defines a function responsible for opening a file with
// write access identified by the absolute path.
type FileOpenFunc func(path string) (io.Writer, error)

// Exporter defines an interface for exporting the Bazel workspace
// rules to a different project format.
type Exporter interface {
	// Export will write the converted Bazel cquery response data.
	Export(qcmd QueryCommand) error
}
