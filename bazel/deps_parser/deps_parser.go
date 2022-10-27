// Copyright 2022 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"flag"
	"fmt"
	"io"
	"os"
	"regexp"
	"strings"
)

type depConfig struct {
	bazelNameOverride string
	needsBazelFile    bool
}

var deps = map[string]depConfig{
	"dawn":    {needsBazelFile: true},
	"dng_sdk": {needsBazelFile: true},
	"expat":   {needsBazelFile: true},
}

func main() {
	var (
		depsFile   = flag.String("file", "DEPS", "The location of the DEPS file. Usually at the root of the repository")
		genBzlFile = flag.String("gen_bzl_file", "bazel/deps.bzl", "The location of the .bzl file that has the generated Bazel repository rules.")
	)
	flag.Parse()

	b, err := os.ReadFile(*depsFile)
	if err != nil {
		fmt.Printf("Could not open %s: %s\n", *depsFile, err)
		os.Exit(1)
	}
	contents := strings.Split(string(b), "\n")

	depsLine := regexp.MustCompile(`externals/(\S+)".+"(https.+)@([a-f0-9]+)"`)

	outputFile, err := os.Create(*genBzlFile)
	if err != nil {
		fmt.Printf("Could not create output file %s: %s\n", *genBzlFile, err)
		os.Exit(1)
	}
	defer outputFile.Close()

	if _, err := outputFile.WriteString(header); err != nil {
		fmt.Printf("Could not write to output file %s: %s\n", *genBzlFile, err)
		os.Exit(1)
	}

	count := 0
	for _, line := range contents {
		if match := depsLine.FindStringSubmatch(line); len(match) > 0 {
			id := match[1]
			repo := match[2]
			rev := match[3]

			cfg, ok := deps[id]
			if !ok {
				continue
			}
			if cfg.bazelNameOverride != "" {
				id = cfg.bazelNameOverride
			}
			if cfg.needsBazelFile {
				if err := writeNewGitRepositoryRule(outputFile, id, repo, rev); err != nil {
					fmt.Printf("Could not write to output file %s: %s\n", *genBzlFile, err)
					os.Exit(1)
				}
			}

			count++
		}
	}
	fmt.Printf("Wrote %d deps\n", count)
}

const header = `"""
This file is auto-generated from //bazel/deps_parser.
DO NOT MODIFY BY HAND.
"""

load("@bazel_tools//tools/build_defs/repo:git.bzl", "new_git_repository")

def git_repos_from_deps():
    # https://bazel.build/rules/lib/repo/git`

func writeNewGitRepositoryRule(w io.StringWriter, bazelName, repo, rev string) error {
	// TODO(kjlubick) In a newer version of Bazel, new_git_repository can be replaced with just
	// git_repository
	_, err := w.WriteString(fmt.Sprintf(`
    new_git_repository(
        name = "%s",
        build_file = "@//bazel/external/%s:BUILD.bazel",
        commit = "%s",
        remote = "%s",
    )
`, bazelName, bazelName, rev, repo))
	return err
}
