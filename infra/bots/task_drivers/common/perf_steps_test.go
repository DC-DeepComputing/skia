package common

import (
	"context"
	"fmt"
	"path/filepath"
	"strings"
	"testing"
	"time"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
	"go.skia.org/infra/go/gcs"
	"go.skia.org/infra/go/gcs/mocks"
	"go.skia.org/infra/go/now"

	infra_testutils "go.skia.org/infra/go/testutils"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
	"go.skia.org/skia/infra/bots/task_drivers/testutils"
)

func TestComputeBenchmarkTestRunnerCLIFlags_Success(t *testing.T) {
	const wildcard = "___wildcard___"

	test := func(name string, benchmarkInfo BenchmarkInfo, expectedFlags []string) {
		t.Run(name, func(t *testing.T) {
			expectedFlagsRegexp := ""
			for _, flag := range expectedFlags {
				if expectedFlagsRegexp != "" {
					expectedFlagsRegexp += " "
				}
				if flag == wildcard {
					expectedFlagsRegexp += "[a-z-A-Z0-9 ]+"
				} else {
					flag = strings.ReplaceAll(flag, ".", `\.`)
					flag = strings.ReplaceAll(flag, "/", `\/`)
					flag = strings.ReplaceAll(flag, "+", `\+`)
					expectedFlagsRegexp += flag
				}
			}

			actualFlags := ComputeBenchmarkTestRunnerCLIFlags(benchmarkInfo)
			actualFlagsStr := strings.Join(actualFlags, " ")

			fmt.Printf("The regexp is:\n%s\n", expectedFlagsRegexp)

			assert.Regexp(t, expectedFlagsRegexp, actualFlagsStr)
		})
	}

	test("post-submit task", BenchmarkInfo{
		GitCommit: "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
		TaskName:  "BazelTest-Foo-Bar",
		TaskID:    "1234567890",
	}, []string{
		"--key",
		wildcard, // Only check that some key is provided (it depends on the OS, CPU, etc.).
		"--gitHash",
		"ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
		"--links",
		"task",
		"https://task-scheduler.skia.org/task/1234567890",
	})

	test("CL task", BenchmarkInfo{
		GitCommit:     "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
		TaskName:      "BazelTest-Foo-Bar",
		TaskID:        "1234567890",
		ChangelistID:  "12345",
		PatchsetOrder: "3",
	}, []string{
		"--key",
		wildcard, // Only check that some key is provided (it depends on the OS, CPU, etc.).
		"--gitHash",
		"ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
		"--links",
		"task",
		"https://task-scheduler.skia.org/task/1234567890",
		"changelist",
		"https://skia-review.googlesource.com/c/skia/+/12345/3",
	})
}

func TestUploadToPerf_NoOutputsZIPOrDir_NothingUploaded(t *testing.T) {
	test := func(name string, benchmarkInfo BenchmarkInfo) {
		t.Run(name, func(t *testing.T) {
			gcsClient := mocks.NewGCSClient(t)
			res := td.RunTestSteps(t, false, func(ctx context.Context) error {
				err := UploadToPerf(ctx, gcsClient, benchmarkInfo, "/no/such/outputs.zip")
				assert.NoError(t, err)
				return err
			})

			require.Empty(t, res.Errors)
			require.Empty(t, res.Exceptions)
			testutils.AssertStepNames(t, res,
				"Test did not produce an undeclared test outputs ZIP file or directory; nothing to upload to Perf",
			)

			gcsClient.AssertNotCalled(t, "Bucket")
			gcsClient.AssertNotCalled(t, "SetFileContents")
			gcsClient.AssertExpectations(t)
		})
	}

	test("post-submit task", BenchmarkInfo{
		GitCommit: "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
		TaskName:  "BazelTest-Foo-Bar",
		TaskID:    "1234567890",
	})

	test("CL task", BenchmarkInfo{
		GitCommit:     "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
		TaskName:      "BazelTest-Foo-Bar",
		TaskID:        "1234567890",
		ChangelistID:  "12345",
		PatchsetOrder: "3",
	})
}

func TestUploadToPerf_NoResultsJSONFile_NothingUploaded(t *testing.T) {
	test := func(name string, zip bool, benchmarkInfo BenchmarkInfo) {
		t.Run(name, func(t *testing.T) {
			undeclaredTestOutputs := map[string]string{
				// Does not contain a results.json file. File contents do not matter for this test.
				"some-image.png":     "fake PNG",
				"some-plaintext.txt": "fake TXT",
			}

			// Write undeclared test outputs to disk.
			outputsZIPOrDir := ""
			if zip {
				outputsZIPOrDir = filepath.Join(t.TempDir(), "outputs.zip")
				testutils.MakeZIP(t, outputsZIPOrDir, undeclaredTestOutputs)
			} else {
				outputsZIPOrDir = t.TempDir()
				testutils.PopulateDir(t, outputsZIPOrDir, undeclaredTestOutputs)
			}

			// Will be returned by the mocked os_steps.TempDir() when the task driver tries to create a
			// directory in which to extract the undeclared outputs ZIP archive.
			outputsZIPExtractionDir := outputsZIPOrDir
			if zip {
				outputsZIPExtractionDir = t.TempDir()
			}

			gcsClient := mocks.NewGCSClient(t)
			res := td.RunTestSteps(t, false, func(ctx context.Context) error {
				if zip {
					// Mock os_steps.TempDir() only for the case where outpusZIPOrDir is a ZIP archive. We
					// don't need to assert the exact number of times that os_steps.TempDir() is called
					// because said function produces a "Creating TempDir" task driver step, and we check the
					// exact set of steps produced.
					ctx = context.WithValue(ctx, os_steps.TempDirContextKey, testutils.MakeTempDirMockFn(t, outputsZIPExtractionDir))
				}

				err := UploadToPerf(ctx, gcsClient, benchmarkInfo, outputsZIPOrDir)
				assert.Error(t, err)
				assert.Contains(t, err.Error(), "stat "+outputsZIPExtractionDir+"/results.json: no such file or directory")
				return err
			})

			require.Empty(t, res.Errors)
			require.Empty(t, res.Exceptions)

			expectedSteps := []string{}
			if zip {
				expectedSteps = append(expectedSteps,
					"Creating TempDir",
					"Extract undeclared outputs archive "+outputsZIPOrDir+" into "+outputsZIPExtractionDir,
					"Extracting file: some-image.png",
					"Not extracting non-PNG / non-JSON file: some-plaintext.txt",
				)
			}
			expectedSteps = append(expectedSteps,
				"Stat "+outputsZIPExtractionDir+"/results.json",
			)
			testutils.AssertStepNames(t, res, expectedSteps...)

			gcsClient.AssertNotCalled(t, "Bucket")
			gcsClient.AssertNotCalled(t, "SetFileContents")
			gcsClient.AssertExpectations(t)
		})
	}

	postSubmitTaskBenchmarkInfo := BenchmarkInfo{
		GitCommit: "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
		TaskName:  "BazelTest-Foo-Bar",
		TaskID:    "1234567890",
	}
	test("post-submit task, ZIP file", true /* =zip */, postSubmitTaskBenchmarkInfo)
	test("post-submit task, directory", false /* =zip */, postSubmitTaskBenchmarkInfo)

	clTaskBenchmarkInfo := BenchmarkInfo{
		GitCommit:     "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
		TaskName:      "BazelTest-Foo-Bar",
		TaskID:        "1234567890",
		ChangelistID:  "12345",
		PatchsetOrder: "3",
	}
	test("CL task, ZIP file", true /* =zip */, clTaskBenchmarkInfo)
	test("CL task, directory", false /* =zip */, clTaskBenchmarkInfo)
}

func TestUploadToPerf_Success(t *testing.T) {
	test := func(name string, zip bool, benchmarkInfo BenchmarkInfo) {
		t.Run(name, func(t *testing.T) {
			resultsJSONFileContents := `{"foo": "this test requires that this file exists; its contents do not matter"}`
			undeclaredTestOutputs := map[string]string{
				"results.json":       resultsJSONFileContents,
				"some-image.png":     "fake PNG",
				"some-plaintext.txt": "fake TXT",
			}

			// Write undeclared test outputs to disk.
			outputsZIPOrDir := ""
			if zip {
				outputsZIPOrDir = filepath.Join(t.TempDir(), "outputs.zip")
				testutils.MakeZIP(t, outputsZIPOrDir, undeclaredTestOutputs)
			} else {
				outputsZIPOrDir = t.TempDir()
				testutils.PopulateDir(t, outputsZIPOrDir, undeclaredTestOutputs)
			}

			// Will be returned by the mocked os_steps.TempDir() when the task driver tries to create a
			// directory in which to extract the undeclared outputs ZIP archive.
			outputsZIPExtractionDir := outputsZIPOrDir
			if zip {
				outputsZIPExtractionDir = t.TempDir()
			}

			gcsClient := mocks.NewGCSClient(t)
			gcsClient.On("Bucket").Return("skia-perf").Once()
			gcsClient.On(
				"SetFileContents",
				infra_testutils.AnyContext,
				"nano-json-v1/2022/01/31/01/ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99/BazelTest-Foo-Bar/results_1234567890.json",
				gcs.FILE_WRITE_OPTS_TEXT,
				[]byte(resultsJSONFileContents)).
				Return(nil).Once()

			res := td.RunTestSteps(t, false, func(ctx context.Context) error {
				fakeNow := time.Date(2022, time.January, 31, 2, 2, 3, 0, time.FixedZone("UTC+1", 60*60))
				ctx = now.TimeTravelingContext(fakeNow).WithContext(ctx)

				if zip {
					// Mock os_steps.TempDir() only for the case where outpusZIPOrDir is a ZIP archive. We
					// don't need to assert the exact number of times that os_steps.TempDir() is called
					// because said function produces a "Creating TempDir" task driver step, and we check the
					// exact set of steps produced.
					ctx = context.WithValue(ctx, os_steps.TempDirContextKey, testutils.MakeTempDirMockFn(t, outputsZIPExtractionDir))
				}

				err := UploadToPerf(ctx, gcsClient, benchmarkInfo, outputsZIPOrDir)
				assert.NoError(t, err)
				return err
			})

			require.Empty(t, res.Errors)
			require.Empty(t, res.Exceptions)

			expectedSteps := []string{}
			if zip {
				expectedSteps = append(expectedSteps,
					"Creating TempDir",
					"Extract undeclared outputs archive "+outputsZIPOrDir+" into "+outputsZIPExtractionDir,
					"Extracting file: results.json",
					"Extracting file: some-image.png",
					"Not extracting non-PNG / non-JSON file: some-plaintext.txt",
				)
			}
			expectedSteps = append(expectedSteps,
				"Stat "+outputsZIPExtractionDir+"/results.json",
				"Read "+outputsZIPExtractionDir+"/results.json",
				"Upload gs://skia-perf/nano-json-v1/2022/01/31/01/ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99/BazelTest-Foo-Bar/results_1234567890.json",
			)
			testutils.AssertStepNames(t, res, expectedSteps...)

			gcsClient.AssertExpectations(t)
		})
	}

	postSubmitTaskBenchmarkInfo := BenchmarkInfo{
		GitCommit: "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
		TaskName:  "BazelTest-Foo-Bar",
		TaskID:    "1234567890",
	}
	test("post-submit task, ZIP file", true /* =zip */, postSubmitTaskBenchmarkInfo)
	test("post-submit task, directory", false /* =zip */, postSubmitTaskBenchmarkInfo)

	clTaskBenchmarkInfo := BenchmarkInfo{
		GitCommit:     "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
		TaskName:      "BazelTest-Foo-Bar",
		TaskID:        "1234567890",
		ChangelistID:  "12345",
		PatchsetOrder: "3",
	}
	test("CL task, ZIP file", true /* =zip */, clTaskBenchmarkInfo)
	test("CL task, directory", false /* =zip */, clTaskBenchmarkInfo)
}
