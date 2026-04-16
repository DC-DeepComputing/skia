#!/usr/bin/env python3
#
# Copyright 2025 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
"""Builds Dawn/Tint using CMake for Bazel.
"""

import argparse
import os
import shutil
import subprocess
import sys

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cc", required=True)
    parser.add_argument("--cxx", required=True)
    parser.add_argument("--ar", required=True)
    parser.add_argument("--cmake_path", required=True)
    parser.add_argument("--ninja_path", required=True)
    parser.add_argument("--repo_root", required=True)
    parser.add_argument("--output_lib", required=True)
    parser.add_argument("--output_headers", required=True)
    parser.add_argument("--build_dir", required=True)
    parser.add_argument("--cxx_flags", action="append", default=[])
    parser.add_argument("--bazel", action="store_true")
    args = parser.parse_args()

    def to_abs(path):
        return os.path.abspath(path)

    # Search for host compilers on Linux instead of using hermetic toolchain
    # to avoid complex trampoline/environment issues.
    def find_host_compiler(pref_name, fallback_name):
        if sys.platform == "linux":
            for name in [pref_name, fallback_name]:
                found = shutil.which(name)
                if found:
                    print(f"Using host compiler: {found}")
                    return found
        return None

    cc = find_host_compiler("clang", "gcc") or to_abs(args.cc)
    cxx = find_host_compiler("clang++", "g++") or to_abs(args.cxx)
    ar = find_host_compiler("ar", "llvm-ar") or to_abs(args.ar)

    repo_root = to_abs(args.repo_root)
    dawn_root = os.path.join(repo_root, "dawn")
    build_dir = to_abs(args.build_dir)
    output_lib = to_abs(args.output_lib)
    output_headers = to_abs(args.output_headers)

    os.makedirs(build_dir, exist_ok=True)

    def get_dep(name):
        return os.path.join(repo_root, name)

    configure_cmd = [
        to_abs(args.cmake_path),
        "-S", dawn_root,
        "-B", build_dir,
        "-G", "Ninja",
        f"-DCMAKE_MAKE_PROGRAM={to_abs(args.ninja_path)}",
        f"-DCMAKE_C_COMPILER={cc}",
        f"-DCMAKE_CXX_COMPILER={cxx}",
        "-DCMAKE_BUILD_TYPE=Release",
        "-DDAWN_FETCH_DEPENDENCIES=OFF",
        "-DDAWN_ENABLE_INSTALL=OFF",
        "-DDAWN_BUILD_MONOLITHIC_LIBRARY=OFF",
      "-DCMAKE_CXX_STANDARD=20",
      "-DCMAKE_CXX_STANDARD_REQUIRED=ON",
      "-DCMAKE_CXX_EXTENSIONS=OFF",
      "-DDAWN_FORCE_SYSTEM_COMPONENT_LOAD=ON", # https://g-issues.chromium.org/issues/399358291
      "-DDAWN_ENABLE_INSTALL=OFF",
      "-DTINT_ENABLE_INSTALL=OFF",

      # TODO(kjlubick) fix for other platforms.
      "-DDAWN_ENABLE_D3D11=OFF",
      "-DDAWN_ENABLE_D3D12=OFF",
      "-DDAWN_ENABLE_OPENGLES=OFF",
      "-DDAWN_ENABLE_METAL=OFF",
      "-DDAWN_ENABLE_VULKAN=ON",

        f"-DDAWN_ABSEIL_DIR={get_dep('abseil-cpp')}",
        f"-DDAWN_EGL_REGISTRY_DIR={get_dep('egl-registry')}",
        f"-DDAWN_GLSLANG_DIR={get_dep('glslang')}",
        f"-DDAWN_JINJA2_DIR={get_dep('jinja2')}",
        f"-DDAWN_MARKUPSAFE_DIR={get_dep('markupsafe')}",
        f"-DDAWN_OPENGL_REGISTRY_DIR={get_dep('opengl-registry')}",
        f"-DDAWN_SPIRV_HEADERS_DIR={get_dep('spirv-headers')}",
        f"-DDAWN_SPIRV_TOOLS_DIR={get_dep('spirv-tools')}",
        f"-DDAWN_VULKAN_HEADERS_DIR={get_dep('vulkan-headers')}",
        f"-DDAWN_VULKAN_UTILITY_LIBRARIES_DIR={get_dep('vulkan-utility-libraries')}",
        f"-DDAWN_WEBGPU_HEADERS_DIR={get_dep('webgpu-headers')}",
        f"-DDAWN_SWIFTSHADER_DIR={get_dep('swiftshader')}",

        f"-DVulkanHeaders_DIR={build_dir}/vulkan-headers/src",

        "-DDAWN_BUILD_BENCHMARKS=OFF",
        "-DDAWN_BUILD_SAMPLES=OFF",
        "-DDAWN_BUILD_TESTS=OFF",
        "-DTINT_BUILD_BENCHMARKS=OFF",
        "-DTINT_BUILD_TESTS=OFF",
    "-DDAWN_USE_GLFW=OFF",
    "-DTINT_BUILD_BENCHMARKS=OFF",
    "-DTINT_BUILD_IR_BINARY=OFF",
    "-DTINT_BUILD_TESTS=OFF",
    "-DDAWN_EMDAWNWEBGPU_DIR=NOT_SYNCED_BY_SKIA",
    "-DDAWN_GLFW_DIR=NOT_SYNCED_BY_SKIA",
    "-DDAWN_LPM_DIR=NOT_SYNCED_BY_SKIA",
    "-DDAWN_PROTOBUF_DIR=NOT_SYNCED_BY_SKIA",
    "-DDAWN_BUILD_PROTOBUF=OFF",
    "-DTINT_BUILD_WGSL_READER=ON",

    "-DDAWN_USE_X11=ON",
    ]

    env = os.environ.copy()
    env["PYTHONPATH"] = os.pathsep.join([repo_root, os.path.join(dawn_root, "generator")])
    env["PYTHONDONTWRITEBYTECODE"] = "1"

    print("Running CMake configure...")
    subprocess.run(configure_cmd, check=True, env=env)

    build_targets = ["webgpu_headers_gen", "dawn_proc", "dawn_native"]

    print("Running Ninja build...")
    subprocess.run([to_abs(args.ninja_path), "-C", build_dir] + build_targets, check=True, env=env)

    # Discover and combine object files.
    # We use ninja -tinputs to find all object files that are part of the build_targets.
    def discover_object_files(build_dir, targets, ninja_path):
        worklist = set(targets)
        seen_targets = set(targets)
        object_files = set()

        while worklist:
            cmd = [to_abs(ninja_path), "-C", build_dir, "-tinputs"] + list(worklist)
            # Filter out targets that were already processed in this batch if any (though worklist is a set)
            output = subprocess.check_output(cmd).decode("utf-8").splitlines()
            worklist = set()
            for line in output:
                line = line.strip()
                if line.endswith(".o") or line.endswith(".obj"):
                    object_files.add(line)
                elif line and not os.path.isabs(line) and line not in seen_targets:
                    # It's a derived target/dependency, recurse.
                    worklist.add(line)
                    seen_targets.add(line)

        result = list(object_files)
        result.sort()
        return result

    print("Discovering object files...")
    object_files = discover_object_files(build_dir, build_targets, args.ninja_path)

    print(f"Combining {len(object_files)} object files into {output_lib}...")
    # Delete if exists to avoid 'ar' appending issues
    if os.path.exists(output_lib):
        os.remove(output_lib)

    if sys.platform == "win32":
        # On Windows, we use a response file because the command line can be too long.
        rsp_file = os.path.join(build_dir, "objs.rsp")
        with open(rsp_file, "w") as f:
            for obj in object_files:
                f.write(f'"{obj}"\n')
        # We use the provided 'ar' which might be lib.exe or llvm-ar.exe
        subprocess.run([ar, "/LIB", f"/OUT:{output_lib}", f"@{rsp_file}"], cwd=build_dir, check=True)
    else:
        # Use a response file for linux/mac as well just in case.
        rsp_file = os.path.join(build_dir, "objs.rsp")
        with open(rsp_file, "w") as f:
            f.write(" ".join(object_files))
        subprocess.run([ar, "rcs", output_lib, f"@{rsp_file}"], cwd=build_dir, check=True)

    # Copy generated and static headers
    print(f"Copying headers to {output_headers}...")
    if os.path.exists(output_headers):
        shutil.rmtree(output_headers)
    os.makedirs(output_headers)

    def copy_only_headers(src_dir, dst_dir):
        if not os.path.exists(src_dir):
            return
        for root, _, files in os.walk(src_dir):
            for f in files:
                if f.endswith((".h", ".hpp", ".inc")):
                    rel_path = os.path.relpath(os.path.join(root, f), src_dir)
                    dest_path = os.path.join(dst_dir, rel_path)
                    os.makedirs(os.path.dirname(dest_path), exist_ok=True)
                    shutil.copy2(os.path.join(root, f), dest_path)

    # Static headers from the source tree
    static_include = os.path.join(dawn_root, "include")
    for d in ["dawn", "webgpu", "tint"]:
        copy_only_headers(os.path.join(static_include, d), os.path.join(output_headers, d))

    # Source headers (needed by Skia as src/tint/... and src/dawn/...)
    for d in ["dawn", "tint"]:
        copy_only_headers(os.path.join(dawn_root, "src", d), os.path.join(output_headers, "src", d))

    # Generated headers from the build directory
    gen_include = os.path.join(build_dir, "gen", "include")
    for d in ["dawn", "webgpu"]:
        copy_only_headers(os.path.join(gen_include, d), os.path.join(output_headers, d))

if __name__ == "__main__":
    main()
