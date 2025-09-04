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
import os
import shutil
import subprocess
import sys

from cmake_utils import copy_if_changed,discover_dependencies,write_depfile

def main():
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
      "--output_path", required=True, help="Path to the output library.")
  parser.add_argument(
      "--gen_dir",
      required=True,
      help="Directory for generated files (e.g., depfile and headers).")
  parser.add_argument(
      "--target_os", required=True, help="Target OS for cross-compilation.")
  parser.add_argument(
      "--target_cpu", required=True, help="Target CPU for cross-compilation.")
  parser.add_argument(
      "--android_ndk_path", default="", help="Path to the Android NDK.")
  parser.add_argument(
      "--android_platform", default="", help="Android platform (e.g., android-29).")
  parser.add_argument(
      "--enable_d3d11", action="store_true", help="Enable D3D11 backend.")
  parser.add_argument(
      "--enable_d3d12", action="store_true", help="Enable D3D12 backend.")
  parser.add_argument(
      "--enable_metal", action="store_true", help="Enable Metal backend.")
  parser.add_argument(
      "--enable_opengles",
      action="store_true",
      help="Enable OpenGLES backend.")
  parser.add_argument(
      "--enable_vulkan", action="store_true", help="Enable Vulkan backend.")
  args = parser.parse_args()

  if not (args.enable_d3d11 or args.enable_d3d12 or args.enable_metal or
          args.enable_opengles or args.enable_vulkan):
    print("Error: At least one backend must be enabled.", file=sys.stderr)
    sys.exit(1)

  target_os_map = {
      # Keys are GN target_os values, values are CMake CMAKE_SYSTEM_NAME values.
      "android": "Android",
      "ios": "iOS",
      "linux": "Linux",
      "mac": "Darwin",
      "win": "Windows",
  }

  target_cpu_map = {
      # Keys are GN target_cpu values, values are CMake CMAKE_SYSTEM_PROCESSOR values.
      "arm": "arm",
      "arm64": "aarch64",
      "x64": "x86_64",
      "x86": "i686",
  }

  output_path = args.output_path
  gen_dir = args.gen_dir
  # The headers are a dependency for all libraries.
  # We want to build the other listed dawn components into one big library.
  build_targets = ["webgpu_headers_gen", "dawn_proc", "dawn_native"]
  depfile_path = os.path.join(gen_dir, "libdawn_combined.a.d")

  script_dir = os.path.dirname(os.path.realpath(__file__))

  dawn_dir = os.path.join(script_dir, "..", "externals", "dawn")
  build_dir = os.path.join("cmake", os.path.dirname(output_path), "dawn_build",
    f'{args.target_os}_{args.target_cpu}')

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
      f"-DCMAKE_SYSTEM_NAME={target_os_map[args.target_os]}",
      f"-DCMAKE_SYSTEM_PROCESSOR={target_cpu_map[args.target_cpu]}",
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

  configure_cmd.extend([
      f"-DDAWN_ENABLE_D3D11={'ON' if args.enable_d3d11 else 'OFF'}",
      f"-DDAWN_ENABLE_D3D12={'ON' if args.enable_d3d12 else 'OFF'}",
      f"-DDAWN_ENABLE_METAL={'ON' if args.enable_metal else 'OFF'}",
      f"-DDAWN_ENABLE_OPENGLES={'ON' if args.enable_opengles else 'OFF'}",
      f"-DDAWN_ENABLE_VULKAN={'ON' if args.enable_vulkan else 'OFF'}",
  ])

  if args.target_os != "android":
    # When we set the android sdk later, it will derive the compiler to use
    configure_cmd.append(f"-DCMAKE_C_COMPILER={args.cc}")
    configure_cmd.append(f"-DCMAKE_CXX_COMPILER={args.cxx}")

  if args.cxx_flags:
    cxx_flags = " ".join(args.cxx_flags)
    configure_cmd.append(f"-DCMAKE_CXX_FLAGS={cxx_flags}")
  if args.ld_flags:
    ld_flags = " ".join(args.ld_flags)
    configure_cmd.append(f"-DCMAKE_EXE_LINKER_FLAGS={ld_flags}")
    configure_cmd.append(f"-DCMAKE_SHARED_LINKER_FLAGS={ld_flags}")
    configure_cmd.append(f"-DCMAKE_MODULE_LINKER_FLAGS={ld_flags}")

  if args.android_ndk_path:
    android_abi = ""
    if (args.target_cpu == "arm64"):
      android_abi = "arm64-v8a"
    elif (args.target_cpu == "arm"):
      android_abi = "armeabi-v7a"
    elif (args.target_cpu == "x64"):
      android_abi = "x86_64"
    elif (args.target_cpu == "x86"):
      android_abi = "x86"
    else:
      print("unknown target")
      sys.exit(1)
    configure_cmd.append(f"-DCMAKE_TOOLCHAIN_FILE={args.android_ndk_path}/build/cmake/android.toolchain.cmake")
    configure_cmd.append(f"-DANDROID_ABI={android_abi}")
    configure_cmd.append(f"-DANDROID_PLATFORM={args.android_platform}")

  subprocess.run(configure_cmd, check=True)

  build_cmd = ["ninja", "-C", build_dir, "-dkeepdepfile"] + build_targets
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

  dependencies, object_files = discover_dependencies(build_dir, build_targets)
  write_depfile(output_path, depfile_path, dependencies)

  # Delete generated .a file if it exists already (otherwise ar sometimes chokes)
  lib_name = os.path.basename(output_path)
  gen_library_path = os.path.join(build_dir, lib_name)
  if os.path.exists(gen_library_path):
    os.remove(gen_library_path)

  # Use ar to join all the object files that comprise the necessary libraries
  # and any transitive dependencies into one .a file.
  combine_obj_cmd = ["ar", "rcs", lib_name] + object_files
  subprocess.run(combine_obj_cmd, cwd=build_dir, check=True)

  copy_if_changed(gen_library_path, os.path.join(os.getcwd(), output_path))

if __name__ == "__main__":
  main()
