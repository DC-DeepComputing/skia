// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This script mirrors the Unicode data files (e.g. https://www.unicode.org/Public/15.0.0/ucd/BidiBrackets.txt)
// to Skia's public GCS bucket (skia-cdn) so we can more reliably download them in a hermetic,
// reproducible way. The file's source of truth is stored on GitHub, so this clones the repo,
// checks it out to a known version, and uploads the data files corresponding to a specific
// version to the bucket. These can be downloaded from the bucket using links like the following:
// https://storage.googleapis.com/skia-cdn/unicodetools/15.0.0/ucd/BidiBrackets.txt
package main

import (
	"context"
	"fmt"
	"io/fs"
	"os"
	"os/exec"
	"path"
	"path/filepath"
	"strings"
)

const (
	unicodeRepo     = "https://github.com/unicode-org/unicodetools.git"
	unicodeRevision = "73717919230ad43616b021fd9fe2950b8774782a"

	version = "15.0.0"
)

func main() {
	cloneDir, err := os.MkdirTemp("", "")
	if err != nil {
		die("Could not make clone dir %s\n", err)
	}
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	cmd := exec.CommandContext(ctx, "git", "clone", unicodeRepo)
	cmd.Dir = cloneDir
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	if err := cmd.Run(); err != nil {
		die("Error cloning %s: %s\n", unicodeRepo, err)
	}
	cloneDir = filepath.Join(cloneDir, "unicodetools")
	fmt.Printf("Cloned to %s\n", cloneDir)

	cmd = exec.CommandContext(ctx, "git", "checkout", unicodeRevision)
	cmd.Dir = cloneDir
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	if err := cmd.Run(); err != nil {
		die("Error checking out %s: %s\n", unicodeRevision, err)
	}

	if err := upload(ctx, filepath.Join(cloneDir, "LICENSE"), "LICENSE"); err != nil {
		die("Could not upload LICENSE %s\n", err)
	}
	targetDir := filepath.Join(cloneDir, "unicodetools", "data", "ucd", version)
	err = filepath.WalkDir(targetDir, func(walkedPath string, d fs.DirEntry, err error) error {
		if d.IsDir() {
			return nil
		}
		targetParts := strings.Split(strings.TrimPrefix(walkedPath, targetDir), string(os.PathSeparator))
		// Intentionally using path here because GCS only uses /
		dstPath := path.Join(targetParts...)
		return upload(ctx, walkedPath, dstPath)
	})
	if err != nil {
		die("Error while uploading %s", err)
	}
}

func upload(ctx context.Context, src, dst string) error {
	bucketDst := fmt.Sprintf("gs://skia-cdn/unicodetools/%s/ucd/%s", version, dst)
	fmt.Printf("Uploading %s to %s\n", src, dst)
	cmd := exec.CommandContext(ctx, "gsutil", "cp", "-Z", src, bucketDst)
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	return cmd.Run()
}

func die(msg string, a ...interface{}) {
	_, _ = fmt.Fprintf(os.Stderr, msg, a...)
	os.Exit(1)
}
