"""
This file assembles a toolchain for a Windows host using a hermetic MSVC.

It downloads the necessary executables and creates symlinks in the external subfolder of the Bazel
cache (the same place third party deps are downloaded with http_archive or similar functions in
WORKSPACE.bazel). These will be able to be used via our
custom c++ toolchain configuration (see //toolchain/windows_msvc_toolchain_config.bzl)

The destination folder for these files and symlinks are:
  [outputRoot (aka Bazel cache)]/[outputUserRoot]/[outputBase]/external/msvc_windows_amd64
  (See https://bazel.build/docs/output_directories#layout-diagram)
"""

load("//bazel:cipd_install.bzl", "cipd_download_urls")
load(":utils.bzl", "gcs_mirror_url")

win_toolchain_pkg = "skia/bots/win_toolchain"
win_toolchain_tag = "version:15"
win_toolchain_sha256 = "f3dfb040052661124470d959faf8350aef4c2da9b396ad191a2c922d7a01c43f"

def _download_windows_amd64_toolchain_impl(ctx):
    # Download the win_toolchain CIPD package (the extraction can take a while).
    ctx.download_and_extract(
        url = cipd_download_urls(win_toolchain_pkg, win_toolchain_sha256, win_toolchain_tag),
        output = "",
        sha256 = win_toolchain_sha256,
        type = "zip",
    )

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
load(":vars.bzl", "MSVC_BIN", "MSVC_INCLUDE", "MSVC_LIB", "WIN_SDK_INCLUDE", "WIN_SDK_LIB")

filegroup(
    name = "archive_files",
    srcs = [
        MSVC_BIN + "/lib.exe",
    ],
    visibility = ["//visibility:public"],
)

filegroup(
    name = "compile_files",
    srcs = [
        MSVC_BIN + "/cl.exe",
    ] + glob(
        include = [
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
        MSVC_BIN + "/link.exe",
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

    # Find the correct version of MSVC and the Windows SDK.
    if ctx.os.name.lower().find("windows") != -1:
        msvc_version = ctx.execute(["powershell.exe", "/c", "(Get-ChildItem -Path VC/Tools/MSVC | sort Name | select -Last 1).BaseName"]).stdout.rstrip()
        win_sdk_version = ctx.execute(["powershell.exe", "/c", "(Get-ChildItem -Path win_sdk/Include | sort Name | select -Last 1).BaseName"]).stdout.rstrip()
    else:
        msvc_version = ctx.execute(["bash", "-c", "ls VC/Tools/MSVC | sort | tail -1"]).stdout.rstrip()
        win_sdk_version = ctx.execute(["bash", "-c", "ls win_sdk/Include | sort | tail -1"]).stdout.rstrip()

    # Write vars.bzl.
    ctx.file(
        "vars.bzl",
        content = """
MSVC_VERSION = "%s"
MSVC_INCLUDE = "VC/Tools/MSVC/" + MSVC_VERSION + "/include"
MSVC_LIB = "VC/Tools/MSVC/" + MSVC_VERSION + "/lib"
MSVC_BIN = "VC/Tools/MSVC/" + MSVC_VERSION + "/bin/Hostx64/x64"
WIN_SDK_VERSION = "%s"
WIN_SDK_INCLUDE = "win_sdk/Include/" + WIN_SDK_VERSION
WIN_SDK_LIB = "win_sdk/Lib/" + WIN_SDK_VERSION
""" % (msvc_version, win_sdk_version),
        executable = False,
    )

# https://bazel.build/rules/repository_rules
download_windows_msvc_amd64_toolchain = repository_rule(
    implementation = _download_windows_amd64_toolchain_impl,
    attrs = {},
    doc = "Downloads MSVC to build Skia with.",
)
