// Copyright 2023 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package common

import (
	"context"
	"errors"
	"os"
	"path/filepath"

	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/go/util"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
)

const resultsJSON = "results.json"

func UploadToPerf(ctx context.Context, outputsZIPOrDir string) error {
	// Were there any undeclared test outputs?
	fileInfo, err := os.Stat(outputsZIPOrDir)
	if err != nil {
		if errors.Is(err, os.ErrNotExist) {
			return td.Do(ctx, td.Props("Test did not produce an undeclared test outputs ZIP file or directory; nothing to upload to Perf"), func(ctx context.Context) error {
				return nil
			})
		} else {
			return skerr.Wrap(err)
		}
	}

	// If the undeclared outputs ZIP file or directory is a ZIP file, extract it.
	outputsDir := ""
	if fileInfo.IsDir() {
		outputsDir = outputsZIPOrDir
	} else {
		var err error
		outputsDir, err = ExtractOutputsZip(ctx, outputsZIPOrDir)
		if err != nil {
			return skerr.Wrap(err)
		}
		defer util.RemoveAll(outputsDir)
	}

	resultsJSONPath := filepath.Join(outputsDir, resultsJSON)
	fileInfo, err = os_steps.Stat(ctx, resultsJSONPath)
	if err != nil {
		return skerr.Wrap(err)
	}
	if fileInfo.IsDir() {
		return skerr.Fmt("file %q is a directory", resultsJSONPath)
	}

	// TODO(lovisolo): Actually upload results.json to Perf.

	return nil
}
