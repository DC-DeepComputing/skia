"""
This file is auto-generated from //bazel/deps_parser.
DO NOT MODIFY BY HAND.
"""

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

def git_repos_from_deps():
    git_repository(
        name = "dawn",
        build_file = "bazel/external/dawn/BUILD.bazel",
        commit = "b4d8c8012bb5ea18b966b783b784878d84404e11",
        remote = "https://dawn.googlesource.com/dawn.git",
    )
