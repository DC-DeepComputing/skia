"""
This file contains a way to set flags from BUILD.bazel instead of requiring users to set them from
the CLI.

It is based off of https://github.com/bazelbuild/examples/tree/main/rules/starlark_configurations/cc_binary_selectable_copts

"""

load("//bazel:cc_binary_with_flags.bzl", "with_flags_transition")

def _transition_rule_impl(ctx):
    target = ctx.attr.target[0]
    return [
        # https://bazel.build/rules/lib/CcInfo
        target[CcInfo],
    ]

cc_library_with_flags = rule(
    implementation = _transition_rule_impl,
    attrs = {
        # set_flags is a dictionary with the keys being the short-form of a flag name
        # (e.g. the part that comes after the colon) and the value being a list of values
        # that the flag should be set to, regardless of the relevant CLI flags.
        # https://bazel.build/rules/lib/attr#string_list_dict
        "set_flags": attr.string_list_dict(),
        "target": attr.label(mandatory = True, allow_single_file = True, cfg = with_flags_transition),
        # This is a stock Bazel requirement for any rule that uses Starlark
        # transitions. It's okay to copy the below verbatim for all such rules.
        "_allowlist_function_transition": attr.label(
            default = "@bazel_tools//tools/allowlists/function_transition_allowlist",
        ),
    },
)
