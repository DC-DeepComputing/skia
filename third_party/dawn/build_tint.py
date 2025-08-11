#!/usr/bin/env python3
#
# Copyright 2025 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
"""Builds Tint using CMake."""

import hashlib
import glob
import os
import re
import shutil
import subprocess
import sys


def discover_dependencies(build_dir, targets):
  """Runs ninja -t inputs to discover all source files for a given set of
  targets, optimized to call ninja fewer times."""
  source_files = set()
  # The worklist contains ninja targets we need to find the inputs for.
  worklist = set(targets)
  # We keep track of all ninja targets we've ever seen to avoid cycles and
  # redundant processing.
  seen_targets = set(targets)

  while worklist:
    # Process the entire worklist in one go.
    current_batch = list(worklist)
    worklist.clear()

    cmd = ["ninja", "-C", build_dir, "-t", "inputs"] + current_batch

    try:
      # The output is a flat list of all inputs for all targets in the batch.
      # It may also contain the names of the targets themselves, which we need to filter out.
      inputs = subprocess.check_output(cmd).decode("utf-8").splitlines()
    except subprocess.CalledProcessError:
      # This might happen if a target has no inputs. It's safe to ignore.
      continue

    for i in inputs:
      # The path 'i' is relative to GN build directory
      path_to_check = os.path.join(build_dir, i)

      if os.path.exists(path_to_check):
        # This is a source file. Add its normalized path to our set.
        source_files.add(os.path.normpath(path_to_check))
      else:
        # This is a ninja target. If we haven't seen it before, add it to
        # the worklist for the next iteration.
        if i not in seen_targets:
          worklist.add(i)
          seen_targets.add(i)

  return list(source_files)


def write_depfile(output_path, depfile_path, dependencies):
  """Generates a .d file that lists all discovered source files as
  dependencies for the given output library."""
  os.makedirs(os.path.dirname(depfile_path), exist_ok=True)
  with open(depfile_path, "w") as f:
    # The output path needs to be relative to the root build directory, not the
    # full path. GN handles this correctly.
    f.write(f"{output_path}:")
    # Make sure to use forward slashes for paths in the depfile, even on
    # Windows, for consistency.
    for dep in sorted(dependencies):
      f.write(" \\\n" + dep.replace("\\", "/"))
    f.write("\n")


def copy_if_changed(src, dest):
  """Copies the file from src to dest if dest doesn't exist or if the
  contents of src and dest are different."""
  if os.path.exists(dest):
    src_hash = hashlib.sha256(open(src, "rb").read()).hexdigest()
    dest_hash = hashlib.sha256(open(dest, "rb").read()).hexdigest()
    if src_hash == dest_hash:
      # The files are identical, no need to copy.
      return

  # Either the destination does not exist or it is different.
  shutil.copyfile(src, dest)
  os.chmod(dest, 0o755)


def main():
  if len(sys.argv) != 3:
    print("Usage: build_tint.py <output_path> <depfile_path>", file=sys.stderr)
    sys.exit(1)

  output_path = sys.argv[1]
  depfile_path = sys.argv[2]
  # Assume Dawn is checked out in $SKIA_ROOT/third_party/externals and we
  # are running from out/SomeBuildDirectory
  dawn_dir = os.path.join("..", "..", "third_party", "externals", "dawn")

  # We will build everything in a temporary directory (under out/SomeBuildDirectory)
  build_dir = os.path.join("cmake", "dawn")

  # Make sure cmake and ninja are on PATH
  subprocess.run(["cmake", "--version"], check=True)
  subprocess.run(["ninja", "--version"], check=True)

  # Configure the project using CMake. Tint is a part of Dawn.
  # https://github.com/google/dawn/blob/main/docs/quickstart-cmake.md
  configure_cmd = [
      "cmake",
      "-S",
      dawn_dir,
      "-B",
      build_dir,
      # Fetch dependencies using DEPS, which is required for stand-alone builds.
      "-DDAWN_FETCH_DEPENDENCIES=ON",
      "-DDAWN_ENABLE_INSTALL=ON",
      "-DCMAKE_BUILD_TYPE=Release",
      "-DDAWN_USE_X11=OFF",
      # skslc needs WGSL support.
      "-DTINT_BUILD_WGSL_READER=ON",
      "-DTINT_BUILD_WGSL_WRITER=ON",
      "-DTINT_ENABLE_INSTALL=ON",
      # Use Ninja for faster builds.
      "-G",
      "Ninja",
  ]
  subprocess.run(configure_cmd, check=True)

  # These tint targets (and their deps) are what Skia needs to build
  tint_targets = ["tint_api", "tint_lang_wgsl_reader", "tint_lang_wgsl_writer"]

  # Generate the depfile. This lists all source files that the Tint library
  # depends on. This allows GN to know when to re-run this script.
  dependencies = discover_dependencies(build_dir, tint_targets)
  write_depfile(output_path, depfile_path, dependencies)

  build_cmd = ["ninja", "-C", build_dir] + tint_targets
  subprocess.run(build_cmd, check=True)

  # After building, Tint consists of 90+ small static libraries. For easier
  # consumption by GN, we combine them into a single archive using ar.
  tint_libs = glob.glob(os.path.join(build_dir, "src", "tint", "lib*.a"))
  if not tint_libs:
    sys.exit("Error: No static libraries found for Tint.")

  # Tint also depends on Abseil. Combining Tint's deps into the library makes
  # consumption by GN easier.
  absl_libs = [
      os.path.join(build_dir, "third_party", "abseil", "absl", "strings",
                   "libabsl_strings.a")
  ]

  # https://stackoverflow.com/a/23621751
  # We first create a "thin" archive. A thin archive does not contain the
  # object files themselves, but rather references to them.
  #   c: Create the archive.
  #   q: Quickly append files (no checking for replacement).
  #   T: Create a thin archive.
  combined_thin_archive = os.path.join(build_dir, "out", "combined_tint_thin.a")
  os.makedirs(os.path.dirname(combined_thin_archive), exist_ok=True)
  combine_cmd = ["ar", "cqT", combined_thin_archive] + tint_libs + absl_libs
  subprocess.run(combine_cmd, check=True)

  # We'll be relocating the output, so a thin archive doesn't work. To create
  # a regular, "fat" archive, we use an MRI script with 'ar'.
  combined_archive = os.path.join(build_dir, "out", "libtint_combined.a")
  mri_script = f"""
create {combined_archive}
addlib {combined_thin_archive}
save
end
"""
  # The 'ar -M' command reads commands from stdin to modify the archive.
  subprocess.run(["ar", "-M"], input=mri_script.encode("utf-8"), check=True)

  # Copy the final archive to the location expected by GN.
  # We check hashes to avoid unnecessary updates to the "last modified" datum
  # of the file, which can cause GN to unnecessarily rebuild downstream dependencies.
  copy_if_changed(combined_archive, os.path.join(os.getcwd(), output_path))


if __name__ == "__main__":
  main()
