# https://github.com/bazelbuild/rules_webtesting/blob/master/web/web.bzl
load("@io_bazel_rules_webtesting//web:web.bzl", "web_test_suite")
load("@build_bazel_rules_nodejs//:providers.bzl", "ExternalNpmPackageInfo", "node_modules_aspect")

def karma_test(name, srcs, config_file, **kwargs):
    """Runs the given tests on Chromium downloaded via Bazel using Karma

    Args:
      srcs: A list of JavaScript test files or helpers.
      config_file: A karma config file. The following template strings must be used...
    """
    if len(srcs) == 0:
        fail("Must pass at least one file into srcs or there will be no tests to run")

    wrapped_test_name = name + "_karma_test"
    _karma_test(
        name = wrapped_test_name,
        srcs = srcs,
        deps = [
            "@npm//karma-chrome-launcher",
            "@npm//karma-firefox-launcher",
            "@npm//karma-jasmine",
            "@npm//jasmine-core",
        ],
        config_file = config_file,
        visibility = ["//visibility:private"],
    )

    # This defines a "Bazel test_suite" [1], basically a different sub test for each
    # browser passed in. For now, this is just one, the hermetic version of chromium.
    # Many of the options on web_test_suite are defined on web_test [2].
    # [1] https://docs.bazel.build/versions/main/be/general.html#test_suite
    # [2] https://github.com/bazelbuild/rules_webtesting/blob/e9cf17123068b1123c68219edf9b274bf057b9cc/web/internal/web_test.bzl#L164
    web_test_suite(
        name = name,
        launcher = ":" + wrapped_test_name,
        browsers = ["@io_bazel_rules_webtesting//browsers:chromium-local"],
        test = wrapped_test_name,
        **kwargs
    )

# This JS code is injected into the the provided karma configuration file. It contains
# Bazel-specific logic that could be re-used across different configuration files.
# Concretely, it sets up the browser configuration and whether we want to just run the tests
# and exit (e.g. the user ran `bazel test foo`) or if we want to have an interactive session
# (e.g. the user ran `bazel run foo`).
_apply_bazel_settings_js_code = """
// Returns true if invoked with bazel run, i.e. the user wants to see the results on a real
// browser.
function isBazelRun() {
  // This env var seems to be a good indicator on Linux, at least.
  return !!process.env['DISPLAY'];
}

// Configures the settings to run chrome.
function applyChromiumSettings(cfg, runfiles, chromiumPath) {
  if (isBazelRun()) {
    cfg.browsers = ['Chrome'];
    cfg.singleRun = false;
  } else {
    // Invoked via bazel test, so run the tests once in a headless browser and be done
    cfg.browsers = ['ChromeHeadless'];
    cfg.singleRun = true;
  }

  try {
    // Setting the CHROME_BIN environment variable tells Karma which chrome to use.
    // We want it to use the Chrome brought via Bazel.
    process.env.CHROME_BIN = runfiles.resolve(chromiumPath);
  } catch {
    throw new Error(`Failed to resolve from  Chromium binary '${
        chromiumPath}' in runfiles`);
  }
}

function applyBazelSettings(cfg) {
  // This is is a JS function provided via environment variables to let us resolve files
  // https://bazelbuild.github.io/rules_nodejs/Built-ins.html#nodejs_binary-templated_args
  const runfiles = require(process.env['BAZEL_NODE_RUNFILES_HELPER']);

  // This is a JSON file that contains this metadata, mixed in with some other data, e.g.
  // the link to the correct executable for the given platform.
  // https://github.com/bazelbuild/rules_webtesting/blob/e9cf17123068b1123c68219edf9b274bf057b9cc/browsers/chromium-local.json
  const webTestMetadata = require(runfiles.resolve(process.env['WEB_TEST_METADATA']));

  const webTestFiles = webTestMetadata['webTestFiles'][0];
  const path = webTestFiles['namedFiles']['CHROMIUM'];
  if (path) {
    applyChromiumSettings(cfg, runfiles, path);
  } else {
    throw new Error("not supported yet");
  }
}

// The user is expected to treat the BAZEL_APPLY_SETTINGS as a function name and pass in
// the configuration as a parameter. Thus, we need to end with the name of our function
// to pick up that argument.
applyBazelSettings"""

def _expand_templates_in_karma_config(ctx):
    # Wrap the absolute paths of our files in quotes and make them space seperated so they
    # can go in the Karma files list.
    srcs = ['"{}"'.format(_absolute_path(ctx, f)) for f in ctx.files.srcs]
    src_list = ", ".join(srcs)

    # Set our base path to that which contains the karma.conf file
    # This requires going up a few directory segments. This allows our absolute paths to
    # all be compatible with each other.
    config_segments = len(ctx.outputs.configuration.short_path.split("/"))
    base_path = "/".join([".."] * config_segments)

    ctx.actions.expand_template(
        output = ctx.outputs.configuration,
        template = ctx.file.config_file,
        substitutions = {
            "BAZEL_SRCS": src_list,
            "BAZEL_BASE_PATH": base_path,
            "BAZEL_APPLY_SETTINGS": _apply_bazel_settings_js_code,
        },
    )

def _absolute_path(ctx, file):
    # Referencing things in @npm yields a short_path that starts with ../
    # For those cases, we can just remove the ../
    if file.short_path.startswith("../"):
        return file.short_path[3:]

    # Otherwise, we have a local file, so we need to include the workspace path to make it
    # an absolute path
    return ctx.workspace_name + "/" + file.short_path

def _create_bash_script_to_invoke_karma(ctx):
    ctx.actions.write(
        output = ctx.outputs.executable,
        is_executable = True,
        content = """#!/usr/bin/env bash
# --- begin runfiles.bash initialization v2 ---
# Copy-pasted from the Bazel Bash runfiles library v2.
# https://github.com/bazelbuild/bazel/blob/master/tools/bash/runfiles/runfiles.bash
set -uo pipefail; f=build_bazel_rules_nodejs/third_party/github.com/bazelbuild/bazel/tools/bash/runfiles/runfiles.bash
source "${{RUNFILES_DIR:-/dev/null}}/$f" 2>/dev/null || \
  source "$(grep -sm1 "^$f " "${{RUNFILES_MANIFEST_FILE:-/dev/null}}" | cut -f2- -d' ')" 2>/dev/null || \
  source "$0.runfiles/$f" 2>/dev/null || \
  source "$(grep -sm1 "^$f " "$0.runfiles_manifest" | cut -f2- -d' ')" 2>/dev/null || \
  source "$(grep -sm1 "^$f " "$0.exe.runfiles_manifest" | cut -f2- -d' ')" 2>/dev/null || \
  {{ echo>&2 "ERROR: cannot find $f"; exit 1; }}; f=; set -e
# --- end runfiles.bash initialization v2 ---

readonly KARMA=$(rlocation "{_KARMA_EXECUTABLE_SCRIPT}")
readonly CONF=$(rlocation "{_KARMA_CONFIGURATION_FILE}")

# set a temporary directory as the home directory, because otherwise Chrome fails to
# start up, complaining about a read-only file system.
export HOME=$(mktemp -d)

readonly COMMAND="${{KARMA}} "start" ${{CONF}}"
${{COMMAND}}
""".format(
            _KARMA_EXECUTABLE_SCRIPT = _absolute_path(ctx, ctx.executable.karma),
            _KARMA_CONFIGURATION_FILE = _absolute_path(ctx, ctx.outputs.configuration),
        ),
    )

def _karma_test_impl(ctx):
    _expand_templates_in_karma_config(ctx)
    _create_bash_script_to_invoke_karma(ctx)

    # Also include files from npm fine grained deps as inputs.
    # These deps are identified by the ExternalNpmPackageInfo provider.
    node_modules_depsets = []
    for dep in ctx.attr.deps:
        if ExternalNpmPackageInfo in dep:
            node_modules_depsets.append(dep[ExternalNpmPackageInfo].sources)
        else:
            print("Not an external npm file?", dep)
    node_modules = depset(transitive = node_modules_depsets)

    # We need the karma configuration file...
    runfiles = [
        ctx.outputs.configuration,
    ]

    # ...the test files the user wants to run
    runfiles += ctx.files.srcs

    # ...the npm packages which karma is depending on being there (e.g. jasmine)
    runfiles += ctx.files.deps

    # https://docs.bazel.build/versions/main/skylark/lib/DefaultInfo.html
    return [DefaultInfo(
        # https://docs.bazel.build/versions/main/skylark/lib/depset.html
        files = depset([ctx.outputs.executable]),
        runfiles = ctx.runfiles(
            files = runfiles,
            transitive_files = depset(transitive = [node_modules]),
        ).merge(ctx.attr.karma[DefaultInfo].data_runfiles),
        executable = ctx.outputs.executable,
    )]

_karma_test = rule(
    implementation = _karma_test_impl,
    test = True,
    executable = True,
    attrs = {
        "config_file": attr.label(
            doc = "The karma config file",
            mandatory = True,
            allow_single_file = [".js"],
        ),
        "srcs": attr.label_list(
            doc = "A list of JavaScript test files",
            allow_files = [".js"],
            mandatory = True,
        ),
        "deps": attr.label_list(
            doc = """Any karma plugins (aka peer deps) required. These are generally listed
            in the provided config_file.""",
            allow_files = True,
            aspects = [node_modules_aspect],
            mandatory = True,
        ),
        "karma": attr.label(
            doc = "karma binary label",
            # By default, we use the karma pulled in via Bazel running npm install
            default = "@npm//karma/bin:karma",
            executable = True,
            cfg = "exec",
            allow_files = True,
        ),
    },
    outputs = {
        "configuration": "%{name}.conf.js",
    },
)
