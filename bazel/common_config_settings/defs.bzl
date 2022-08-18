"""
THIS IS THE EXTERNAL-ONLY VERSION OF THIS FILE. G3 HAS ITS OWN.

This file contains helpers for defining build flags and options that are used to
configure the Skia build.
"""

# https://github.com/bazelbuild/bazel-skylib/blob/main/rules/common_settings.bzl
load("@bazel_skylib//rules:common_settings.bzl", "string_flag", skylib_bool_flag = "bool_flag")

# Forked from https://github.com/bazelbuild/bazel-skylib/blob/main/rules/common_settings.bzl
BuildSettingInfo = provider(
    doc = "A singleton provider that contains the raw value of a multi-string build setting",
    fields = ["values"],
)

USE_DEFAULTS = "use_defaults"

def _multi_string_impl(ctx):
    allowed_values = ctx.attr.values
    if ctx.build_setting_value == USE_DEFAULTS:
        values = ctx.default_values
    else:
        values = ctx.build_setting_value

    for v in values:
        if v not in ctx.attr.values:
            fail("Error setting " + str(ctx.label) + ": invalid value '" + v + "'. Allowed values are " + str(allowed_values))
    return BuildSettingInfo(values = values)

multi_string_flag = rule(
    implementation = _multi_string_impl,
    build_setting = config.string(flag = True, allow_multiple = True),
    attrs = {
        "values": attr.string_list(
            doc = "The list of allowed values for this setting. An error is raised if any other values are given.",
        ),
        "default_values": attr.string_list(
            doc = "The default values to be used if this flag was not set from the CLI",
        ),
    },
    doc = "A string-typed build setting that can be set multiple times on the command line",
)

def string_flag_with_values(
        name,
        values,
        external_default = None,
        internal_default = None,
        multiple = False):
    """Create a string flag and corresponding config_settings.

    string_flag_with_values is a Bazel Macro that defines a flag with the given name and a set
    of valid values for that flag. For each value, a config_setting is defined with the name
    of the value, associated with the created flag.
    This is defined to make the BUILD.bazel file easier to read w/o the boilerplate of defining
    a string_flag rule and n config_settings
    https://docs.bazel.build/versions/main/skylark/macros.html

    Args:
        name: string, the name of the flag to create and use for the config_settings
        values: list of strings, the valid values for this flag to be set to.
        external_default: string, list of strings, or None: The default if this flag is not set
                          from the CLI.
        internal_default: string, list of strings, or None: Only used for internal builds.
                          The default if this flag is not set from the CLI.
        multiple: boolean, True if the flag should be able to be set multiple times on the CLI.
    """

    if multiple:
        ALLOWED_TYPES = ["NoneType", "list"]
        if type(external_default) not in ALLOWED_TYPES or type(internal_default) not in ALLOWED_TYPES:
            fail("defaults for multistring flag must be a list or None")

        #values.append(USE_DEFAULTS)
        multi_string_flag(
            name = name,
            build_setting_default = USE_DEFAULTS,  # sentinel value
            default_values = external_default,
            values = values,
        )
    else:
        ALLOWED_TYPES = ["NoneType", "string"]
        if type(external_default) not in ALLOWED_TYPES or type(internal_default) not in ALLOWED_TYPES:
            fail("defaults for string flag must be a string or None")

        if not external_default:
            # We have to specify a default value, even if that value is empty string.
            # https://docs.bazel.build/versions/main/skylark/config.html#instantiating-build-settings
            external_default = ""

            # Empty string means "none of the options", which we need to add to the list of
            # valid values.
            values.append("")

        string_flag(
            name = name,
            build_setting_default = external_default,
            values = values,
        )

    # For each of the values given, we define a config_setting. This allows us to use
    # select statements, on the given setting, e.g. referencing
    # //bazel/common_config_settings:some_valid_value_for_a_flag
    for v in values:
        if not v:  # skip making a target for empty string, aka "none selected"
            continue
        native.config_setting(
            name = v,
            flag_values = {
                ":" + name: v,
            },
            visibility = ["//:__subpackages__"],
        )

def bool_flag(name, external_default = None, internal_default = None, public = True):
    """Create a boolean flag and corresponding config_settings.

    bool_flag is a Bazel Macro that defines a boolean flag with the given name two config_settings,
    one for True, one for False. Reminder that Bazel has special syntax for unsetting boolean flags,
    but this does not work well with aliases.
    https://docs.bazel.build/versions/main/skylark/config.html#using-build-settings-on-the-command-line
    Thus it is best to define both an "enabled" alias and a "disabled" alias.

    Args:
        name: string, the name of the flag to create and use for the config_settings
        external_default: boolean: The default if this flag is not set from the CLI.
        internal_default: boolean: Only used for internal builds.
                          The default if this flag is not set from the CLI.
        public: boolean, if the flag should be usable from other packages or if it is meant to be
            combined with some other constraint.
    """
    if type(external_default) == "NoneType" or type(internal_default) == "NoneType":
        fail("Must set external_default and internal_default for bool_flag")

    skylib_bool_flag(name = name, build_setting_default = external_default)
    vis = ["//:__subpackages__"]
    if not public:
        vis = ["//visibility:private"]

    native.config_setting(
        name = name + "_true",
        flag_values = {
            # The value must be a string, but it will be parsed to a boolean
            # https://docs.bazel.build/versions/main/skylark/config.html#build-settings-and-select
            ":" + name: "True",
        },
        visibility = vis,
    )

    native.config_setting(
        name = name + "_false",
        flag_values = {
            ":" + name: "False",
        },
        visibility = vis,
    )
