"""
This file contains logic related to Clang's -Wunsafe-buffer-usage warning, which flags pointer
arithmetic that cannot be bounds checked.

Skia compiles with -Weverything, which includes -Wunsafe-buffer-usage, so the warning is
suppressed wholesale in //include/config/copts.bzl while the codebase is being spanified.
To be able to turn the warning back on one directory at a time, Clang supports a "suppression
mappings" file which lists the paths the warning should (and should not) be suppressed for:
https://clang.llvm.org/docs/WarningSuppressionMappings.html

The driver flag which loads that file (--warning-suppression-mappings) was only added in Clang 20,
and Clang errors out on flags it does not recognize, so it can only be passed by toolchains new
enough to understand it. That is why this lives in the toolchain, where the compiler version is
known, instead of in copts.bzl, which applies to every toolchain (including the Emscripten one and
whatever toolchain an external client happens to be using).

Both halves are driven by a single knob, the "unsafe_buffer_usage" feature:

    bazel build --features=unsafe_buffer_usage //...

which:
  1. makes //include/config/copts.bzl emit -Wunsafe-buffer-usage instead of
     -Wno-unsafe-buffer-usage (see //bazel/common_config_settings:enforce_unsafe_buffer_usage), and
  2. makes the toolchain pass --warning-suppression-mappings, and (via the cc_toolchain's
     compiler_files) makes the mappings file an input of every compile action.

The copts half is the one that actually turns the warning on or off, because copts are appended
to the command line *after* all toolchain flags (and -Weverything, which re-enables everything,
is itself a copt).

TODO(b/532990050): Enable this feature by default once all of our hermetic toolchains are on
Clang 20 or newer.
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

# Must be kept in sync with //bazel/common_config_settings:enforce_unsafe_buffer_usage, which is
# what tells copts.bzl to stop suppressing the warning.
UNSAFE_BUFFER_USAGE_FEATURE = "unsafe_buffer_usage"

# The label of the mappings file. Toolchains take this as an attribute (rather than hard-coding
# the path) so that the path is correct both when Skia is the main repository and when it is
# being built as an external dependency.
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
    """Returns a list of features which point Clang at our -Wunsafe-buffer-usage suppressions.

    Args:
        mappings_file: Execroot-relative path of the suppression mappings file, that is
            ctx.file.unsafe_buffers_mappings.path. The same file must be listed in the
            compiler_files of the cc_toolchain using this feature, otherwise it will not be
            staged into the sandbox when the compile action runs.
        supported: Whether this toolchain's Clang understands --warning-suppression-mappings,
            which was added in Clang 20. Toolchains which are older still declare the feature
            (so that --features=unsafe_buffer_usage is not silently misspelled) but pass no
            flags, which means the warning will fire everywhere instead of only in the
            directories that have been spanified. In other words, do not ask for this feature
            until your toolchain is new enough.
        actions: The compile actions the flag should be added to. Toolchains which build
            Objective-C should pass COMPILE_ACTIONS + OBJC_COMPILE_ACTIONS.

    Returns:
        A list containing one Bazel "feature" called "unsafe_buffer_usage".
    """
    flag_sets = []
    if supported:
        flag_sets.append(flag_set(
            actions = actions,
            flag_groups = [
                flag_group(
                    flags = [
                        "--warning-suppression-mappings=" + mappings_file,
                    ],
                ),
            ],
        ))

    return [
        feature(
            name = UNSAFE_BUFFER_USAGE_FEATURE,
            # This is off by default until the codebase (and our toolchains) are ready; see the
            # TODO at the top of this file.
            enabled = False,
            flag_sets = flag_sets,
        ),
    ]
