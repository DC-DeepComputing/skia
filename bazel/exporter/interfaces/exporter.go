// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package interfaces

import "io"

// Exporter defines an interface for exporting the Bazel workspace
// rules to a different project format.
type Exporter interface {
	// Export will write the converted Bazel cquery response data to
	// the supplied writer with the given project name.
	Export(qcmd QueryCommand, writer io.Writer, projName string) error
}
