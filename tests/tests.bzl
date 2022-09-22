"""
This file contains macros that generate multiple test targets, one per file.
"""

load("//bazel:macros.bzl", "skia_cc_test")

# buildifier: disable=unnamed-macro
def skia_cpu_tests(tests, harness):
    for filename in tests:
        skia_cc_test(
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
        )

# buildifier: disable=unnamed-macro
def skia_ganesh_tests(tests, harness):
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
        )
