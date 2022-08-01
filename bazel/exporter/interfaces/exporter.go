// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package interfaces

import "io"

// FileOpenFunc defines a function responsible for opening a file with
// write access identified by the absolute path.
type FileOpenFunc func(path string) (io.Writer, error)

// Exporter defines an interface for exporting the Bazel workspace
// rules to a different project format.
type Exporter interface {
	// Export will write the converted Bazel cquery response data to
	// a new project format.
	Export(qcmd QueryCommand) error
}
