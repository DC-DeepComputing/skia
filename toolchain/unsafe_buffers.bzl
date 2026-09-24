"""
This file contains logic related to Clang's -Wunsafe-buffer-usage warning, which flags pointer
arithmetic that cannot be bounds checked.

Skia compiles with -Weverything, so this warning is on by default and has to be turned off while
the codebase is being spanified. Rather than turning it off everywhere, Clang can be given a
"suppression mappings" file which lists the paths the warning should be suppressed for, letting us
enforce it one directory at a time as they are spanified:
https://clang.llvm.org/docs/WarningSuppressionMappings.html

The driver flag which loads that file (--warning-suppression-mappings) is recent, and Clang errors
out on flags it does not recognize, so we cannot pass it unconditionally. There is no manual switch
for this and no version number written down anywhere: when a toolchain repository is fetched, it
runs the Clang it just downloaded and asks it whether it accepts the flag. Whatever the answer is,
the toolchain acts on it:

    accepted:  -Wunsafe-buffer-usage --warning-suppression-mappings=<file>
    rejected:  -Wno-unsafe-buffer-usage

Enforcement therefore turns itself on the moment a toolchain is upgraded to a Clang that supports
the flag, with no other change required.

Toolchains we do not control (Emscripten, the NDK, or whatever an external client is using) never
see either flag from here; //include/config/copts.bzl keeps suppressing the warning for them,
keyed off the //bazel/platform:use_hermetic_toolchain constraint.

Two ordering rules make this work, both of which have been verified against the real build:
  1. Clang applies explicit -W/-Wno- flags in the order they appear, and the last one wins. That
     is why the "suppress" feature below is returned first and the "enforce" feature second.
  2. -Weverything does NOT re-enable a warning that was explicitly turned off, so the
     -Wno-unsafe-buffer-usage emitted here survives the -Weverything which copts add later.

TODO(b/532990050): Once every toolchain accepts the flag, the suppress feature and the copts
select can both go away.
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

# The name of the config_setting that unsafe_buffers_build_file_content() adds to a downloaded
# toolchain repository. //toolchain/BUILD.bazel select()s on it to tell each toolchain config
# whether its compiler passed the probe.
SUPPORTS_SUPPRESSION_MAPPINGS_TARGET = "supports_warning_suppression_mappings"

COMPILE_ACTIONS = [
    ACTION_NAMES.c_compile,
    ACTION_NAMES.cpp_compile,
]

OBJC_COMPILE_ACTIONS = [
    ACTION_NAMES.objc_compile,
    ACTION_NAMES.objcpp_compile,
]

# Scratch directory used by probe_warning_suppression_mappings. It is deleted again before the
# repository rule returns, so it never shows up in a glob or in an action's inputs.
_PROBE_DIR = "unsafe_buffers_probe"

def probe_warning_suppression_mappings(ctx, clang):
    """Runs a compiler to find out whether it accepts --warning-suppression-mappings.

    This asks the exact binary that will build Skia, rather than inferring an answer from a version
    number or from whatever compiler happens to be on the host's PATH. It stays hermetic because
    the compiler being run was just downloaded by hash into the repository being fetched.

    Args:
        ctx: The repository_ctx of the toolchain repository, after the compiler has been extracted.
        clang: Path of the compiler to run, e.g. ctx.path("bin/clang").

    Returns:
        True if the compiler accepted the flags Skia would pass it.
    """
    source = _PROBE_DIR + "/probe.cpp"
    mappings = _PROBE_DIR + "/probe_mappings.txt"
    object_file = _PROBE_DIR + "/probe.o"

    # An empty translation unit is enough: we are testing the driver, not the compiler, and this
    # way the probe needs no headers and therefore no sysroot.
    ctx.file(source, "", executable = False)
    ctx.file(mappings, "[unsafe-buffer-usage]\n", executable = False)

    # https://bazel.build/rules/lib/builtins/repository_ctx#execute
    result = ctx.execute([
        clang,
        "-c",
        source,
        "-o",
        object_file,
        "-Wunsafe-buffer-usage",
        "--warning-suppression-mappings=" + mappings,
        # An unrecognized driver flag is a hard error for clang but only a warning for clang-cl,
        # which would otherwise ignore the flag and let the probe pass. -Werror=unknown-argument
        # makes both of them fail.
        "-Werror",
        "-Werror=unknown-argument",
    ])

    ctx.delete(_PROBE_DIR)

    return result.return_code == 0

def unsafe_buffers_build_file_content(supported):
    """Returns BUILD file text publishing the result of probe_warning_suppression_mappings.

    Toolchain repositories append this to the BUILD file they generate. The result has to reach
    //toolchain/BUILD.bazel through a select() rather than a load(): //toolchain is loaded on every
    Bazel invocation (.bazelrc registers its toolchains), and a load() would force every toolchain
    repository - including the ones for other operating systems - to be downloaded every time.

    Args:
        supported: The value returned by probe_warning_suppression_mappings.

    Returns:
        A string to concatenate onto the repository's BUILD file content.
    """
    return """
load("@bazel_skylib//rules:common_settings.bzl", "bool_setting")

# Whether the compiler in this repository accepted --warning-suppression-mappings when it was run
# at fetch time. See //toolchain/unsafe_buffers.bzl.
bool_setting(
    name = "warning_suppression_mappings",
    build_setting_default = {supported},
    visibility = ["//visibility:public"],
)

config_setting(
    name = "{target}",
    flag_values = {{":warning_suppression_mappings": "True"}},
    visibility = ["//visibility:public"],
)
""".format(
        supported = supported,
        target = SUPPORTS_SUPPRESSION_MAPPINGS_TARGET,
    )

def make_unsafe_buffers_features(mappings_file, supported, actions = COMPILE_ACTIONS):
    """Returns the features which control Clang's -Wunsafe-buffer-usage warning.

    Args:
        mappings_file: Execroot-relative path of the suppression mappings file, that is
            ctx.file.unsafe_buffers_mappings.path. The same file must be listed in the
            compiler_files of the cc_toolchain using these features, otherwise it will not be
            staged into the sandbox when the compile action runs.
        supported: Whether this toolchain's compiler understands --warning-suppression-mappings,
            as probed when its repository was fetched.
        actions: The compile actions these flags should be added to. Toolchains which build
            Objective-C should pass COMPILE_ACTIONS + OBJC_COMPILE_ACTIONS.

    Returns:
        A list of Bazel "features". The "suppress_unsafe_buffer_usage" feature is always present;
        "unsafe_buffer_usage" is added on top of it when the compiler supports the mappings file.
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
