"""
This file assembles a toolchain for a Mac M1 host using the Clang Compiler and glibc.

It downloads the necessary headers, executables, and pre-compiled static/shared libraries to
the external subfolder of the Bazel cache (the same place third party deps are downloaded with
http_archive or similar functions in WORKSPACE.bazel). These will be able to be used via our
custom c++ toolchain configuration (see //toolchain/mac_M1_toolchain_config.bzl)

Most files are downloaded as [files from where?] (with us acting as the dependency
resolver) and extracted to
  [outputRoot (aka Bazel cache)]/[outputUserRoot]/[outputBase]/external/clang_mac_M1
  (See https://bazel.build/docs/output_directories#layout-diagram)
which will act as our sysroot.
"""
# From https://github.com/llvm/llvm-project/releases/download/llvmorg-13.0.0/clang+llvm-13.0.0-x86_64-apple-darwin.tar.xz
clang_prefix = "clang+llvm-13.0.0-x86_64-apple-darwin"
clang_sha256 = "d051234eca1db1f5e4bc08c64937c879c7098900f7a0370f3ceb7544816a8b09"
clang_url = "https://github.com/llvm/llvm-project/releases/download/llvmorg-13.0.0/clang+llvm-13.0.0-x86_64-apple-darwin.tar.xz"

def _download_mac_m1_toolchain_impl(ctx):
    # Download the clang toolchain (the extraction can take a while)
    # https://bazel.build/rules/lib/repository_ctx#download_and_extract
    ctx.download_and_extract(
        url = clang_url,
        output = "",
        stripPrefix = clang_prefix,
        sha256 = clang_sha256,
    )
    
    # Create a BUILD.bazel file that makes all the files in this subfolder
    # available for use in rules, i.e. in the toolchain declaration.
    # https://bazel.build/rules/lib/repository_ctx#file
    ctx.file(
        "BUILD.bazel",
        content = """
filegroup(
    name = "all_files",
    srcs = glob([
        "**",
    ]),
    visibility = ["//visibility:public"]
)
""",
        executable = False,
    )

# https://bazel.build/rules/repository_rules
download_mac_m1_toolchain = repository_rule(
    implementation = _download_mac_m1_toolchain_impl,
    attrs = {},
    doc = "Downloads clang, and all supporting headers, executables, " +
          "and shared libraries required to build Skia on a Mac M1 host",
)
