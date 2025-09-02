#!/usr/bin/env python3
#
# Copyright 2025 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
"""Builds Dawn using CMake.

This script configures and builds Dawn using CMake and Ninja. It then
discovers all object files for the specified libraries and combines them
into static archives (.a files) using `ar`. It also copies Dawn's generated
headers to the location GN expects.
"""

import argparse
import hashlib
import glob
import os
import shutil
import subprocess
import pprint
import sys


def discover_dependencies(build_dir, target):
  """Runs ninja -tinputs recursively to discover all targets, then uses
  ninja -tdeps to get all source files and object files for those targets.
  Returns a tuple of (list of source files, list of object files)."""
  # The worklist contains ninja targets we need to find the inputs for.
  worklist = {target}
  # We keep track of all ninja targets we've ever seen to avoid cycles and
  # redundant processing.
  seen_targets = {target}

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

  object_files = []
  for target in seen_targets:
    if target.endswith(".o"):
      object_files.append(target)
  object_files.sort()

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
  return result, object_files


def write_depfile(deps_map, depfile_path):
  """Generates a .d file that lists all discovered source files as
  dependencies for each of the given output libraries."""
  os.makedirs(os.path.dirname(depfile_path), exist_ok=True)
  with open(depfile_path, "w") as f:
    for output_path, dependencies in deps_map.items():
      # Make sure to use forward slashes for paths in the depfile, even on
      # Windows, for consistency.
      deps_lines = []
      for dep in dependencies:
        deps_lines.append(dep.replace("\\", "/"))
      deps_str = " \\n" + " \\n".join(deps_lines)
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
  print("Environment for Dawn build:")
  pprint.pprint(dict(os.environ))
  parser = argparse.ArgumentParser(description="Build Dawn using CMake.")
  parser.add_argument("--cc", required=True, help="Path to the C compiler.")
  parser.add_argument("--cxx", required=True, help="Path to the C++ compiler.")
  parser.add_argument(
      "--cxx_flags",
      default=[],
      action="append",
      help="C++ compiler flags. Can be specified multiple times.")
  parser.add_argument(
      "--ld_flags",
      default=[],
      action="append",
      help="Linker flags. Can be specified multiple times.")
  parser.add_argument(
      "--output_dir", required=True, help="Directory to place output libraries.")
  parser.add_argument(
      "--gen_dir",
      required=True,
      help="Directory for generated files (e.g., depfiles).")
  parser.add_argument(
      "libs", nargs="+", help="List of libraries to build.")
  args = parser.parse_args()

  output_dir = args.output_dir
  gen_dir = args.gen_dir
  libs_to_build = args.libs
  build_target = ["webgpu_headers_gen"] + libs_to_build
  depfile_path = os.path.join(gen_dir, "dawn_libs.d")

  # The script's CWD is the build root, so this will create absolute paths.
  abs_output_dir = os.path.abspath(output_dir)

  script_dir = os.path.dirname(os.path.realpath(__file__))
  print("script_dir is", script_dir)
  dawn_dir = os.path.join(script_dir, "..", "externals", "dawn")
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
      f"-DCMAKE_C_COMPILER={args.cc}",
      f"-DCMAKE_CXX_COMPILER={args.cxx}",
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
  if args.cxx_flags:
    cxx_flags = " ".join(args.cxx_flags)
    configure_cmd.append(f"-DCMAKE_CXX_FLAGS={cxx_flags}")
  if args.ld_flags:
    ld_flags = " ".join(args.ld_flags)
    configure_cmd.append(f"-DCMAKE_EXE_LINKER_FLAGS={ld_flags}")
    configure_cmd.append(f"-DCMAKE_SHARED_LINKER_FLAGS={ld_flags}")
    configure_cmd.append(f"-DCMAKE_MODULE_LINKER_FLAGS={ld_flags}")
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
  header_deps, _ = discover_dependencies(build_dir, "webgpu_headers_gen")

  deps_map = {}
  for lib_name in libs_to_build:
    # Discover the specific dependencies for this library.
    lib_deps, lib_obj = discover_dependencies(build_dir, lib_name)

    # Combine header deps and lib deps, remove duplicates, and sort.
    all_deps = sorted(list(set(header_deps + lib_deps)))

    # Add the depfile entry for this library.
    deps_map[lib_name] = all_deps

    # Delete generated .a file if it exists already (otherwise ar sometimes chokes)
    gen_library_path = os.path.join(build_dir, lib_name)
    if os.path.exists(gen_library_path):
      os.remove(gen_library_path)
    # Use ar to join all the object files that comprise the necessary libraries
    # and any transitive dependencies into one .a file.
    combine_obj_cmd = ["ar", "rcs", lib_name] + lib_obj
    subprocess.run(combine_obj_cmd, cwd=build_dir, check=True)
    dest_path = os.path.join(abs_output_dir, lib_name)
    copy_if_changed(gen_library_path, dest_path)

  write_depfile(deps_map, depfile_path)

if __name__ == "__main__":
  main()
