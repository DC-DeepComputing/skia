"""
This file is auto-generated from //bazel/deps_parser.
DO NOT MODIFY BY HAND.
"""

load("@bazel_tools//tools/build_defs/repo:git.bzl", "new_git_repository")

def git_repos_from_deps():
    # https://bazel.build/rules/lib/repo/git
    new_git_repository(
        name = "dawn",
        build_file = "@//bazel/external/dawn:BUILD.bazel",
        commit = "0cdf25f990d0ec2adb7ab0f164b62e4c417a851a",
        remote = "https://dawn.googlesource.com/dawn.git",
    )

    new_git_repository(
        name = "dng_sdk",
        build_file = "@//bazel/external/dng_sdk:BUILD.bazel",
        commit = "c8d0c9b1d16bfda56f15165d39e0ffa360a11123",
        remote = "https://android.googlesource.com/platform/external/dng_sdk.git",
    )

    new_git_repository(
        name = "expat",
        build_file = "@//bazel/external/expat:BUILD.bazel",
        commit = "a28238bdeebc087071777001245df1876a11f5ee",
        remote = "https://chromium.googlesource.com/external/github.com/libexpat/libexpat.git",
    )
