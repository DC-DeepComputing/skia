#!/usr/bin/env python3
#
# Copyright 2025 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#

import os
import sys
import shutil
import subprocess
import glob
import tempfile

# This script is executed from the root build directory, e.g. out/DawnVulkan
# We can find the dawn source tree in ../../third_party/externals/dawn
DAWN_SOURCE_DIR = os.path.abspath(os.path.join(os.getcwd(), '../../third_party/externals/dawn'))
CMAKE_BUILD_DIR = os.path.join(os.getcwd(), 'cmake/dawn')

# The single output of this script is the monolithic library.
# The path is relative to the build directory, e.g. out/DawnVulkan
OUTPUT_LIB = sys.argv[1]

# Add a comment explaining the purpose of the script.
# This script builds the Tint compiler from source using CMake and then combines
# all the resulting static libraries into a single archive for easier linking.
print("Building Tint from source using CMake...")

subprocess.run(["cmake", "--version"], check=True)
subprocess.run(["ninja", "--version"], check=True)

configure_cmd = [
"cmake",
"-S", DAWN_SOURCE_DIR,
"-B", CMAKE_BUILD_DIR,
"-DDAWN_FETCH_DEPENDENCIES=ON",
"-DDAWN_ENABLE_INSTALL=ON",
"-DCMAKE_BUILD_TYPE=Release",
"-DDAWN_USE_X11=OFF",
"-DTINT_BUILD_WGSL_READER=ON",
"-DTINT_BUILD_WGSL_WRITER=ON",
"-DTINT_ENABLE_INSTALL=ON",
"-G", "Ninja",
]

subprocess.run(configure_cmd, check=True)

# Build the tint target. This will build the tint executable and all of its
# dependent static libraries.
build_cmd = [
"cmake",
"--build", CMAKE_BUILD_DIR,
"--target", "tint",
]

subprocess.run(build_cmd, check=True)

# Find all the static libraries that were built.
static_libs = glob.glob(os.path.join(CMAKE_BUILD_DIR, '**', '*.a'), recursive=True)
if not static_libs:
  sys.exit("Error: No static libraries found in " + CMAKE_BUILD_DIR)

# Create a temporary directory to extract all the object files into.
with tempfile.TemporaryDirectory() as tmpdir:
  print(f"Combining {len(static_libs)} static libraries into a single archive...")
  # Extract all the object files from each static library.
  for lib in static_libs:
    subprocess.run(['ar', 'x', lib], cwd=tmpdir, check=True)

  # Combine all the extracted object files into a new archive.
  object_files = glob.glob(os.path.join(tmpdir, '*.o'))
  if not object_files:
    sys.exit("Error: No object files found in the extracted libraries.")

  # Create the final combined archive.
  subprocess.run(['ar', 'rcs', OUTPUT_LIB] + object_files, check=True)

os.chmod(OUTPUT_LIB, 0o755)
print("Successfully created " + OUTPUT_LIB)
