"""
This file contains flags for the C++ compiler, referred to by Bazel as copts.

The copts in a cc_library to not flow to the children (dependencies) nor the parents
(dependents), so we cannot define them along side the defines in //bazel/BUILD.bazel.

We cannot (easily) define them in the C++ toolchain configuration
(e.g. //toolchain/linux_amd64_toolchain_config.bzl), because that does not support listening
to arbitrary Bazel flags (e.g. those defined in //bazel/common_config_settings). If we wanted
to implement these flags in the toolchain, we would need to group them into features [1],
but we don't control the features implemented by the G3 toolchain. Because we want to
automatically roll into G3 with minimal changes, the copts cannot go in the toolchain,

Thus, they go here, so we can use select statements to conditionally control them and
override what they do (if necessary) in G3.

They are divided into several lists/selects and were initially created to be identical to the
 GN ones [2][3]

[1] https://bazel.build/docs/cc-toolchain-config-reference#features
[2] https://github.com/google/skia/blob/2b07cdb07e88f2870260eabac708f31bc7977d08/gn/BUILDCONFIG.gn#L177-L181
[3] https://github.com/google/skia/blob/2b07cdb07e88f2870260eabac708f31bc7977d08/gn/skia/BUILD.gn#L593-L630
"""

CORE_COPTS = [
    "-fstrict-aliasing",
    "-fPIC",
    "-fvisibility=hidden",
    "-fno-exceptions",
    "-fno-rtti",  # Reduces code size
]

OPT_LEVEL = select({
    "//bazel/common_config_settings:debug_build": [
        "--optimize=0",
        "--debug",
    ],
    "//bazel/common_config_settings:release_build": [
        "--optimize=3",
        # Strip dead code (in conjunction with linkopts)
        "-fdata-sections",
        "-ffunction-sections",
    ],
    "//bazel/common_config_settings:fast_build": [
        "--optimize=0",
        "-gline-tables-only",
    ],
})

WARNINGS = [
    "-Wno-psabi",  # noisy
]

DEFAULT_COPTS = CORE_COPTS + OPT_LEVEL + WARNINGS
