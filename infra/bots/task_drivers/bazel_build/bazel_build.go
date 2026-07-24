// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This executable runs a Bazel(isk) build command for a single label using the provided
// config (which is assumed to be in //bazel/buildrc) and any provided Bazel args.
// This handles any setup needed to run Bazel on our CI machines before running the task, like
// setting up logs and the Bazel cache.
package main

import (
	"context"
	"flag"
	"fmt"
	"io/fs"
	"os"
	"path/filepath"
	"runtime"
	"strings"

	infra_common "go.skia.org/infra/go/common"
	sk_exec "go.skia.org/infra/go/exec"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/task_driver/go/lib/bazel"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
	"go.skia.org/skia/infra/bots/task_drivers/common"
)

var (
	// Required properties for this task.
	projectId      = flag.String("project_id", "", "ID of the Google Cloud project.")
	taskId         = flag.String("task_id", "", "ID of this task.")
	taskName       = flag.String("task_name", "", "Name of the task.")
	workdir        = flag.String("workdir", ".", "Working directory in which the build will be performed.")
	outPath        = flag.String("out_path", "", "Directory into which to copy the //bazel-bin subdirectories provided via --saved_output_dir. If unset, nothing will be copied.")
	savedOutputDir = infra_common.NewMultiStringFlag("saved_output_dir", nil, `//bazel-bin subdirectories to copy into the path provided via --out_path (e.g. "tests" will copy the contents of //bazel-bin/tests).`)

	// Optional flags.
	local  = flag.Bool("local", false, "True if running locally (as opposed to on the CI/CQ)")
	output = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
)

func main() {
	bazelFlags := common.MakeBazelFlags(common.MakeBazelFlagsOpts{
		Label:          true,
		Config:         true,
		AdditionalArgs: true,
	})

	// StartRun calls flag.Parse()
	ctx := td.StartRun(projectId, taskId, taskName, output, local)
	defer td.EndRun(ctx)

	bazelFlags.Validate(ctx)

	if *outPath != "" && len(*savedOutputDir) == 0 {
		td.Fatal(ctx, fmt.Errorf("at least one --saved_output_dir is required if --out_path is set"))
	}

	checkoutPath, err := os_steps.Abs(ctx, *workdir)
	if err != nil {
		td.Fatal(ctx, err)
	}

	// Dynamically locate and add the MSVC compiler and runtime folders to the system PATH.
	var newPATH, bazelVC string
	ctx, newPATH, bazelVC = locateAndAddMSVCToPATH(ctx, checkoutPath)

	// For Mac builds, ensure Xcode is downloaded/installed via mac_toolchain before we build.
	if runtime.GOOS == "darwin" {
		parentDir := filepath.Dir(checkoutPath)
		macToolchainBin := filepath.Join(parentDir, "mac_toolchain", "mac_toolchain")
		xcodeAppPath := filepath.Join(parentDir, "cache", "Xcode.app")

		if _, err := os.Stat(macToolchainBin); err == nil {
			err = td.Do(ctx, td.Props("Ensure Xcode via mac_toolchain"), func(ctx context.Context) error {
				runCmd := &sk_exec.Command{
					Name: macToolchainBin,
					Args: []string{
						"install",
						"-kind", "ios",
						"-xcode-version", "16a242d", // Xcode 16.0
						"-output-dir", xcodeAppPath,
					},
					InheritEnv: true,
					LogStdout:  true,
					LogStderr:  true,
				}
				_, runErr := sk_exec.RunCommand(ctx, runCmd)
				return runErr
			})
			if err != nil {
				td.Fatal(ctx, err)
			}
		}

		cachedXcodeDevDir := filepath.Join(xcodeAppPath, "Contents", "Developer")
		if _, err := os.Stat(cachedXcodeDevDir); err == nil {
			// We set DEVELOPER_DIR to the absolute path of the cached Xcode to bypass
			// the global system-wide 'xcode-select' pointer. This is similar to
			// 'sudo xcode-select -switch' but doesn't affect the global state.
			os.Setenv("DEVELOPER_DIR", cachedXcodeDevDir)
		}
	}

	var outputPath string
	if *outPath != "" {
		outputPath, err = os_steps.Abs(ctx, *outPath)
		if err != nil {
			td.Fatal(ctx, err)
		}
	}

	opts := bazel.BazelOptions{
		// We want the cache to be on a bigger disk than default. The root disk, where the home
		// directory (and default Bazel cache) lives, is only 15 GB on our GCE VMs.
		CachePath: *bazelFlags.CacheDir,
	}
	bzl, err := bazel.New(ctx, checkoutPath, "", opts)
	if err != nil {
		td.Fatal(ctx, err)
	}

	// Schedule the cleanup steps.
	defer func() {
		if !*local {
			// Ignore any error here until after we've run "shutdown".
			cleanErr := common.BazelCleanIfLowDiskSpace(ctx, *bazelFlags.CacheDir, checkoutPath, "bazelisk")
			if _, err := bzl.Do(ctx, "shutdown"); err != nil {
				td.Fatal(ctx, err)
			}
			if cleanErr != nil {
				td.Fatal(ctx, cleanErr)
			}
		}
	}()

	// Perform the build.
	args := append([]string{*bazelFlags.Label, fmt.Sprintf("--config=%s", *bazelFlags.Config)}, *bazelFlags.AdditionalArgs...)
	if os.Getenv("DEVELOPER_DIR") != "" {
		args = append(args, "--repo_env=DEVELOPER_DIR")
	}
	if runtime.GOOS == "windows" {
		// On Windows, Bazel's strict action environment (enabled by --incompatible_strict_action_env=true in .bazelrc)
		// strips the PATH variable from actions. This causes MSVC-compiled build utilities (like launcher_maker.exe)
		// and rules_rust compilation actions (which run with use_default_shell_env = False) to fail with
		// STATUS_DLL_NOT_FOUND (0xC0000135).
		//
		// Crucially, under --incompatible_strict_action_env, name-only action_env (i.e. --action_env=PATH) only works
		// if the variable is present in the client's environment with the EXACT same casing. On Windows, the client
		// env has "Path" (mixed case), but rules_rust's Starlark rule reads "PATH" (all caps) from default_shell_env.
		// By explicitly passing --action_env and --host_action_env with a static, pre-resolved VALUE (VAR=VALUE),
		// we bypass case-sensitivity mismatches and guarantee that both target and host/exec actions natively
		// receive the required MSVC runtime path containing sys64.
		if newPATH != "" {
			args = append(args, fmt.Sprintf("--action_env=PATH=%s", newPATH))
			args = append(args, fmt.Sprintf("--host_action_env=PATH=%s", newPATH))
		}
		if bazelVC != "" {
			args = append(args, fmt.Sprintf("--action_env=BAZEL_VC=%s", bazelVC))
			args = append(args, fmt.Sprintf("--host_action_env=BAZEL_VC=%s", bazelVC))
		}
	}

	if _, err := bzl.Do(ctx, "build", args...); err != nil {
		td.Fatal(ctx, err)
	}

	if outputPath != "" {
		if err := copyBazelBinSubdirs(ctx, checkoutPath, *savedOutputDir, outputPath); err != nil {
			td.Fatal(ctx, err)
		}
	}
}

// copyBazelBinSubdirs copies the contents of the bazel-bin directory into the given path.
func copyBazelBinSubdirs(ctx context.Context, checkoutDir string, bazelBinSubdirs []string, destinationDir string) error {
	for _, subdir := range bazelBinSubdirs {
		if err := td.Do(ctx, td.Props(fmt.Sprintf("Copying bazel-bin subdirectory %q into %q", subdir, destinationDir)), func(ctx context.Context) error {
			srcDir := filepath.Join(checkoutDir, "bazel-bin", subdir)
			dstDir := filepath.Join(destinationDir, subdir)

			return filepath.WalkDir(srcDir, func(path string, d fs.DirEntry, err error) error {
				if err != nil {
					// A non-nil err argument tells us the reason why filepath.WalkDir will not walk into
					// that directory (see https://pkg.go.dev/io/fs#WalkDirFunc). We choose to fail loudly
					// as this might reveal permission issues, problems with symlinks, etc.
					return skerr.Wrap(err)
				}

				relPath, err := filepath.Rel(srcDir, path)
				if err != nil {
					return skerr.Wrap(err)
				}
				dstPath := filepath.Join(dstDir, relPath)

				if d.IsDir() {
					return skerr.Wrap(os_steps.MkdirAll(ctx, dstPath))
				}
				return skerr.Wrap(os_steps.CopyFile(ctx, path, dstPath))
			})
		}); err != nil {
			return skerr.Wrap(err)
		}
	}
	return nil
}

// locateAndAddMSVCToPATH dynamically scans the pre-downloaded win_toolchain asset on the bot,
// detects the active MSVC compiler version, resolves its absolute directory, and appends it
// directly to the system PATH of the running task driver process (so all spawned child processes inherit it).
func locateAndAddMSVCToPATH(ctx context.Context, checkoutPath string) (context.Context, string, string) {
	if runtime.GOOS != "windows" {
		return ctx, "", ""
	}
	var newPATH, bazelVC string
	_ = td.Do(ctx, td.Props("Dynamic MSVC Runtime Path Resolver"), func(ctx context.Context) error {
		// 1. Locate the pre-downloaded win_toolchain asset directory
		winToolchainDir := "win_toolchain"
		if _, err := os.Stat(winToolchainDir); err != nil {
			// If not found in CWD, try relative to the parent of checkoutPath
			winToolchainDir = filepath.Join(filepath.Dir(checkoutPath), "win_toolchain")
		}

		if _, err := os.Stat(winToolchainDir); err != nil {
			fmt.Printf("Warning: win_toolchain directory does not exist: %s\n", err)
			return nil
		}

		// Walk the win_toolchain directory to locate vcruntime140.dll or vcruntime140_1.dll (specifically x64)
		var pathsToAdd []string
		added := make(map[string]bool)
		err := filepath.WalkDir(winToolchainDir, func(path string, d fs.DirEntry, err error) error {
			if err != nil {
				return nil
			}
			if !d.IsDir() {
				name := strings.ToLower(filepath.Base(path))
				if name == "vcruntime140.dll" || name == "vcruntime140_1.dll" {
					dir := filepath.Dir(path)
					lowerDir := strings.ToLower(filepath.ToSlash(dir))
					// We want x64 binaries. Exclude arm, x86, win32, and sys32 to avoid architecture mismatches.
					if !strings.Contains(lowerDir, "arm") && !strings.Contains(lowerDir, "x86") && !strings.Contains(lowerDir, "win32") && !strings.Contains(lowerDir, "sys32") {
						absPath, err := filepath.Abs(dir)
						if err == nil && !added[absPath] {
							added[absPath] = true
							pathsToAdd = append(pathsToAdd, absPath)
						}
					}
				}
			}
			return nil
		})
		if err != nil {
			fmt.Printf("Failed to walk win_toolchain directory: %s\n", err)
			return nil
		}

		// Also explicitly add the Hostx64 compiler tools folder if it exists
		msvcRoot := filepath.Join(winToolchainDir, "VC", "Tools", "MSVC")
		if entries, err := os.ReadDir(msvcRoot); err == nil {
			var msvcVersion string
			for _, entry := range entries {
				if entry.IsDir() {
					msvcVersion = entry.Name()
				}
			}
			if msvcVersion != "" {
				if compilerBinDir, err := filepath.Abs(filepath.Join(msvcRoot, msvcVersion, "bin", "Hostx64", "x64")); err == nil {
					if !added[compilerBinDir] {
						added[compilerBinDir] = true
						pathsToAdd = append(pathsToAdd, compilerBinDir)
					}
				}
			}
		}

		// Resolve and set BAZEL_VC for standard Bazel C++ toolchain discovery
		if vcDir, err := filepath.Abs(filepath.Join(winToolchainDir, "VC")); err == nil {
			os.Setenv("BAZEL_VC", vcDir)
			bazelVC = vcDir
			td.StepText(ctx, "Dynamic BAZEL_VC Set", vcDir)
		}

		if len(pathsToAdd) == 0 {
			fmt.Printf("Warning: Could not locate any x64 MSVC runtime DLLs or compiler tools in %s\n", winToolchainDir)
			return nil
		}

		// Prepend these folders directly to the system PATH of this running process
		currentPATH := os.Getenv("PATH")
		tempPATH := currentPATH
		for _, path := range pathsToAdd {
			tempPATH = fmt.Sprintf("%s;%s", path, tempPATH)
			td.StepText(ctx, "Dynamic PATH Added", path)
		}
		os.Setenv("PATH", tempPATH)
		newPATH = tempPATH
		return nil
	})

	if newPATH != "" {
		if bazelVC != "" {
			return td.WithEnv(ctx, []string{
				"PATH=" + newPATH,
				"BAZEL_VC=" + bazelVC,
			}), newPATH, bazelVC
		}
		return td.WithEnv(ctx, []string{"PATH=" + newPATH}), newPATH, ""
	}
	return ctx, "", ""
}
