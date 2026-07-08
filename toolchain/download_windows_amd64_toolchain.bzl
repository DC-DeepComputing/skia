"""
This file assembles a toolchain for a Windows host using the Clang Compiler.

It downloads the necessary executables and creates symlinks in the external subfolder of the Bazel
cache (the same place third party deps are downloaded with http_archive or similar functions in
WORKSPACE.bazel). These will be able to be used via our
custom c++ toolchain configuration (see //toolchain/windows_toolchain_config.bzl)

The destination folder for these files and symlinks are:
  [outputRoot (aka Bazel cache)]/[outputUserRoot]/[outputBase]/external/clang_windows_amd64
  (See https://bazel.build/docs/output_directories#layout-diagram)
"""

load("//bazel:cipd_install.bzl", "cipd_download_urls")

# This is currently clang 23 (see //infra/bots/assets/clang_win/create.py)
win_clang_pkg = "skia/bots/clang_win"
win_clang_tag = "version:19"
win_clang_sha256 = "6e5416ab9756a6e75804f75bbb006d61b1bd39677284223789e9a060d32fb8c1"

# When updating this, update vars.bzl below
win_toolchain_pkg = "skia/bots/win_toolchain"
win_toolchain_tag = "version:16"
win_toolchain_sha256 = "57d5317c61cd7e10352bcf8b6b3c0983f37cf8f814c382a6a7c9938c4bb87210"

def _download_windows_amd64_toolchain_impl(ctx):
    # Download the clang toolchain (the extraction can take a while)
    # https://bazel.build/rules/lib/repository_ctx#download_and_extract
    ctx.download_and_extract(
        url = cipd_download_urls(win_clang_pkg, win_clang_sha256, win_clang_tag),
        output = "",
        sha256 = win_clang_sha256,
        type = "zip",
    )

    # Symlink compiler and linker binaries to GCC-compatible names.
    # https://bazel.build/rules/lib/builtins/actions#symlink
    ctx.symlink("bin/clang-cl.exe", "bin/clang.exe")
    ctx.symlink("bin/lld-link.exe", "bin/ld.lld.exe")
    ctx.symlink("bin/lld-link.exe", "bin/lld.exe")

    # Create a BUILD.bazel file that makes the files necessary for compiling,
    # linking and creating archive files visible to Bazel.
    # The smaller the globs are, the more performant the sandboxed builds will be.
    # Additionally, globs that are too wide can pick up infinite symlink loops,
    # and be difficult to quash: https://github.com/bazelbuild/bazel/issues/13950
    # https://bazel.build/rules/lib/repository_ctx#file
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
        output = "win_toolchain",
        sha256 = win_toolchain_sha256,
        type = "zip",
    )

    # Link the VC and win_sdk directories to the root of the repository.
    ctx.symlink("win_toolchain/VC", "VC")
    ctx.symlink("win_toolchain/Windows Kits/10", "win_sdk")

    # Write vars.bzl.
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
    doc = "Downloads clang and MSVC to build Skia on Windows with.",
)
