#!/usr/bin/env python3
#
# Copyright 2025 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
"""Builds Tint using CMake.

This script configures and builds Tint using CMake and Ninja. It then
discovers all object files for the specified libraries and combines them
into static archives (.a files) using `ar`.
"""

import argparse
import os
import shutil
import subprocess
import sys

from cmake_utils import (copy_if_changed, discover_dependencies,
                         quote_if_needed, write_depfile)

def main():
  parser = argparse.ArgumentParser(description="Build Tint using CMake.")
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
      "--output_path", required=True, help="Path to the output library.")
  parser.add_argument(
      "--depfile_path", required=True, help="Path to the depfile to generate.")
  parser.add_argument(
      "--target_os", default="", help="Target OS for host compilation.")
  parser.add_argument(
      "--target_cpu", default="", help="Target CPU for host compilation.")
  parser.add_argument(
      "--win_sdk", default="", help="Path to the Windows SDK.")
  parser.add_argument(
      "--win_sdk_version", default="", help="Version of the Windows SDK.")
  parser.add_argument(
      "--win_vc", default="", help="Path to Visual C++.")
  parser.add_argument(
      "--win_toolchain_version", default="", help="Version of the MSVC toolchain.")
  parser.add_argument(
      "--build_type", default="Release", help="CMake build type.")
  args = parser.parse_args()

  output_path = args.output_path
  depfile_path = args.depfile_path
  script_dir = os.path.dirname(os.path.realpath(__file__))
  dawn_dir = os.path.join(script_dir, "..", "externals", "dawn")

  # We will build everything in a temporary directory (under out/SomeBuildDirectory).
  # The build can be invoked in parallel for different toolchains, so we use
  # part of the output path to create a unique build directory.
  build_dir = os.path.join("cmake", os.path.dirname(output_path), "dawn_for_tint")

  cmake_exe = shutil.which("cmake")
  if not cmake_exe:
    print("Error: cmake not found in PATH.")
    sys.exit(1)

  ninja_exe = shutil.which("ninja")
  if not ninja_exe:
    print("Error: ninja not found in PATH.")
    sys.exit(1)

  # Configure the project using CMake. Tint is a part of Dawn.
  # https://github.com/google/dawn/blob/main/docs/quickstart-cmake.md
  configure_cmd = [
      cmake_exe,
      "-S",
      dawn_dir,
      "-B",
      build_dir,
      f"-DCMAKE_C_COMPILER={quote_if_needed(args.cc)}",
      f"-DCMAKE_CXX_COMPILER={quote_if_needed(args.cxx)}",
      # Fetch dependencies using DEPS, which is required for stand-alone builds.
      "-DDAWN_FETCH_DEPENDENCIES=ON",
      "-DDAWN_ENABLE_INSTALL=ON",
      f"-DCMAKE_BUILD_TYPE={args.build_type}",
      "-DDAWN_USE_X11=OFF",
      # Samples and tests are not needed and may have extra dependencies.
      "-DDAWN_BUILD_SAMPLES=OFF",
      "-DTINT_BUILD_TESTS=OFF",
      # skslc needs WGSL support.
      "-DTINT_BUILD_WGSL_READER=ON",
      "-DTINT_BUILD_WGSL_WRITER=ON",
      "-DTINT_ENABLE_INSTALL=ON",
      # Use Ninja for faster builds.
      "-G",
      "Ninja",
      f"-DCMAKE_MAKE_PROGRAM={ninja_exe}",
      # Disable all backends since we only need Tint. This avoids building
      # the emdawnwebgpu target, which is not needed and causes python errors.
      "-DDAWN_ENABLE_D3D11=OFF",
      "-DDAWN_ENABLE_D3D12=OFF",
      "-DDAWN_ENABLE_METAL=OFF",
      "-DDAWN_ENABLE_NULL=OFF",
      "-DDAWN_ENABLE_DESKTOP_GL=OFF",
      "-DDAWN_ENABLE_OPENGLES=OFF",
      "-DDAWN_ENABLE_VULKAN=OFF",
  ]
  cxx_flags_to_add = args.cxx_flags or []

  ld_flags = args.ld_flags or []
  if args.win_sdk:
    assert(args.win_vc)
    assert(args.win_sdk_version)
    # Set the Windows SDK version for CMake.
    configure_cmd.append(
        f"-DCMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION={args.win_sdk_version}")
    # Explicitly tell CMake where to find the Resource Compiler, Manifest Tool, and Archiver.
    rc_exe_path = os.path.join(args.win_sdk, "bin", args.win_sdk_version,
                               args.target_cpu, "rc.exe")
    configure_cmd.append(f"-DCMAKE_RC_COMPILER={quote_if_needed(rc_exe_path.replace(os.sep, '/'))}")
    mt_exe_path = os.path.join(args.win_sdk, "bin", args.win_sdk_version,
                               args.target_cpu, "mt.exe")
    configure_cmd.append(f"-DCMAKE_MT={quote_if_needed(mt_exe_path.replace(os.sep, '/'))}")
    ar_exe_path = os.path.join(args.win_vc, "Tools", "MSVC",
                               args.win_toolchain_version, "bin", "Hostx64",
                               args.target_cpu, "lib.exe")
    configure_cmd.append(f"-DCMAKE_AR={quote_if_needed(ar_exe_path.replace(os.sep, '/'))}")

    # On Windows, we need to explicitly tell clang where to find the toolchain
    # headers and libraries.
    um_lib_path = os.path.join(args.win_sdk, "Lib", args.win_sdk_version, "um", args.target_cpu)
    ucrt_lib_path = os.path.join(args.win_sdk, "Lib", args.win_sdk_version, "ucrt", args.target_cpu)
    msvc_lib_path = os.path.join(args.win_vc, "Tools", "MSVC", args.win_toolchain_version, "lib", args.target_cpu)

    ld_flags += [
      f"/LIBPATH:{quote_if_needed(um_lib_path.replace(os.sep, '/'))}",
      f"/LIBPATH:{quote_if_needed(ucrt_lib_path.replace(os.sep, '/'))}",
      f"/LIBPATH:{quote_if_needed(msvc_lib_path.replace(os.sep, '/'))}",
    ]

    if "clang" in os.path.basename(args.cc).lower():
      cxx_flags_to_add += [
          # Skia uses a hermetic toolchain, so we need to tell clang where to
          # find the MSVC headers and libraries.
          "-imsvc",
          os.path.join(args.win_vc, "Tools", "MSVC", args.win_toolchain_version, "include"),
          "-imsvc",
          os.path.join(args.win_sdk, "Include", args.win_sdk_version, "ucrt"),
          "-imsvc",
          os.path.join(args.win_sdk, "Include", args.win_sdk_version, "shared"),
          "-imsvc",
          os.path.join(args.win_sdk, "Include", args.win_sdk_version, "um"),
          "-imsvc",
          os.path.join(args.win_sdk, "Include", args.win_sdk_version, "winrt"),
      ]
    else:
      cxx_flags_to_add += [
          # For MSVC, we need to explicitly pass the include paths.
          "/I" + os.path.join(args.win_vc, "Tools", "MSVC", args.win_toolchain_version, "include"),
          "/I" + os.path.join(args.win_sdk, "Include", args.win_sdk_version, "ucrt"),
          "/I" + os.path.join(args.win_sdk, "Include", args.win_sdk_version, "shared"),
          "/I" + os.path.join(args.win_sdk, "Include", args.win_sdk_version, "um"),
          "/I" + os.path.join(args.win_sdk, "Include", args.win_sdk_version, "winrt"),
      ]

    # We want to build Dawn (and its dependencies) with /MT so we can statically
    # link it into Skia.
    configure_cmd.append("-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded")
    configure_cmd.append("-DABSL_MSVC_STATIC_RUNTIME=ON")

  if cxx_flags_to_add:
    c_cxx_flags_str = " ".join(cxx_flags_to_add)
    configure_cmd.append(f"-DCMAKE_CXX_FLAGS={c_cxx_flags_str}")

  if ld_flags:
    ld_flags_str = " ".join(ld_flags)
    configure_cmd.append(f"-DCMAKE_EXE_LINKER_FLAGS={ld_flags_str}")
    configure_cmd.append(f"-DCMAKE_SHARED_LINKER_FLAGS={ld_flags_str}")
    configure_cmd.append(f"-DCMAKE_MODULE_LINKER_FLAGS={ld_flags_str}")
    # if not args.win_sdk:
    #     configure_cmd.append(f"-DCMAKE_STATIC_LINKER_FLAGS={ld_flags_str}")
  # Set PYTHONPATH to include Dawn's third_party directory. This is needed
  # for the generator scripts to find jinja2 and markupsafe as packages.
  third_party_dir = os.path.abspath(os.path.join(dawn_dir, "third_party"))
  env = os.environ.copy()
  print(f"PYTHONPATH before modification: {env.get('PYTHONPATH')}")
  if "PYTHONPATH" in env:
    env["PYTHONPATH"] = third_party_dir + os.pathsep + env["PYTHONPATH"]
  else:
    env["PYTHONPATH"] = third_party_dir
  print(f"PYTHONPATH after modification: {env.get('PYTHONPATH')}")

  subprocess.run(configure_cmd, check=True, env=env)

  # These tint targets (and their deps) are what Skia needs to build
  tint_targets = ["tint_api", "tint_lang_wgsl_reader", "tint_lang_wgsl_writer"]

  build_cmd = [ninja_exe, "-C", build_dir, "-dkeepdepfile"] + tint_targets
  subprocess.run(build_cmd, check=True, env=env)

  dependencies, object_files = discover_dependencies(build_dir, tint_targets)
  # Generate the depfile. This lists all source files that the Tint library
  # depends on. This allows GN to know when to re-run this script.
  write_depfile(output_path, depfile_path, dependencies)

  # After building, Tint consists of many small object files. For easier
  # consumption by GN, we combine them into a single archive using ar.
  lib_name = os.path.basename(output_path)
  gen_library_path = os.path.join(build_dir, lib_name)
  # ar can fail if the archive already exists, e.g. "archive is malformed"
  if os.path.exists(gen_library_path):
    os.remove(gen_library_path)

  assert len(object_files) > 0
  # Use ar/lib to join all the object files that comprise the necessary
  # libraries and any transitive dependencies into one .a file.
  if args.win_sdk:
    # On Windows, we use lld-link.exe, which acts like lib.exe when passed
    # the /LIB flag. It should be in the same directory as the C compiler.
    # The command line can be too long, so we use a response file.
    linker_exe = os.path.join(os.path.dirname(args.cc), "lld-link.exe")
    response_file_name = "objects.rsp"
    response_file_path = os.path.join(build_dir, response_file_name)
    with open(response_file_path, "w") as f:
      for obj in object_files:
        f.write(f'"{obj}"\n')
    combine_obj_cmd = [
        linker_exe, "/LIB", f"/OUT:{lib_name}", f"@{response_file_name}"
    ]
  else:
    combine_obj_cmd = ["ar", "rcs", lib_name] + object_files
  subprocess.run(combine_obj_cmd, cwd=build_dir, check=True)

  # Copy the final archive to the location expected by GN.
  # We check hashes to avoid unnecessary updates to the "last modified" datum
  # of the file, which can cause GN to unnecessarily rebuild downstream dependencies.
  copy_if_changed(gen_library_path, os.path.join(os.getcwd(), output_path))


if __name__ == "__main__":
  main()
