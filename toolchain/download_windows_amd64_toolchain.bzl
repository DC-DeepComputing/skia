"""
This file assembles a toolchain for a Windows host using the Clang Compiler.

It downloads the necessary executables and creates symlinks in the external subfolder of the Bazel
cache. These will be able to be used via our custom c++ toolchain configuration
(see //toolchain/windows_toolchain_config.bzl).
"""

load("//bazel:cipd_install.bzl", "cipd_download_urls")

# Download the exact same clang version as GN (from skia/bots/clang_win)
win_clang_pkg = "skia/bots/clang_win"
win_clang_tag = "version:19"
win_clang_sha256 = "6e5416ab9756a6e75804f75bbb006d61b1bd39677284223789e9a060d32fb8c1"

win_toolchain_pkg = "skia/bots/win_toolchain"
win_toolchain_tag = "version:16"
win_toolchain_sha256 = "57d5317c61cd7e10352bcf8b6b3c0983f37cf8f814c382a6a7c9938c4bb87210"

def _download_windows_amd64_toolchain_impl(ctx):
    # Download the clang-win asset from CIPD
    ctx.download_and_extract(
        url = cipd_download_urls(win_clang_pkg, win_clang_sha256, win_clang_tag),
        output = "",
        sha256 = win_clang_sha256,
        type = "zip",
    )

    # Symlink compiler and linker binaries to GCC-compatible names.
    # On Windows, Bzlmod runs natively in Java and creates correct link configurations.
    ctx.symlink("bin/clang-cl.exe", "bin/clang.exe")
    ctx.symlink("bin/lld-link.exe", "bin/ld.lld.exe")
    ctx.symlink("bin/lld-link.exe", "bin/lld.exe")

    # Create a BUILD.bazel file that makes the files necessary for compiling,
    # linking and creating archive files visible to Bazel.
    ctx.file(
        "BUILD.bazel",
        content = """
# DO NOT EDIT THIS BAZEL FILE DIRECTLY
# Generated from ctx.file action in download_windows_amd64_toolchain.bzl
load(":vars.bzl", "MSVC_INCLUDE", "MSVC_LIB", "WIN_SDK_INCLUDE", "WIN_SDK_LIB")

filegroup(
    name = "archive_files",
    srcs = [
        "bin/lld-link.exe",
    ],
    visibility = ["//visibility:public"],
)

filegroup(
    name = "compile_files",
    srcs = [
        "bin/clang.exe",
    ] + glob(
        include = [
            "lib/clang/23/include/**",
            MSVC_INCLUDE + "/**",
            WIN_SDK_INCLUDE + "/**",
        ],
        allow_empty = False,
    ),
    visibility = ["//visibility:public"],
)

filegroup(
    name = "link_files",
    srcs = [
        "bin/clang.exe",
        "bin/ld.lld.exe",
        "bin/lld.exe",
    ] + glob(
        [
            MSVC_LIB + "/**",
            WIN_SDK_LIB + "/**",
        ],
        allow_empty = False,
    ),
    visibility = ["//visibility:public"],
)
""",
        executable = False,
    )

    # Download the win_toolchain CIPD package.
    # We extract it to a subfolder to prevent CIPD metadata file collisions with the clang_win package.
    ctx.download_and_extract(
        url = cipd_download_urls(win_toolchain_pkg, win_toolchain_sha256, win_toolchain_tag),
        output = "win_toolchain_temp",
        sha256 = win_toolchain_sha256,
        type = "zip",
    )

    # Link the VC and win_sdk directories to the root of the repository.
    # Bazel's ctx.symlink is built-in, cross-platform, and handles junction/link creation natively on Windows.
    ctx.symlink("win_toolchain_temp/VC", "VC")
    ctx.symlink("win_toolchain_temp/Windows Kits/10", "win_sdk")

    # Write vars.bzl.
    # The CIPD package skia/bots/win_toolchain (tag: version:16) always contains:
    # - MSVC Toolset: 14.51.36231
    # - Windows SDK: 10.0.26100.0
    # Specifying these directly avoids spawning slow, platform-dependent shell processes.
    ctx.file(
        "vars.bzl",
        content = """
MSVC_VERSION = "14.51.36231"
MSVC_INCLUDE = "VC/Tools/MSVC/" + MSVC_VERSION + "/include"
MSVC_LIB = "VC/Tools/MSVC/" + MSVC_VERSION + "/lib"
WIN_SDK_VERSION = "10.0.26100.0"
WIN_SDK_INCLUDE = "win_sdk/Include/" + WIN_SDK_VERSION
WIN_SDK_LIB = "win_sdk/Lib/" + WIN_SDK_VERSION
""",
        executable = False,
    )

# https://bazel.build/rules/repository_rules
download_windows_amd64_toolchain = repository_rule(
    implementation = _download_windows_amd64_toolchain_impl,
    attrs = {},
    doc = "Downloads clang and msvc to build Skia on Windows with.",
)
