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
import shutil
import subprocess
import sys


def write_depfile(output_path, depfile_path, dawn_dir):
  """Generates a .d file that lists all Tint and Abseil source files as
  dependencies for the given output library."""
  tint_src_dir = os.path.join(dawn_dir, "src", "tint")
  absl_src_dir = os.path.join(dawn_dir, "third_party", "abseil-cpp")

  tint_files = glob.glob(os.path.join(tint_src_dir, "**", "*"), recursive=True)
  absl_files = glob.glob(os.path.join(absl_src_dir, "**", "*"), recursive=True)

  # The dependencies are the collection of all files in those directories.
  # We must filter out directories and only include files.
  all_deps = [f for f in tint_files + absl_files if os.path.isfile(f)]

  os.makedirs(os.path.dirname(depfile_path), exist_ok=True)
  with open(depfile_path, "w") as f:
    # The output path needs to be relative to the root build directory, not the
    # full path. GN handles this correctly.
    f.write(f"{output_path}: \n")
    # Make sure to use forward slashes for paths in the depfile, even on
    # Windows, for consistency.
    f.write(" \n".join(p.replace("\\", "/") for p in all_deps))


def main():
  if len(sys.argv) != 3:
    print("Usage: build_tint.py <output_path> <depfile_path>", file=sys.stderr)
    sys.exit(1)

  output_path = sys.argv[1]
  depfile_path = sys.argv[2]
  # Assume Dawn is checked out in third_party/externals.
  dawn_dir = os.path.join("..", "..", "third_party", "externals", "dawn")

  # Generate the depfile. This lists all source files that the Tint library
  # depends on. This allows GN to know when to re-run this script.
  write_depfile(output_path, depfile_path, dawn_dir)

  # We will build everything in a temporary directory.
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

  # Build the configured project.
  build_cmd = ["cmake", "--build", build_dir]
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
  # We check hashes to avoid unnecessary copies, which can cause GN to
  # rebuild downstream dependencies.
  copy_if_changed(combined_archive, os.path.join(os.getcwd(), output_path))


def copy_if_changed(src, dest):
  """Copies the file from src to dest if dest doesn't exist or if the
  contents of src and dest are different."""
  if os.path.exists(dest):
    src_hash = hashlib.sha256(open(src, 'rb').read()).hexdigest()
    dest_hash = hashlib.sha256(open(dest, 'rb').read()).hexdigest()
    if src_hash == dest_hash:
      # The files are identical, no need to copy.
      return

  # Either the destination does not exist or it is different.
  shutil.copyfile(src, dest)
  os.chmod(dest, 0o755)


if __name__ == "__main__":
  main()
