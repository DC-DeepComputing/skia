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
            srcs = select({
                # Make this a no-op test if compiling with a GPU backend.
                "//src/gpu:has_gpu_backend": ["BazelNoopRunner.cpp"],
                "//conditions:default": [
                    "BazelTestRunner.cpp",
                    filename,
                ],
            }),
            deps = select({
                "//src/gpu:has_gpu_backend": [],
                # Only build and apply deps if we have a no-op test.
                "//conditions:default": [
                    harness,
                    "//:skia_internal",
                ],
            }),
            data = resources,
            set_flags = flags,
        )

# buildifier: disable=unused-variable
def skia_ganesh_tests(name, tests, harness, resources = [], extra_deps = []):
    for filename in tests:
        skia_cc_test(
            name = filename[:-4],  # trim .cpp
            size = "small",
            srcs = select({
                "//src/gpu:has_gpu_backend": [
                    "BazelTestRunner.cpp",
                    filename,
                ],
                # Make this a no-op test if not compiling with a GPU backend.
                "//conditions:default": ["BazelNoopRunner.cpp"],
            }),
            deps = select({
                # Only build and apply deps if we have a no-op test.
                "//src/gpu:has_gpu_backend": [
                    harness,
                    "//:skia_internal",
                ] + extra_deps,
                "//conditions:default": [],
            }),
            tags = [
                # We currently have no RBE machines with GPUs, so we cannot run these remotely.
                "no-remote",
            ],
            data = resources,
        )
