#!/usr/bin/env python3
#
# Copyright 2025 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
"""Builds Dawn using CMake."""

import hashlib
import glob
import os
import shutil
import subprocess
import sys


def discover_dependencies(build_dir, targets):
  """Runs ninja -tinputs recursively to discover all targets, then uses
  ninja -tdeps to get all source files for those targets."""
  # The worklist contains ninja targets we need to find the inputs for.
  worklist = set(targets)
  # We keep track of all ninja targets we've ever seen to avoid cycles and
  # redundant processing.
  seen_targets = set(targets)

  while len(worklist) > 0:
    current_batch = list(worklist)
    worklist.clear()

    cmd = ["ninja", "-C", build_dir, "-tinputs"] + current_batch
    inputs = subprocess.check_output(cmd).decode("utf-8").splitlines()
    for line in inputs:
      line = line.strip()
      if line.startswith("/"):
        continue  # file, which are always absolute paths, skip it
      if line not in seen_targets:
        worklist.add(line)
        seen_targets.add(line)

  # Now that we have all the targets, get the dependencies for them.
  source_files = set()
  cmd = ["ninja", "-C", build_dir, "-tdeps"] + list(seen_targets)
  output = subprocess.check_output(cmd).decode("utf-8").splitlines()

  for line in output:
    if not line.startswith("  ") or "deps not found" in line:
      continue
    line = line.strip()
    source_files.add(line)

  result = list(source_files)
  result.sort()
  return result


def write_depfile(deps_map, depfile_path):
  """Generates a .d file that lists all discovered source files as
  dependencies for each of the given output libraries."""
  os.makedirs(os.path.dirname(depfile_path), exist_ok=True)
  with open(depfile_path, "w") as f:
    for output_path, dependencies in deps_map.items():
      # Make sure to use forward slashes for paths in the depfile, even on
      # Windows, for consistency.
      deps_str = ""
      for dep in dependencies:
        deps_str += " \\\n" + dep.replace("\\", "/")
      f.write(f"{output_path}:{deps_str}\n")


def copy_if_changed(src, dest):
  """Copies the file from src to dest if dest doesn't exist or if the
  contents of src and dest are different."""
  if os.path.exists(dest):
    src_hash = hashlib.sha256(open(src, "rb").read()).hexdigest()
    dest_hash = hashlib.sha256(open(dest, "rb").read()).hexdigest()
    if src_hash == dest_hash:
      return
  shutil.copyfile(src, dest)
  os.chmod(dest, 0o755)


def main():
  if len(sys.argv) < 4:
    print(
        "Usage: build_dawn.py <output_dir> <gen_dir> [libs...]", file=sys.stderr)
    sys.exit(1)

  output_dir = sys.argv[1]
  gen_dir = sys.argv[2]
  libs_to_build = sys.argv[3:]
  build_target = ["webgpu_headers_gen"] + libs_to_build
  depfile_path = os.path.join(gen_dir, "dawn_libs.d")

  dawn_dir = os.path.join("..", "..", "third_party", "externals", "dawn")
  build_dir = os.path.join("cmake", os.path.dirname(output_dir), "dawn_build")

  subprocess.run(["cmake", "--version"], check=True)
  subprocess.run(["ninja", "--version"], check=True)

  configure_cmd = [
      "cmake",
      "-S",
      dawn_dir,
      "-B",
      build_dir,
      "-G",
      "Ninja",
      f"--install-prefix={os.path.abspath(gen_dir)}",
      "-DDAWN_FETCH_DEPENDENCIES=ON",
      "-DDAWN_BUILD_MONOLITHIC_LIBRARY=STATIC",
      "-DCMAKE_BUILD_TYPE=Release",
      "-DDAWN_ENABLE_INSTALL=ON",
      "-DTINT_ENABLE_INSTALL=ON",
      "-DDAWN_BUILD_SAMPLES=OFF",
      "-DDAWN_BUILD_TESTS=OFF",
      "-DTINT_BUILD_TESTS=OFF",
      "-DTINT_BUILD_HLSL_WRITER=OFF",
      "-DDAWN_USE_X11=OFF",
  ]
  subprocess.run(configure_cmd, check=True)

  build_cmd = ["ninja", "-C", build_dir, "-dkeepdepfile"] + build_target
  subprocess.run(build_cmd, check=True)

  # The install target is broken, so we manually copy the headers.
  # The headers are located in the build directory in a gen/include folder.
  # The Skia build expects to find headers in subdirectories of the include path.
  generated_headers_src = os.path.join(build_dir, "gen", "include")
  generated_headers_dest = os.path.join(gen_dir, "include")

  if os.path.exists(generated_headers_dest):
    shutil.rmtree(generated_headers_dest)

  # Copy the contents of the 'dawn' and 'webgpu' directories into the destination.
  shutil.copytree(
      os.path.join(generated_headers_src, "dawn"),
      os.path.join(generated_headers_dest, "dawn"),
      dirs_exist_ok=True)
  shutil.copytree(
      os.path.join(generated_headers_src, "webgpu"),
      os.path.join(generated_headers_dest, "webgpu"),
      dirs_exist_ok=True)

  # The headers are a dependency for all libraries.
  header_deps = discover_dependencies(build_dir, ["webgpu_headers_gen"])

  deps_map = {}
  for lib_name in libs_to_build:
    # Discover the specific dependencies for this library.
    lib_deps = discover_dependencies(build_dir, [lib_name])

    # Combine header deps and lib deps, remove duplicates, and sort.
    all_deps = sorted(list(set(header_deps + lib_deps)))

    # Add the depfile entry for this library.
    output_path = os.path.join(output_dir, lib_name)
    deps_map[output_path] = all_deps

  write_depfile(deps_map, depfile_path)

  for lib_name in libs_to_build:
    # Search for the library in the build directory.
    search_pattern = os.path.join(build_dir, "**", lib_name)
    found_files = glob.glob(search_pattern, recursive=True)

    if not found_files:
      sys.exit(f"Error: Library not found: {lib_name}")

    if len(found_files) > 1:
      sys.exit(f"Error: Found multiple locations for {lib_name}: {found_files}")

    src_path = found_files[0]
    dest_path = os.path.join(output_dir, lib_name)
    copy_if_changed(src_path, dest_path)


if __name__ == "__main__":
  main()
