#!/usr/bin/env python3
import os
import re
import sys

def parse_cmake_list(text):
    """Simple parser for CMake lists like (file1 file2 "file 3")"""
    files = []
    text = re.sub(r'#.*', '', text)
    matches = re.findall(r'"([^"]*)"|([^\s()]+)', text)
    for m in matches:
        f = m[0] or m[1]
        if f:
            files.append(f)
    return files

def extract_variable(cmake_content, var_name):
    """Extracts a variable set using set(var_name ...) or list(APPEND var_name ...)"""
    set_match = re.search(r'set\(\s*' + re.escape(var_name) + r'\s+([^)]+)\)', cmake_content, re.IGNORECASE | re.DOTALL)
    files = []
    if set_match:
        files.extend(parse_cmake_list(set_match.group(1)))

    append_matches = re.finditer(r'list\(APPEND\s+' + re.escape(var_name) + r'\s+([^)]+)\)', cmake_content, re.IGNORECASE | re.DOTALL)
    for am in append_matches:
        files.extend(parse_cmake_list(am.group(1)))

    return sorted(list(set(files)))

def resolve_variables(files, cmake_content):
    """Resolves ${var} references in a list of files using the provided cmake_content."""
    resolved = []
    for f in files:
        if f.startswith("${") and f.endswith("}"):
            var_name = f[2:-1]
            if var_name in ["DAWN_INCLUDE_DIR", "DAWN_SRC_DIR"]:
                resolved.append(f)
                continue
            expanded = extract_variable(cmake_content, var_name)
            resolved.extend(resolve_variables(expanded, cmake_content))
        else:
            resolved.append(f)
    return resolved

def clean_path(f):
    """Replaces standard Dawn path variables and cleans up result."""
    f = f.replace("${DAWN_INCLUDE_DIR}/", "include/")
    f = f.replace("${DAWN_SRC_DIR}/", "src/")
    return f

def get_dawn_files(dawn_root):
    common_cmake_path = os.path.join(dawn_root, "src/dawn/common/CMakeLists.txt")
    native_cmake_path = os.path.join(dawn_root, "src/dawn/native/CMakeLists.txt")
    wire_cmake_path = os.path.join(dawn_root, "src/dawn/wire/CMakeLists.txt")
    platform_cmake_path = os.path.join(dawn_root, "src/dawn/platform/CMakeLists.txt")
    utils_cmake_path = os.path.join(dawn_root, "src/dawn/utils/CMakeLists.txt")

    with open(common_cmake_path) as f: common_content = f.read()
    with open(native_cmake_path) as f: native_content = f.read()
    with open(wire_cmake_path) as f: wire_content = f.read()
    with open(platform_cmake_path) as f: platform_content = f.read()
    with open(utils_cmake_path) as f: utils_content = f.read()

    def prepend(files, prefix):
        res = []
        for f in files:
            f = clean_path(f)
            if f.startswith("include/") or f.startswith("src/"):
                res.append(f)
            else:
                res.append(os.path.join(prefix, f))
        return res

    def filter_by_dir(files, dir_name):
        return [f for f in files if f.startswith(dir_name + "/")]

    def filter_out_dirs(files, dirs):
        return [f for f in files if not any(f.startswith(d + "/") for d in dirs)]

    def extract_from_add_library(content, target_name, section):
        pattern = r'dawn_add_library\(\s*' + re.escape(target_name) + r'.*?' + re.escape(section) + r'\s+(.*?)(?:\s+[A-Z_]+\s+|$|\))'
        match = re.search(pattern, content, re.IGNORECASE | re.DOTALL)
        if match:
            raw_files = parse_cmake_list(match.group(1))
            resolved = resolve_variables(raw_files, content)

            final = []
            for f in resolved:
                f = clean_path(f)
                if not f.startswith("${"):
                    final.append(f)
            return final
        return []

    data = {}

    # Public headers that we move to BUILD.bazel
    public_native_headers = [
        "include/dawn/native/D3D11Backend.h", "include/dawn/native/D3D12Backend.h", "include/dawn/native/D3DBackend.h",
        "include/dawn/native/dawn_native_export.h", "include/dawn/native/DawnNative.h", "include/dawn/native/MetalBackend.h",
        "include/dawn/native/NullBackend.h", "include/dawn/native/OpenGLBackend.h", "include/dawn/native/VulkanBackend.h",
        "include/dawn/native/WebGPUBackend.h",
    ]

    # --- NATIVE ---
    native_all_sources = resolve_variables(extract_variable(native_content, "sources"), native_content)
    native_all_hdrs = resolve_variables(extract_variable(native_content, "private_headers"), native_content)
    native_base_hdrs = resolve_variables(extract_variable(native_content, "headers"), native_content)

    backends = ["vulkan", "opengl", "metal", "d3d11", "d3d12", "null", "webgpu"]

    # Base native files
    data["DAWN_NATIVE_SRCS"] = prepend(filter_out_dirs(native_all_sources, backends + ["d3d"]), "src/dawn/native")
    data["DAWN_NATIVE_HDRS"] = prepend(filter_out_dirs(native_all_hdrs + native_base_hdrs, backends + ["d3d"]), "src/dawn/native")

    for b in backends:
        data[f"DAWN_NATIVE_{b.upper()}_SRCS"] = prepend(filter_by_dir(native_all_sources, b), "src/dawn/native")
        data[f"DAWN_NATIVE_{b.upper()}_HDRS"] = prepend(filter_by_dir(native_all_hdrs + native_base_hdrs, b), "src/dawn/native")

    # Fix D3D (some files are in d3d/ common dir)
    d3d_sources = prepend(filter_by_dir(native_all_sources, "d3d"), "src/dawn/native")
    d3d_hdrs = prepend(filter_by_dir(native_all_hdrs + native_base_hdrs, "d3d"), "src/dawn/native")
    data["DAWN_NATIVE_D3D11_SRCS"].extend(d3d_sources)
    data["DAWN_NATIVE_D3D11_HDRS"].extend(d3d_hdrs)
    data["DAWN_NATIVE_D3D12_SRCS"].extend(d3d_sources)
    data["DAWN_NATIVE_D3D12_HDRS"].extend(d3d_hdrs)

    # Special handling for X11, RenderDoc and Vulkan Linux
    data["DAWN_NATIVE_X11_SRCS"] = prepend([f for f in native_all_sources if "X11" in f], "src/dawn/native")
    data["DAWN_NATIVE_X11_HDRS"] = prepend([f for f in native_all_hdrs if "X11" in f], "src/dawn/native")
    data["DAWN_NATIVE_RENDERDOC_SRCS"] = prepend([f for f in native_all_sources + native_all_hdrs if "RenderDoc" in f], "src/dawn/native")

    def is_android_native(f):
        return "AHBFunctions" in f

    vulkan_linux_patterns = ["DmaBuf", "OpaqueFD", "semaphore/SemaphoreServiceImplementationFD"]
    vulkan_android_patterns = ["AHardwareBuffer"]

    def is_vulkan_linux(f):
        return "vulkan/external" in f and any(p in f for p in vulkan_linux_patterns)

    data["DAWN_NATIVE_VULKAN_LINUX_SRCS"] = prepend([f for f in native_all_sources if is_vulkan_linux(f)], "src/dawn/native")
    data["DAWN_NATIVE_VULKAN_LINUX_HDRS"] = prepend([f for f in native_all_hdrs if is_vulkan_linux(f)], "src/dawn/native")

    # Remove duplicates from base and backend lists
    # Note: data[...] entries already have "src/dawn/native" prepended
    data["DAWN_NATIVE_VULKAN_SRCS"] = [f for f in data["DAWN_NATIVE_VULKAN_SRCS"] if not any(p in f for p in vulkan_linux_patterns + vulkan_android_patterns)]
    data["DAWN_NATIVE_VULKAN_HDRS"] = [f for f in data["DAWN_NATIVE_VULKAN_HDRS"] if not any(p in f for p in vulkan_linux_patterns + vulkan_android_patterns)]

    data["DAWN_NATIVE_SRCS"] = [f for f in data["DAWN_NATIVE_SRCS"] if "X11" not in f and "RenderDoc" not in f and not f.startswith("src/dawn/native/utils/") and not f.endswith(".mm") and "AHBFunctions" not in f]
    data["DAWN_NATIVE_HDRS"] = [f for f in data["DAWN_NATIVE_HDRS"] if "X11" not in f and "RenderDoc" not in f and not f.startswith("src/dawn/native/utils/") and "AHBFunctions" not in f]

    # Remove duplicated public headers from ALL native lists
    for var in list(data.keys()):
        if var.startswith("DAWN_NATIVE_"):
            data[var] = [f for f in data[var] if f not in public_native_headers]

    # Manual Overrides for Native
    data["DAWN_NATIVE_SRCS"].append("src/dawn/native/SpirvValidation.cpp")

    data["DAWN_NATIVE_UTILS_SRCS"] = prepend([f for f in native_all_sources if f.startswith("utils/") and "WGPUHelpers" in f], "src/dawn/native")
    data["DAWN_NATIVE_UTILS_HDRS"] = prepend([f for f in native_all_hdrs if f.startswith("utils/") and "WGPUHelpers" in f], "src/dawn/native")

    # --- COMMON ---
    common_all_sources = resolve_variables(extract_variable(common_content, "sources"), common_content)
    common_all_hdrs = resolve_variables(extract_variable(common_content, "private_headers"), common_content)

    def is_windows(f): return "WindowsUtils" in f or "windows_with_undefs" in f
    def is_objc(f): return f.endswith("_mac.mm") or "IOSurface" in f

    data["DAWN_COMMON_SRCS"] = prepend([f for f in common_all_sources if not is_objc(f) and not is_windows(f)], "src/dawn/common")
    data["DAWN_COMMON_HDRS"] = prepend([f for f in common_all_hdrs if not is_objc(f) and not is_windows(f)], "src/dawn/common")
    data["DAWN_COMMON_OBJC_SRCS"] = prepend([f for f in common_all_sources if is_objc(f)], "src/dawn/common")
    data["DAWN_COMMON_OBJC_HDRS"] = prepend([f for f in common_all_hdrs if is_objc(f)], "src/dawn/common")

    # Manual Overrides for Common
    data["DAWN_COMMON_HDRS"].append("src/dawn/common/Strings.h")

    # --- WIRE ---
    data["DAWN_WIRE_SRCS"] = prepend(extract_from_add_library(wire_content, "dawn_wire", "SOURCES"), "src/dawn/wire")
    data["DAWN_WIRE_HDRS"] = prepend(extract_from_add_library(wire_content, "dawn_wire", "PRIVATE_HEADERS"), "src/dawn/wire")
    data["DAWN_WIRE_HDRS"].extend(extract_from_add_library(wire_content, "dawn_wire", "HEADERS"))

    # --- PLATFORM ---
    data["DAWN_PLATFORM_SRCS"] = prepend(extract_from_add_library(platform_content, "dawn_platform", "SOURCES"), "src/dawn/platform")
    data["DAWN_PLATFORM_HDRS"] = prepend(extract_from_add_library(platform_content, "dawn_platform", "PRIVATE_HEADERS"), "src/dawn/platform")
    data["DAWN_PLATFORM_HDRS"].extend(extract_from_add_library(platform_content, "dawn_platform", "HEADERS"))

    # --- SYSTEM UTILS ---
    data["DAWN_UTILS_SRCS"] = prepend(extract_from_add_library(utils_content, "dawn_system_utils", "SOURCES"), "src/dawn/utils")
    ag_sys_sources = data["DAWN_UTILS_SRCS"]
    data["DAWN_UTILS_SRCS"] = [f for f in ag_sys_sources if "Posix" not in f and "OSX" not in f and "ObjC" not in f and "Empty" not in f and "Windows" not in f]
    data["DAWN_UTILS_POSIX_SRCS"] = [f for f in ag_sys_sources if "Posix" in f or "Empty" in f]
    data["DAWN_UTILS_OBJC_SRCS"] = [f for f in ag_sys_sources if "OSX" in f or "ObjC" in f]

    data["DAWN_UTILS_HDRS"] = prepend(extract_from_add_library(utils_content, "dawn_system_utils", "PRIVATE_HEADERS"), "src/dawn/utils")
    ag_sys_hdrs = data["DAWN_UTILS_HDRS"]
    data["DAWN_UTILS_HDRS"] = [f for f in ag_sys_hdrs if "ObjC" not in f and "Windows" not in f]
    data["DAWN_UTILS_OBJC_HDRS"] = [f for f in ag_sys_hdrs if "ObjC" in f]

    return data

def main():
    if len(sys.argv) < 3:
        print("Usage: generate_dawn_files.py <dawn_root> <output_bzl>")
        sys.exit(1)

    dawn_root = sys.argv[1]
    output_bzl = sys.argv[2]
    files_data = get_dawn_files(dawn_root)

    with open(output_bzl, "w") as f:
        f.write('"""\nThis file contains the list of files used to build Dawn.\n\nGenerated by generate_dawn_files.py\n"""\n\n')
        for var_name, files in sorted(files_data.items()):
            f.write(f"{var_name} = [\n")
            for file_path in sorted(list(set(files))):
                f.write(f'    "{file_path}",\n')
            f.write("]\n\n")

if __name__ == "__main__":
    main()
