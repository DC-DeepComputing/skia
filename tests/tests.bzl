"""
This file contains macros that generate multiple test targets, one per file.
"""

load("//bazel:macros.bzl", "skia_cc_test")
load("//bazel:cc_test_with_flags.bzl", "cc_test_with_flags")

# buildifier: disable=unnamed-macro
def skia_cpu_tests(tests, harness, resources = [], flags = {}):
    for filename in tests:
        cc_test_with_flags(
            name = filename[:-4],  # trim .cpp
            size = "small",
            srcs = [
                "BazelTestRunner.cpp",
                filename,
            ],
            deps = [
                harness,
                "//:skia_internal",
            ],
            data = resources,
            set_flags = flags,
        )

# buildifier: disable=unnamed-macro
def skia_ganesh_tests(tests, harness, resources = []):
    for filename in tests:
        skia_cc_test(
            name = filename[:-4],  # trim .cpp
            size = "small",
            srcs = [
                "BazelTestRunner.cpp",
            ] + select({
                "//src/gpu:has_gpu_backend": [filename],
                "//conditions:default": [],
            }),
            deps = [
                harness,
                "//:skia_internal",
            ],
            tags = [
                # We currently have no RBE machines with GPUs, so we cannot run these remotely.
                "no-remote",
            ],
            data = resources,
        )
