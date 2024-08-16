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


def _get_system_paths(ctx):
    ctx.file(
        "find_msvc.py",
        content="""
import os
import subprocess
import sys


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
    return os.path.join(vctools_prefix, subdirs[0].rstrip())


def get_windows_sdk_path():
    # These environment variables are set when running in a Developer Console.
    # eg. "C:\\Program Files (x86)\\Windows Kits\\10\\"
    win_sdk_dir = os.getenv('WindowsSdkDir')
    # eg. "10.0.22621.0\\"
    win_sdk_version = os.getenv('WindowsSDKVersion')
    if win_sdk_dir and win_sdk_version:
      return os.path.join(win_sdk_dir, 'Include', win_sdk_version.rstrip('\\\\'))

    # Fallback to searching "Program Files(x86)".
    program_files_x86 = os.getenv('ProgramFiles(x86)')
    if not program_files_x86:
        raise Exception('Unable to find Program Files (x86)')
    include_dir = os.path.join(program_files_x86, 'Windows Kits', '10', 'Include')

    # There's a version-numbered directory under Include. Find the latest version
    # and use that.
    subdirs = os.listdir(include_dir)
    subdirs.sort(reverse=True)
    return os.path.join(include_dir, subdirs[0].rstrip())


if __name__ == '__main__':
    if len(sys.argv) != 2:
        raise Exception('Usage: %s <msvc|sdk>' % sys.argv[0])
    if sys.argv[1] == 'msvc':
        print(get_system_msvc_path())
    elif sys.argv[1] == 'sdk':
        print(get_windows_sdk_path())
""",
        executable=False,
    )

    res = ctx.execute(["python.exe", ctx.path("find_msvc.py"), 'msvc'])
    if res.return_code != 0:
        fail("Failed to find MSVC path: " + res.stderr)
    msvc_path = res.stdout.rstrip()

    res = ctx.execute(["python.exe", ctx.path("find_msvc.py"), 'sdk'])
    if res.return_code != 0:
        fail("Failed to find Windows SDK path: " + res.stderr)
    win_sdk_path = res.stdout.rstrip()
    return (msvc_path, win_sdk_path)


def _delete_msvc_symlinks(ctx):
    ctx.delete(".\\symlinks\\msvc\\include")
    ctx.delete(".\\symlinks\\msvc\\lib")
    ctx.delete(".\\symlinks\\win-sdk\\include")


def _create_msvc_symlinks(ctx):
    system_msvc_path, win_sdk_path = _get_system_paths(ctx)

    # https://bazel.build/rules/lib/actions#symlink
    ctx.symlink(
        # from =
        system_msvc_path + "\\include",
        # to =
        ".\\symlinks\\msvc\\include",
    )
    ctx.symlink(
        # from =
        win_sdk_path,
        # to =
        ".\\symlinks\\win-sdk\\include",
    )

    # It is very important to symlink the frameworks directory to [sysroot]/System/Library/Frameworks
    # because some Frameworks "re-export" other frameworks. These framework paths are relative to
    # the sysroot (which on a typical machine is /), and it is difficult to change these paths.
    # By making the symlinks emulate the original path structure, we can keep those re-exports
    # from breaking.
    ctx.symlink(
        # from =
        system_msvc_path + "\\lib",
        # to =
        ".\\symlinks\\msvc\\lib",
    )


def _download_windows_toolchain_impl(ctx):
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
    # builtin_include_directories = [
    #     "external/clang_windows/symlinks/msvc/include",
    #     "external/clang_windows/symlinks/win-sdk/include/ucrt",
    #     "external/clang_windows/symlinks/win-sdk/include/um",
    #     "external/clang_windows/symlinks/win-sdk/include/shared",
    #     "external/clang_windows/symlinks/win-sdk/include/winrt",
    #     "external/clang_windows/symlinks/win-sdk/include/cppwinrt",
    #     "external/clang_windows/include/clang",
    #     "external/clang_windows/include/clang-c",
    #     "external/clang_windows/include/clang-tidy",
    #     "external/clang_windows/include/lld",
    #     "external/clang_windows/include/lldb",
    #     "external/clang_windows/include/llvm",
    #     "external/clang_windows/include/llvm-c",
    #     "external/clang_windows/lib/clang/18/include",
    # ]

    # generate_system_module_map(
    #     ctx,
    #     module_file = "toolchain_system_headers.modulemap",
    #     folders = builtin_include_directories,
    # )

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
