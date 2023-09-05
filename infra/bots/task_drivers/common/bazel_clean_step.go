// Copyright 2023 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package common

import (
	"context"
	"fmt"

	sk_exec "go.skia.org/infra/go/exec"

	"github.com/shirou/gopsutil/disk"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/task_driver/go/td"
)

const (
	bazelCachePartitionMountpoint = "/mnt/pd0"

	// The DiskSpaceLow alert triggers at 10GB, so we set this threshold to a slightly higher value.
	bazelCachePartitionMinRequiredFreeSpaceBytes = uint64(15_000_000_000)
)

type bazelCleanIfLowDiskSpaceContextKeyType = string

// This context key can be used from tests to override the function that computes the free space
// (measured in bytes) on the partition where the Bazel cache lives.
const BazelCleanIfLowDiskSpaceContextKey = bazelCleanIfLowDiskSpaceContextKeyType("overwriteBazelCleanIfLowDiskSpace")

// BazelCleanIfLowDiskSpace runs "bazel clean" as a task driver step if disk space is too low. This
// step should be added at the end of any task driver that shells out to Bazel in order to prevent
// DiskSpaceLow alerts due to the Bazel cache in /mnt/pd0/bazel_cache growing too large.
//
// Ideally, we would like to tell Bazel to prevent the cache from growing above a certain size, but
// there is currently no way to do this. For example, see:
//
// - https://github.com/bazelbuild/bazel/issues/1035
// - https://github.com/bazelbuild/bazel/issues/5139
//
// Testing: Set the BazelCleanIfLowDiskSpaceContextKey context key to override the function that
// computes the free space (measured in bytes) on the partition where the Bazel cache lives.
func BazelCleanIfLowDiskSpace(ctx context.Context, bazelWorkspaceDir, pathToBazel string) error {
	return skerr.Wrap(td.Do(ctx, td.Props("Clean Bazel cache if disk space is too low"), func(ctx context.Context) error {
		var (
			freeSpace uint64
			err       error
		)
		if fn := ctx.Value(BazelCleanIfLowDiskSpaceContextKey); fn != nil {
			freeSpace, err = fn.(func() (uint64, error))()
		} else {
			freeSpace, err = freeBytesOnPartition(bazelCachePartitionMountpoint)
		}
		if err != nil {
			return skerr.Wrap(err)
		}

		if freeSpace < bazelCachePartitionMinRequiredFreeSpaceBytes {
			msg := fmt.Sprintf("Free space on partition %s is %d bytes, which is below the threshold of %d bytes", bazelCachePartitionMountpoint, freeSpace, bazelCachePartitionMinRequiredFreeSpaceBytes)
			if err := td.Do(ctx, td.Props(msg), func(ctx context.Context) error { return nil }); err != nil {
				return skerr.Wrap(err)
			}

			cmd := &sk_exec.Command{
				Name:       pathToBazel,
				Dir:        bazelWorkspaceDir,
				Args:       []string{"clean"},
				InheritEnv: true, // Make sure "bazelisk" is on PATH.
				LogStdout:  true,
				LogStderr:  true,
			}
			_, err := sk_exec.RunCommand(ctx, cmd)
			return skerr.Wrap(err)
		}

		msg := fmt.Sprintf("No need to clear the Bazel cache: free space on partition %s is %d bytes, which is above the threshold of %d bytes", bazelCachePartitionMountpoint, freeSpace, bazelCachePartitionMinRequiredFreeSpaceBytes)
		return skerr.Wrap(td.Do(ctx, td.Props(msg), func(ctx context.Context) error { return nil }))
	}))
}

func freeBytesOnPartition(mountpoint string) (uint64, error) {
	usage, err := disk.Usage(mountpoint)
	if err != nil {
		return 0, skerr.Wrap(err)
	}
	return usage.Free, nil
}
