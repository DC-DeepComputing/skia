"""
This file assembles a toolchain for a Windows host using the Clang Compiler.

It downloads the necessary executables and creates symlinks in the external subfolder of the Bazel
cache (the same place third party deps are downloaded with http_archive or similar functions in
WORKSPACE.bazel). These will be able to be used via our
custom c++ toolchain configuration (see //toolchain/windows_toolchain_config.bzl)

The destination folder for these files and symlinks are:
  [outputRoot (aka Bazel cache)]/[outputUserRoot]/[outputBase]/external/clang_windows
  (See https://bazel.build/docs/output_directories#layout-diagram)
"""

load(":clang_layering_check.bzl", "generate_system_module_map")
load(":utils.bzl", "gcs_mirror_url")

# From https://github.com/llvm/llvm-project/releases/tag/llvmorg-15.0.1
# When updating this, don't forget to use //bazel/gcs_mirror to upload a new version.
# go run bazel/gcs_mirror/gcs_mirror.go --url [clang_url] --sha256 [clang_sha256]
clang_prefix = "clang+llvm-18.1.8-x86_64-pc-windows-msvc"
clang_sha256 = "22c5907db053026cc2a8ff96d21c0f642a90d24d66c23c6d28ee7b1d572b82e8"
clang_url = "https://github.com/llvm/llvm-project/releases/download/llvmorg-18.1.8/clang+llvm-18.1.8-x86_64-pc-windows-msvc.tar.xz"


def _get_system_msvc_path(ctx):
    ctx.file(
        "find_msvc.py",
        content="""
import os
import subprocess


def get_system_msvc_path():
    # VCToolsInstallDir and associated environment variables are set when
    # running in a Developer Console. If that's the case, don't bother looking
    # elsewhere.  This also gives the user the opportunity to override the
    # location if desired.
    vctools_install_dir = os.getenv('VCToolsInstallDir')
    if vctools_install_dir:
        return vctools_install_dir.rstrip().rstrip('\\\\')

    # Attempt to use vswhere to find the installation path.
    program_files_x86 = os.getenv('ProgramFiles(x86)')
    if not program_files_x86:
        raise Exception('Unable to find Program Files (x86)')
    vswhere = os.path.join(
        program_files_x86,
        'Microsoft Visual Studio', 'Installer', 'vswhere.exe')
    output = subprocess.check_output(
        args=[vswhere,
              '-latest',
              '-products', '*',
              '-requires', 'Microsoft.Component.MSBuild',
              '-property', 'installationPath',
        ]).decode()
    vctools_prefix = os.path.join(output.rstrip(), 'VC', 'Tools', 'MSVC')

    # There's a version-numbered directory under MSVC. Find the latest version
    # and use that.
    subdirs = os.listdir(vctools_prefix)
    subdirs.sort(reverse=True)
    return vctools_prefix + "\\\\" + subdirs[0].rstrip()


if __name__ == '__main__':
    print(get_system_msvc_path())
""",
        executable=False,
    )
    res = ctx.execute(["python.exe", ctx.path("find_msvc.py")])
    if res.return_code != 0:
        fail("Failed to find MSVC path: " + res.stderr)
    return res.stdout.rstrip()


def _delete_msvc_symlinks(ctx):
    ctx.delete("./symlinks/msvc/include")
    ctx.delete("./symlinks/msvc/lib")

def _create_msvc_symlinks(ctx):
    system_msvc_path = _get_system_msvc_path(ctx)

    # https://bazel.build/rules/lib/actions#symlink
    ctx.symlink(
        # from =
        system_msvc_path + "/include",
        # to =
        "./symlinks/msvc/include",
    )

    # It is very important to symlink the frameworks directory to [sysroot]/System/Library/Frameworks
    # because some Frameworks "re-export" other frameworks. These framework paths are relative to
    # the sysroot (which on a typical machine is /), and it is difficult to change these paths.
    # By making the symlinks emulate the original path structure, we can keep those re-exports
    # from breaking.
    ctx.symlink(
        # from =
        system_msvc_path + "/lib",
        # to =
        "./symlinks/msvc/lib",
    )


def _download_windows_toolchain_impl(ctx):
    system_msvc_path = _get_system_msvc_path(ctx)

    # Download the clang toolchain (the extraction can take a while)
    # https://bazel.build/rules/lib/repository_ctx#download_and_extract
    ctx.download_and_extract(
        url = gcs_mirror_url(clang_url, clang_sha256),
        output = "",
        stripPrefix = clang_prefix,
        sha256 = clang_sha256,
    )

    # Symlink the local MSVS install.
    _delete_msvc_symlinks(ctx)
    _create_msvc_symlinks(ctx)

    # This list of files lines up with _make_default_flags() in windows_toolchain_config.bzl
    # It is all locations that our toolchain could find a system header.
    builtin_include_directories = [
        "include/c++/v1",
        "lib/clang/18/include",
    ]

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
# Generated from ctx.file action in download_windows_toolchain.bzl
filegroup(
    name = "generated_module_map",
    srcs = ["toolchain_system_headers.modulemap"],
    visibility = ["//visibility:public"],
)

filegroup(
    name = "archive_files",
    srcs = [
        "bin/llvm-ar.exe",
    ],
    visibility = ["//visibility:public"],
)

filegroup(
    name = "compile_files",
    srcs = [
        "bin/clang.exe",
    ] + glob(
        include = [
            "lib/clang/18/include/**",
        ],
        allow_empty = False,
    ),
    visibility = ["//visibility:public"],
)

filegroup(
    name = "link_files",
    srcs = [
        "bin/clang",
        "bin/ld.lld.exe",
        "bin/lld.exe",
    ],
    visibility = ["//visibility:public"],
)
""",
        executable = False,
    )

# https://bazel.build/rules/repository_rules
download_windows_toolchain = repository_rule(
    implementation = _download_windows_toolchain_impl,
    attrs = {},
    doc = "Downloads clang to build Skia with." +
          "Assumes you have msvc located on your device and have" +
          "MSVC in your $PATH.",
)
