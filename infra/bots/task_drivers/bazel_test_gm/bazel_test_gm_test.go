// Copyright 2023 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"archive/zip"
	"bytes"
	"context"
	"fmt"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
	"go.skia.org/infra/go/exec"
	"go.skia.org/infra/task_driver/go/td"
	"os"
	"path/filepath"
	"sort"
	"testing"
)

func TestValidateLabelAndReturnOutputZipPath_ValidLabel_Success(t *testing.T) {
	test := func(label, expected string) {
		t.Run(label, func(t *testing.T) {
			actual, err := validateLabelAndReturnOutputsZipPath("/path/to/skia", label)
			require.NoError(t, err)
			assert.Equal(t, expected, actual)
		})
	}

	test("//:foo", "/path/to/skia/bazel-testlogs/foo/test.outputs/outputs.zip")
	test("//foo:bar", "/path/to/skia/bazel-testlogs/foo/bar/test.outputs/outputs.zip")
	test("//foo/bar:baz", "/path/to/skia/bazel-testlogs/foo/bar/baz/test.outputs/outputs.zip")
	test("//foo/bar/baz:qux", "/path/to/skia/bazel-testlogs/foo/bar/baz/qux/test.outputs/outputs.zip")
}

func TestValidateLabelAndReturnOutputZipPath_InvalidLabel_Error(t *testing.T) {
	test := func(label string) {
		t.Run(label, func(t *testing.T) {
			_, err := validateLabelAndReturnOutputsZipPath("/path/to/skia", label)
			require.Error(t, err)
			assert.Contains(t, err.Error(), fmt.Sprintf("invalid label: %q", label))
		})
	}

	test("foo")
	test("/foo")
	test("//foo")
	test(":foo")
	test("/:foo")

	test("foo/bar")
	test("foo:bar")
	test("/foo/bar")
	test("/foo:bar")
	test(":foo/bar")
	test(":foo:bar")
	test("//foo/bar")

	test("foo/bar/baz")
	test("foo/bar:baz")
	test("foo:bar/baz")
	test("foo:bar:baz")
	test("/foo/bar/baz")
	test("/foo/bar:baz")
	test("/foo:bar/baz")
	test("/foo:bar:baz")
	test("//foo/bar/baz")
	test("//foo:bar/baz")
	test("//foo:bar:baz")
}

func TestRun_NoOutputsZip_Success(t *testing.T) {
	test := func(cl bool) {
		name := "post-submit task"
		if cl {
			name = "CL task"
		}
		t.Run(name, func(t *testing.T) {
			goldctlBazelLabelAllowList["//some/test:target"] = true
			defer func() { goldctlBazelLabelAllowList["//some/test:target"] = false }()

			commandCollector := exec.CommandCollector{}
			res := td.RunTestSteps(t, false, func(ctx context.Context) error {
				ctx = td.WithExecRunFn(ctx, commandCollector.Run)

				tdArgs := taskDriverArgs{
					checkoutDir: "/path/to/skia",
					bazelLabel:  "//some/test:target",
					bazelConfig: "linux_rbe",
					goldctlPath: "/path/to/goldctl",
					gitCommit:   "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
				}
				if cl {
					// These arguments are only used when there are images to upload, and are therefore
					// ignored by the task driver under this test.
					tdArgs.changelistID = "changelist-id"
					tdArgs.patchsetOrder = "1"
					tdArgs.tryjobID = "tryjob-id"
				}

				err := run(ctx, tdArgs)

				assert.NoError(t, err)
				return err
			})

			require.Empty(t, res.Errors)
			require.Empty(t, res.Exceptions)
			assertStepNamesMatchRegexps(t, res,
				"^Test //some/test:target with config linux_rbe$",
				"^bazelisk test //some/test:target --config=linux_rbe --test_output=errors --jobs=100$",
				"^Test did not produce an undeclared test outputs ZIP file; nothing to upload to Gold$",
			)

			require.Len(t, commandCollector.Commands(), 1)
			cmd := commandCollector.Commands()[0]
			assert.Equal(t, "/path/to/skia", cmd.Dir)
			assert.Equal(t, "bazelisk", cmd.Name)
			assert.Equal(t, []string{
				"test",
				"//some/test:target",
				"--config=linux_rbe",
				"--test_output=errors",
				"--jobs=100",
			}, cmd.Args)
		})
	}

	test(false /* =cl */)
	test(true /* =cl */)
}

func TestRun_WithOutputsZip_NoValidImages_NoGoldctlInvocations(t *testing.T) {
	test := func(cl bool) {
		name := "post-submit task"
		if cl {
			name = "CL task"
		}
		t.Run(name, func(t *testing.T) {
			goldctlBazelLabelAllowList["//some/test:target"] = true
			defer func() { goldctlBazelLabelAllowList["//some/test:target"] = false }()

			checkoutDir := t.TempDir()

			// Create fake archive with undeclared test outputs.
			outputsZipPath := filepath.Join(checkoutDir, "bazel-testlogs", "some", "test", "target", "test.outputs", "outputs.zip")
			require.NoError(t, os.MkdirAll(filepath.Dir(outputsZipPath), 0700))
			makeZip(t, outputsZipPath, map[string]string{
				// The contents of PNG files does not matter for this test.
				"image-with-invalid-json-file.png": "fake PNG",
				"image-with-invalid-json-file.json": `{
					"invalid JSON file": "This JSON file should be ignored"
				}`,
				"image-with-no-json-file.png":    "fake PNG",
				"not-an-image-nor-json-file.txt": "I'm neither a PNG nor a JSON file.",
			})

			commandCollector := exec.CommandCollector{}
			res := td.RunTestSteps(t, false, func(ctx context.Context) error {
				ctx = td.WithExecRunFn(ctx, commandCollector.Run)

				tdArgs := taskDriverArgs{
					checkoutDir: checkoutDir,
					bazelLabel:  "//some/test:target",
					bazelConfig: "linux_rbe",
					goldctlPath: "/path/to/goldctl",
					gitCommit:   "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
				}
				if cl {
					// These arguments are only used when there are images to upload, and are therefore
					// ignored by the task driver under this test.
					tdArgs.changelistID = "changelist-id"
					tdArgs.patchsetOrder = "1"
					tdArgs.tryjobID = "tryjob-id"
				}

				err := run(ctx, tdArgs)

				assert.NoError(t, err)
				return err
			})

			require.Empty(t, res.Errors)
			require.Empty(t, res.Exceptions)

			assertStepNamesMatchRegexps(t, res,
				"^Test //some/test:target with config linux_rbe$",
				"^bazelisk test //some/test:target --config=linux_rbe --test_output=errors --jobs=100$",
				"^Extract undeclared outputs archive .+/bazel-testlogs/some/test/target/test.outputs/outputs.zip into .+$",
				"^Extracting file: image-with-invalid-json-file.json$",
				"^Extracting file: image-with-invalid-json-file.png$",
				"^Extracting file: image-with-no-json-file.png$",
				"^Ignoring non-PNG / non-JSON file: not-an-image-nor-json-file.txt$",
				"Gather JSON and PNG files produced by GMs",
				"^Ignoring \"image-with-invalid-json-file.json\": key \"image_md5\" not found$",
				"^Undeclared test outputs ZIP file contains no GM outptus; nothing to upload to Gold$",
			)

			require.Len(t, commandCollector.Commands(), 1)

			cmd := commandCollector.Commands()[0]
			assert.Equal(t, checkoutDir, cmd.Dir)
			assert.Equal(t, "bazelisk", cmd.Name)
			assert.Equal(t, []string{
				"test",
				"//some/test:target",
				"--config=linux_rbe",
				"--test_output=errors",
				"--jobs=100",
			}, cmd.Args)
		})
	}

	test(false /* =cl */)
	test(true /* =cl */)
}

func TestRun_WithOutputsZip_ValidImages_ImagesUploadedToGold(t *testing.T) {
	test := func(cl bool) {
		name := "post-submit task"
		if cl {
			name = "CL task"
		}
		t.Run(name, func(t *testing.T) {
			goldctlBazelLabelAllowList["//some/test:target"] = true
			defer func() { goldctlBazelLabelAllowList["//some/test:target"] = false }()

			checkoutDir := t.TempDir()

			// Create fake archive with undeclared test outputs.
			outputsZipPath := filepath.Join(checkoutDir, "bazel-testlogs", "some", "test", "target", "test.outputs", "outputs.zip")
			require.NoError(t, os.MkdirAll(filepath.Dir(outputsZipPath), 0700))
			makeZip(t, outputsZipPath, map[string]string{
				// The contents of PNG files does not matter for this test.
				"alfa.png": "fake PNG",
				"alfa.json": `{
					"build_system": "bazel",
					"name": "alfa",
					"image_md5": "a01a01a01a01a01a01a01a01a01a01a0",
					"source_type": "gm"
				}`,
				"beta.png": "fake PNG",
				"beta.json": `{
					"build_system": "bazel",
					"name": "beta",
					"image_md5": "b02b02b02b02b02b02b02b02b02b02b0",
					"source_type": "gm"
				}`,
				"image-with-invalid-json-file.png": "fake PNG",
				"image-with-invalid-json-file.json": `{
					"invalid JSON file": "This JSON file should be ignored"
				}`,
				"image-with-no-json-file.png":    "fake PNG",
				"not-an-image-nor-json-file.txt": "I'm neither a PNG nor a JSON file.",
			})

			commandCollector := exec.CommandCollector{}
			res := td.RunTestSteps(t, false, func(ctx context.Context) error {
				ctx = td.WithExecRunFn(ctx, commandCollector.Run)

				tdArgs := taskDriverArgs{
					checkoutDir: checkoutDir,
					bazelLabel:  "//some/test:target",
					bazelConfig: "linux_rbe",
					goldctlPath: "/path/to/goldctl",
					gitCommit:   "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
				}
				if cl {
					tdArgs.changelistID = "changelist-id"
					tdArgs.patchsetOrder = "1"
					tdArgs.tryjobID = "tryjob-id"
				}

				err := run(ctx, tdArgs)

				assert.NoError(t, err)
				return err
			})

			require.Empty(t, res.Errors)
			require.Empty(t, res.Exceptions)

			goldctlImgtestInitStepRegexp := "^/path/to/goldctl imgtest init --work-dir [a-zA-Z0-9-/]+ --instance skia --url https://gold.skia.org --bucket skia-infra-gm --git_hash ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99 --key os:linux$"
			if cl {
				goldctlImgtestInitStepRegexp = "^/path/to/goldctl imgtest init --work-dir [a-zA-Z0-9-/]+ --instance skia --url https://gold.skia.org --bucket skia-infra-gm --git_hash ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99 --crs gerrit --cis buildbucket --changelist changelist-id --patchset 1 --jobid tryjob-id --key os:linux$"
			}
			assertStepNamesMatchRegexps(t, res,
				"^Test //some/test:target with config linux_rbe$",
				"^bazelisk test //some/test:target --config=linux_rbe --test_output=errors --jobs=100$",
				"^Extract undeclared outputs archive .+/bazel-testlogs/some/test/target/test.outputs/outputs.zip into .+$",
				"^Extracting file: alfa.json$",
				"^Extracting file: alfa.png$",
				"^Extracting file: beta.json$",
				"^Extracting file: beta.png$",
				"^Extracting file: image-with-invalid-json-file.json$",
				"^Extracting file: image-with-invalid-json-file.png$",
				"^Extracting file: image-with-no-json-file.png$",
				"^Ignoring non-PNG / non-JSON file: not-an-image-nor-json-file.txt$",
				"Gather JSON and PNG files produced by GMs",
				"^Gather \"alfa.png\"$",
				"^Gather \"beta.png\"$",
				"^Ignoring \"image-with-invalid-json-file.json\": key \"image_md5\" not found$",
				"^Upload GM outputs to Gold$",
				"^Creating TempDir$",
				"^/path/to/goldctl auth --work-dir [a-zA-Z0-9-/]+ --luci$",
				goldctlImgtestInitStepRegexp,
				"^/path/to/goldctl imgtest add --work-dir [a-zA-Z0-9-/]+ --test-name alfa --png-file [a-zA-Z0-9-/]+/alfa.png --png-digest a01a01a01a01a01a01a01a01a01a01a0 --add-test-key build_system:bazel --add-test-key name:alfa --add-test-key source_type:gm$",
				"^/path/to/goldctl imgtest add --work-dir [a-zA-Z0-9-/]+ --test-name beta --png-file [a-zA-Z0-9-/]+/beta.png --png-digest b02b02b02b02b02b02b02b02b02b02b0 --add-test-key build_system:bazel --add-test-key name:beta --add-test-key source_type:gm$",
				"^/path/to/goldctl imgtest finalize --work-dir [a-zA-Z0-9-/]+$",
			)

			require.Len(t, commandCollector.Commands(), 6)

			cmd := commandCollector.Commands()[0]
			assert.Equal(t, checkoutDir, cmd.Dir)
			assert.Equal(t, "bazelisk", cmd.Name)
			assert.Equal(t, []string{
				"test",
				"//some/test:target",
				"--config=linux_rbe",
				"--test_output=errors",
				"--jobs=100",
			}, cmd.Args)

			cmd = commandCollector.Commands()[1]
			assert.Equal(t, checkoutDir, cmd.Dir)
			assert.Equal(t, "/path/to/goldctl", cmd.Name)
			assertSliceMatchesRegexps(t, cmd.Args,
				"^auth$",
				"^--work-dir$",
				"^[a-zA-Z0-9-/]+$",
				"^--luci$",
			)

			cmd = commandCollector.Commands()[2]
			assert.Equal(t, checkoutDir, cmd.Dir)
			assert.Equal(t, "/path/to/goldctl", cmd.Name)
			args := []string{
				"^imgtest$",
				"^init$",
				"^--work-dir$",
				"^[a-zA-Z0-9-/]+$",
				"^--instance$",
				"^skia$",
				"^--url$",
				`^https://gold\.skia\.org$`,
				"^--bucket$",
				"^skia-infra-gm$",
				"^--git_hash$",
				"^ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99$",
			}
			if cl {
				args = append(args,
					"^--crs$",
					"^gerrit$",
					"^--cis$",
					"^buildbucket$",
					"^--changelist$",
					"^changelist-id$",
					"^--patchset$",
					"^1$",
					"^--jobid$",
					"^tryjob-id$",
				)
			}
			args = append(args,
				"^--key$",
				"^os:linux$",
			)
			assertSliceMatchesRegexps(t, cmd.Args, args...)

			cmd = commandCollector.Commands()[3]
			assert.Equal(t, checkoutDir, cmd.Dir)
			assert.Equal(t, "/path/to/goldctl", cmd.Name)
			assertSliceMatchesRegexps(t, cmd.Args,
				"^imgtest$",
				"^add$",
				"^--work-dir$",
				"^[a-zA-Z0-9-/]+$",
				"^--test-name$",
				"^alfa$",
				"^--png-file$",
				"^[a-zA-Z0-9-/]+/alfa.png$",
				"^--png-digest$",
				"^a01a01a01a01a01a01a01a01a01a01a0$",
				"^--add-test-key$",
				"^build_system:bazel$",
				"^--add-test-key$",
				"^name:alfa$",
				"^--add-test-key$",
				"^source_type:gm$",
			)

			cmd = commandCollector.Commands()[4]
			assert.Equal(t, checkoutDir, cmd.Dir)
			assert.Equal(t, "/path/to/goldctl", cmd.Name)
			assertSliceMatchesRegexps(t, cmd.Args,
				"^imgtest$",
				"^add$",
				"^--work-dir$",
				"^[a-zA-Z0-9-/]+$",
				"^--test-name$",
				"^beta$",
				"^--png-file$",
				"^[a-zA-Z0-9-/]+/beta.png$",
				"^--png-digest$",
				"^b02b02b02b02b02b02b02b02b02b02b0$",
				"^--add-test-key$",
				"^build_system:bazel$",
				"^--add-test-key$",
				"^name:beta$",
				"^--add-test-key$",
				"^source_type:gm$",
			)

			cmd = commandCollector.Commands()[5]
			assert.Equal(t, checkoutDir, cmd.Dir)
			assert.Equal(t, "/path/to/goldctl", cmd.Name)
			assertSliceMatchesRegexps(t, cmd.Args,
				"^imgtest$",
				"^finalize$",
				"^--work-dir$",
				"^[a-zA-Z0-9-/]+$",
			)
		})
	}

	test(false /* =cl */)
	test(true /* =cl */)
}

// assertStepNamesMatchRegexps flattens the names of the steps in the given report, then asserts
// that they match the given regular expressions.
func assertStepNamesMatchRegexps(t *testing.T, report *td.StepReport, expectedRegexps ...string) {
	var actualStepNames []string
	report.Recurse(func(sr *td.StepReport) bool {
		if sr == report {
			// The root step created by td.RunTestSteps() is always "fake-test-task". We can omit it.
			return true
		}
		actualStepNames = append(actualStepNames, sr.Name)
		return true
	})

	assertSliceMatchesRegexps(t, actualStepNames, expectedRegexps...)
}

// assertSliceMatchesRegexps asserts that the elements in the given slice match the given regular
// expressions.
func assertSliceMatchesRegexps(t *testing.T, actual []string, expectedRegexps ...string) {
	require.Len(t, actual, len(expectedRegexps), "actual slice: %#v", actual)
	for i, expected := range expectedRegexps {
		assert.Regexp(t, expected, actual[i], "offending item index: %d", i)
	}
}

// makeZip creates a ZIP archive with the given contents, represented as a map from file paths to
// file contents.
func makeZip(t *testing.T, outputZipPath string, contents map[string]string) {
	buf := &bytes.Buffer{}
	zipWriter := zip.NewWriter(buf)

	// Sort files before adding them to the archive for determinism.
	var sortedFiles []string
	for file := range contents {
		sortedFiles = append(sortedFiles, file)
	}
	sort.Strings(sortedFiles)

	for _, file := range sortedFiles {
		fileWriter, err := zipWriter.Create(file)
		require.NoError(t, err)
		_, err = fileWriter.Write([]byte(contents[file]))
		require.NoError(t, err)
	}

	require.NoError(t, zipWriter.Close())
	require.NoError(t, os.WriteFile(outputZipPath, buf.Bytes(), 0644))
}
