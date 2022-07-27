"""
This file contains flags for the C++ linker, referred to by Bazel as linkopts.

For similar reasons as //bazel/copts.bzl, these are defined in this file as opposed
to alternatives.

"""

OPT_LEVEL = select({
    "//bazel/common_config_settings:debug_build": [],
    "//bazel/common_config_settings:release_build": [
        "-dead_strip",
        "-Wl,--gc-sections",
        "-Wl,--strip-all",
    ],
    "//bazel/common_config_settings:fast_build": [
        "-Wl,--strip-debug",
    ],
})

DEFAULT_LINKOPTS = OPT_LEVEL
