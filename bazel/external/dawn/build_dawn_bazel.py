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
        "-DDAWN_FORCE_SYSTEM_COMPONENT_LOAD=ON",
        
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
        
        "-DDAWN_BUILD_BENCHMARKS=OFF",
        "-DDAWN_BUILD_SAMPLES=OFF",
        "-DDAWN_BUILD_TESTS=OFF",
        "-DTINT_BUILD_BENCHMARKS=OFF",
        "-DTINT_BUILD_TESTS=OFF",
    ]

    # Only pass flags if we are using the hermetic toolchain (not host)
    # or if they are basic enough. For now, let's skip them if using host
    # to avoid path issues again.
    if cc == to_abs(args.cc) and args.cxx_flags:
         configure_cmd.append(f"-DCMAKE_CXX_FLAGS={' '.join(args.cxx_flags)}")

    env = os.environ.copy()
    env["PYTHONPATH"] = repo_root
    env["PYTHONDONTWRITEBYTECODE"] = "1"

    print("Running CMake configure...")
    subprocess.run(configure_cmd, check=True, env=env)

    build_targets = ["webgpu_headers_gen", "dawn_proc", "dawn_native"]
    
    print("Running Ninja build...")
    subprocess.run([to_abs(args.ninja_path), "-C", build_dir] + build_targets, check=True, env=env)

    object_files = []
    for root, _, files in os.walk(build_dir):
        for f in files:
            if f.endswith(".o") or f.endswith(".obj"):
                object_files.append(os.path.join(root, f))
    
    print(f"Combining {len(object_files)} object files into {output_lib}...")
    if sys.platform == "win32":
        subprocess.run([ar, "/LIB", f"/OUT:{output_lib}"] + object_files, check=True)
    else:
        rsp_file = os.path.join(build_dir, "objs.rsp")
        with open(rsp_file, "w") as f:
            f.write(" ".join(object_files))
        subprocess.run([ar, "rcs", output_lib, f"@{rsp_file}"], check=True)

    gen_include = os.path.join(build_dir, "gen", "include")
    if os.path.exists(output_headers):
        shutil.rmtree(output_headers)
    os.makedirs(output_headers)
    
    for d in ["dawn", "webgpu"]:
        src = os.path.join(gen_include, d)
        if os.path.exists(src):
            shutil.copytree(src, os.path.join(output_headers, d), dirs_exist_ok=True)

if __name__ == "__main__":
    main()
