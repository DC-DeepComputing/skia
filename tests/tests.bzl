"""
This file contains macros that generate multiple test targets, one per file.
"""

load("//bazel:macros.bzl", "skia_cc_test")
load("//bazel:cc_test_with_flags.bzl", "cc_test_with_flags")

# buildifier: disable=unused-variable
def skia_cpu_tests(name, tests, harness, resources = [], flags = {}):
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

# buildifier: disable=unused-variable
def skia_ganesh_tests(name, tests, harness, resources = [], extra_deps = []):
    for filename in tests:
        skia_cc_test(
            name = filename[:-4],  # trim .cpp
            size = "small",
            srcs = [
                "BazelTestRunner.cpp",
            ] + select({
                # Make this a no-op test if not compiling with GPU backend.
                "//src/gpu:has_gpu_backend": [filename],
                "//conditions:default": [],
            }),
            deps = [
                harness,
                "//:skia_internal",
            ] + select({
                # extra_deps might only compile with GPU backend code, so conditionally add it
                # just like we do the actual file
                "//src/gpu:has_gpu_backend": extra_deps,
                "//conditions:default": [],
            }),
            tags = [
                # We currently have no RBE machines with GPUs, so we cannot run these remotely.
                "no-remote",
            ],
            data = resources,
        )
