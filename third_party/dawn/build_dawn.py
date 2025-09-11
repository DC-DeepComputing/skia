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

from cmake_utils import (copy_if_changed, discover_dependencies,
                         quote_if_needed, write_depfile)

def get_cmake_os_cpu(os, cpu):
  # https://stackoverflow.com/a/70498851
  os = os.lower().strip()
  cpu = cpu.lower().strip()
  if os == "android":
    target_cpu_map = {
      "arm": "armeabi-v7a",
      "arm64": "arm64-v8a",
      "x64": "x86_64",
      "x86": "x86",
    }
    return "Android", target_cpu_map[cpu]

  if os == "linux":
    target_cpu_map = {
      "arm": "arm",
      "arm64": "aarch64",
      "x64": "x86_64",
      "x86": "i686",
    }
    return "Linux", target_cpu_map[cpu]

  if os == "mac":
    target_cpu_map = {
      "arm64": "arm64",
      "x64": "x86_64",
    }
    return "Darwin", target_cpu_map[cpu]

  if os == "win":
    target_cpu_map = {
      "arm64": "ARM64",
      "x64": "AMD64",
    }
    return "Windows", target_cpu_map[cpu]

  print("Unsupported OS")
  sys.exit(1)

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
      "--win_sdk", default="", help="Path to the Windows SDK.")
  parser.add_argument(
      "--win_sdk_version", default="", help="Version of the Windows SDK.")
  parser.add_argument(
      "--win_vc", default="", help="Path to Visual C++.")
  parser.add_argument(
      "--win_toolchain_version", default="", help="Version of the MSVC toolchain.")
  args = parser.parse_args()

  cmake_exe = shutil.which("cmake")
  if not cmake_exe:
    print("Error: cmake not found in PATH.")
    sys.exit(1)

  ninja_exe = shutil.which("ninja")
  if not ninja_exe:
    print("Error: ninja not found in PATH.")
    sys.exit(1)

  target_os, target_cpu = get_cmake_os_cpu(args.target_os, args.target_cpu)

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

  configure_cmd = [
      cmake_exe,
      "-S",
      dawn_dir,
      "-B",
      build_dir,
      "-G",
      "Ninja",
      f"-DCMAKE_MAKE_PROGRAM={ninja_exe}",
      f"--install-prefix={os.path.abspath(gen_dir)}",
      f"-DCMAKE_SYSTEM_NAME={target_os}",
      f"-DCMAKE_SYSTEM_PROCESSOR={target_cpu}",
      "-DDAWN_FETCH_DEPENDENCIES=ON",
      "-DDAWN_BUILD_MONOLITHIC_LIBRARY=STATIC",
      "-DCMAKE_BUILD_TYPE=Release",
      # Explicitly set the C++ standard to avoid issues with CMake's feature
      # detection for Clang on Windows.
      "-DCMAKE_CXX_STANDARD=20",
      "-DCMAKE_CXX_STANDARD_REQUIRED=ON",
      "-DCMAKE_CXX_EXTENSIONS=OFF",
      "-DDAWN_ENABLE_INSTALL=OFF",
      "-DTINT_ENABLE_INSTALL=OFF",
      "-DDAWN_BUILD_SAMPLES=OFF",
      "-DDAWN_BUILD_TESTS=OFF",
      "-DTINT_BUILD_TESTS=OFF",
      "-DDAWN_USE_X11=OFF",
  ]

  if target_os == "Windows":
    # The D3D backend requires the HLSL writer.
    configure_cmd.append("-DTINT_BUILD_HLSL_WRITER=ON")
  else:
    configure_cmd.append("-DTINT_BUILD_HLSL_WRITER=OFF")

  cxx_flags_to_add = []
  if target_os == "Windows":
    # Work around CMake bug with clang-cl.exe
    configure_cmd.append("-DABSL_PROPAGATE_CXX_STD=OFF")
    # Force CMake to identify the compiler as MSVC, since clang-cl.exe is a
    # drop-in replacement.
    #configure_cmd.append("-DCMAKE_C_COMPILER_ID=MSVC")
    #configure_cmd.append("-DCMAKE_CXX_COMPILER_ID=MSVC")
    # Skia is built with the static runtime, so Dawn and its dependencies must
    # be as well.
    configure_cmd.append("-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded")
    # Abseil needs to be explicitly told to use the static runtime as well.
    configure_cmd.append("-DABSL_MSVC_STATIC_RUNTIME=ON")
    # The Dawn build can treat warnings as errors, which causes failures
    # with new compiler versions. Explicitly disable this.
    cxx_flags_to_add.append("/WX-")
    # Suppress specific warning that is causing issues with Clang 18.
    cxx_flags_to_add.append("-Wno-assume")

  if target_os != "Android":
    # When we set the android sdk later, it will derive the compiler to use
    configure_cmd.append(f"-DCMAKE_C_COMPILER={quote_if_needed(args.cc)}")
    configure_cmd.append(f"-DCMAKE_CXX_COMPILER={quote_if_needed(args.cxx)}")

  if args.cxx_flags:
    cxx_flags_to_add.extend(args.cxx_flags)

  if cxx_flags_to_add:
    cxx_flags_str = " ".join(cxx_flags_to_add)
    configure_cmd.append(f"-DCMAKE_CXX_FLAGS={cxx_flags_str}")

  ld_flags = args.ld_flags
  if args.win_sdk and args.win_vc:
    # We need to tell the linker where to find the standard Windows libraries.
    um_lib_path = os.path.join(args.win_sdk, "Lib", args.win_sdk_version, "um", args.target_cpu)
    ucrt_lib_path = os.path.join(args.win_sdk, "Lib", args.win_sdk_version, "ucrt", args.target_cpu)
    msvc_lib_path = os.path.join(args.win_vc, "Tools", "MSVC", args.win_toolchain_version, "lib", args.target_cpu)

    # Prepend the library paths to the existing linker flags.
    ld_flags = [
      f"/LIBPATH:{quote_if_needed(um_lib_path.replace(os.sep, '/'))}",
      f"/LIBPATH:{quote_if_needed(ucrt_lib_path.replace(os.sep, '/'))}",
      f"/LIBPATH:{quote_if_needed(msvc_lib_path.replace(os.sep, '/'))}",
    ] + ld_flags

  if ld_flags:
    ld_flags_str = " ".join(ld_flags)
    configure_cmd.append(f"-DCMAKE_EXE_LINKER_FLAGS={ld_flags_str}")
    configure_cmd.append(f"-DCMAKE_SHARED_LINKER_FLAGS={ld_flags_str}")
    configure_cmd.append(f"-DCMAKE_MODULE_LINKER_FLAGS={ld_flags_str}")

  if args.win_sdk:
    rc_path = os.path.join(args.win_sdk, "bin", args.win_sdk_version, args.target_cpu, "rc.exe")
    rc_path = rc_path.replace("\\", "/")
    configure_cmd.append(f"-DCMAKE_RC_COMPILER={quote_if_needed(rc_path)}")
    mt_path = os.path.join(args.win_sdk, "bin", args.win_sdk_version, args.target_cpu, "mt.exe")
    mt_path = mt_path.replace("\\", "/")
    configure_cmd.append(f"-DCMAKE_MT={quote_if_needed(mt_path)}")

  if args.android_ndk_path:
    configure_cmd.append(f"-DCMAKE_TOOLCHAIN_FILE={args.android_ndk_path}/build/cmake/android.toolchain.cmake")
    configure_cmd.append(f"-DANDROID_ABI={target_cpu}")
    configure_cmd.append(f"-DANDROID_PLATFORM={args.android_platform}")

  if target_os == "Darwin" or target_os == "iOS":
    configure_cmd.append(f"-DCMAKE_OSX_ARCHITECTURES={target_cpu}")

  print(" ".join(configure_cmd))
  subprocess.run(configure_cmd, check=True)

  build_cmd = [ninja_exe, "-C", build_dir, "-dkeepdepfile"] + build_targets
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

  copy_if_changed(gen_library_path, os.path.join(os.getcwd(), output_path))

if __name__ == "__main__":
  main()
