"""
This file contains logic related to Clang's -Wunsafe-buffer-usage warning, which flags pointer
arithmetic that cannot be bounds checked.

Skia compiles with -Weverything, so this warning is on by default and has to be turned off while
the codebase is being spanified. Rather than turning it off everywhere, Clang can be given a
"suppression mappings" file which lists the paths the warning should be suppressed for, letting us
enforce it one directory at a time as they are spanified:
https://clang.llvm.org/docs/WarningSuppressionMappings.html

The driver flag which loads that file (--warning-suppression-mappings) was added in Clang 20, and
Clang errors out on flags it does not recognize. Skia's hermetic toolchains each pin a specific
Clang, so whether that Clang supports the flag is known statically; every toolchain config passes
its pinned Clang major version to make_unsafe_buffers_features(), which emits:

    Clang >= 20:  -Wunsafe-buffer-usage --warning-suppression-mappings=<file>
    Clang <  20:  -Wno-unsafe-buffer-usage

When a toolchain's Clang is upgraded, its version constant must be updated too, which is what turns
enforcement on for that toolchain.

Toolchains we do not control (Emscripten, the NDK, or whatever an external client is using) never
see either flag from here; //include/config/copts.bzl keeps suppressing the warning for them,
keyed off the //bazel/platform:use_hermetic_toolchain constraint.

Two ordering rules make this work:
  1. Clang applies explicit -W/-Wno- flags in the order they appear, and the last one wins. That
     is why the "suppress" feature below is returned first and the "enforce" feature second.
  2. -Weverything does NOT re-enable a warning that was explicitly turned off, so the
     -Wno-unsafe-buffer-usage emitted here survives the -Weverything which copts add later.

TODO(b/532990050): Once every hermetic toolchain is on Clang 20+, the suppress feature and the
copts select can both go away.
"""

# https://github.com/bazelbuild/bazel/blob/master/tools/build_defs/cc/action_names.bzl
load("@bazel_tools//tools/build_defs/cc:action_names.bzl", "ACTION_NAMES")

# https://github.com/bazelbuild/bazel/blob/master/tools/cpp/cc_toolchain_config_lib.bzl
load(
    "@bazel_tools//tools/cpp:cc_toolchain_config_lib.bzl",
    "feature",
    "flag_group",
    "flag_set",
)

# The label of the mappings file. Toolchains take this as an attribute (rather than hard-coding the
# path) so that the path is correct both when Skia is the main repository and when it is being
# built as an external dependency.
UNSAFE_BUFFERS_MAPPINGS_LABEL = "//tools/unsafe_buffers:unsafe_buffers_mappings.txt"

# The first Clang release which accepts --warning-suppression-mappings.
MIN_CLANG_VERSION_FOR_SUPPRESSION_MAPPINGS = 20

COMPILE_ACTIONS = [
    ACTION_NAMES.c_compile,
    ACTION_NAMES.cpp_compile,
]

OBJC_COMPILE_ACTIONS = [
    ACTION_NAMES.objc_compile,
    ACTION_NAMES.objcpp_compile,
]

def supports_warning_suppression_mappings(clang_major_version):
    """Returns whether a Clang of the given major version accepts --warning-suppression-mappings.

    Args:
        clang_major_version: The major version of the toolchain's pinned Clang, as an int.
    """
    return clang_major_version >= MIN_CLANG_VERSION_FOR_SUPPRESSION_MAPPINGS

def make_unsafe_buffers_features(clang_major_version, mappings_file = None, actions = COMPILE_ACTIONS):
    """Returns the features which control Clang's -Wunsafe-buffer-usage warning.

    Args:
        clang_major_version: The major version of the toolchain's pinned Clang, as an int.
        mappings_file: Execroot-relative path of the suppression mappings file, that is
            ctx.file.unsafe_buffers_mappings.path. Required if the Clang supports suppression
            mappings. The same file must be listed in the compiler_files of the cc_toolchain using
            these features, otherwise it will not be staged into the sandbox when the compile
            action runs.
        actions: The compile actions these flags should be added to. Toolchains which build
            Objective-C should pass COMPILE_ACTIONS + OBJC_COMPILE_ACTIONS.

    Returns:
        A list of Bazel "features". The "suppress_unsafe_buffer_usage" feature is always present;
        "unsafe_buffer_usage" is added on top of it when the Clang supports the mappings file.
        Both are on by default; disabling the latter (--features=-unsafe_buffer_usage) falls back
        to suppressing the warning, which is a safe way to work around a bad mappings file.
    """

    def _flags(flags):
        return [flag_set(
            actions = actions,
            flag_groups = [flag_group(flags = flags)],
        )]

    features = [
        # Declared first so that anything below can override it. See ordering rule 1 above.
        feature(
            name = "suppress_unsafe_buffer_usage",
            enabled = True,
            flag_sets = _flags(["-Wno-unsafe-buffer-usage"]),
        ),
    ]

    if supports_warning_suppression_mappings(clang_major_version):
        if not mappings_file:
            fail("Clang %d supports --warning-suppression-mappings, so a mappings_file is required" %
                 clang_major_version)
        features.append(feature(
            name = "unsafe_buffer_usage",
            enabled = True,
            flag_sets = _flags([
                "-Wunsafe-buffer-usage",
                "--warning-suppression-mappings=" + mappings_file,
            ]),
        ))

    return features
