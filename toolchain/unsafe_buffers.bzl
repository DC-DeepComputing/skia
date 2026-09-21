"""
This file contains logic related to Clang's -Wunsafe-buffer-usage warning, which flags pointer
arithmetic that cannot be bounds checked.

Skia compiles with -Weverything, so this warning is on by default and has to be turned off while
the codebase is being spanified. Rather than turning it off everywhere, Clang can be given a
"suppression mappings" file which lists the paths the warning should be suppressed for, letting us
enforce it one directory at a time as they are spanified:
https://clang.llvm.org/docs/WarningSuppressionMappings.html

The driver flag which loads that file (--warning-suppression-mappings) was only added in Clang 20,
and Clang errors out on flags it does not recognize. There is no manual switch for this: each
toolchain knows which Clang it downloads, so it decides for itself which of the two behaviors to
apply, and enforcement turns on automatically as toolchains are upgraded.

    Clang >= 20:  -Wunsafe-buffer-usage --warning-suppression-mappings=<file>
    Clang <  20:  -Wno-unsafe-buffer-usage

Toolchains we do not control (Emscripten, the NDK, or whatever an external client is using) never
see either flag from here; //include/config/copts.bzl keeps suppressing the warning for them,
keyed off the //bazel/platform:use_hermetic_toolchain constraint.

Two ordering rules make this work, both of which have been verified against the real build:
  1. Clang applies explicit -W/-Wno- flags in the order they appear, and the last one wins. That
     is why the "suppress" feature below is returned first and the "enforce" feature second.
  2. -Weverything does NOT re-enable a warning that was explicitly turned off, so the
     -Wno-unsafe-buffer-usage emitted here survives the -Weverything which copts add later.

TODO(b/532990050): Once every toolchain is on Clang 20+, the suppress feature and the copts
select can both go away.
"""

# https://github.com/bazelbuild/bazel/blob/master/tools/build_defs/cc/action_names.bzl
load("@bazel_tools//tools/build_defs/cc:action_names.bzl", "ACTION_NAMES")

# https://github.com/bazelbuild/rules_cc/blob/main/cc/cc_toolchain_config_lib.bzl
load(
    "@rules_cc//cc:cc_toolchain_config_lib.bzl",
    "feature",
    "flag_group",
    "flag_set",
)

# The Clang release which added --warning-suppression-mappings. Toolchains compare the version of
# the Clang they download against this to decide whether they can enforce the warning.
MIN_CLANG_VERSION_FOR_SUPPRESSION_MAPPINGS = 20

# The label of the mappings file. Toolchains take this as an attribute (rather than hard-coding the
# path) so that the path is correct both when Skia is the main repository and when it is being
# built as an external dependency.
UNSAFE_BUFFERS_MAPPINGS_LABEL = "//tools/unsafe_buffers:unsafe_buffers_mappings.txt"

COMPILE_ACTIONS = [
    ACTION_NAMES.c_compile,
    ACTION_NAMES.cpp_compile,
]

OBJC_COMPILE_ACTIONS = [
    ACTION_NAMES.objc_compile,
    ACTION_NAMES.objcpp_compile,
]

def make_unsafe_buffers_features(mappings_file, supported, actions = COMPILE_ACTIONS):
    """Returns the features which control Clang's -Wunsafe-buffer-usage warning.

    Args:
        mappings_file: Execroot-relative path of the suppression mappings file, that is
            ctx.file.unsafe_buffers_mappings.path. The same file must be listed in the
            compiler_files of the cc_toolchain using these features, otherwise it will not be
            staged into the sandbox when the compile action runs.
        supported: Whether this toolchain's Clang understands --warning-suppression-mappings.
            Toolchains should compute this by comparing the version of the Clang they download
            against MIN_CLANG_VERSION_FOR_SUPPRESSION_MAPPINGS, so that upgrading the toolchain
            turns the warning on without any other change.
        actions: The compile actions these flags should be added to. Toolchains which build
            Objective-C should pass COMPILE_ACTIONS + OBJC_COMPILE_ACTIONS.

    Returns:
        A list of Bazel "features". The "suppress_unsafe_buffer_usage" feature is always present;
        "unsafe_buffer_usage" is added on top of it when the toolchain is new enough. Both are on
        by default; disabling the latter (--features=-unsafe_buffer_usage) falls back to
        suppressing the warning, which is a safe way to work around a bad mappings file.
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

    if supported:
        features.append(feature(
            name = "unsafe_buffer_usage",
            enabled = True,
            flag_sets = _flags([
                "-Wunsafe-buffer-usage",
                "--warning-suppression-mappings=" + mappings_file,
            ]),
        ))

    return features
