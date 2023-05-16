"""This module defines the skia_android_unit_test macro."""

load("//bazel:cc_binary_with_flags.bzl", "cc_binary_with_flags")

def skia_test(
        name,
        srcs,
        deps,
        requires_resources_dir = False,
        extra_args = [],
        flags = {},
        limit_to = [],
        tags = [],
        size = None):
    """Defines a generic Skia C++ unit test.

    This macro produces a <name>_binary C++ binary and a <name>.sh wrapper script that runs the
    binary with the desired command-line arguments (see the extra_args and requires_resources_dir
    arguments). The <name>.sh wrapper script is exposed as a Bazel test target via the sh_target
    rule.

    The reason why we place command-line arguments in a wrapper script is that it makes it easier
    to run a Bazel-built skia_test outside of Bazel. This is useful e.g. for CI jobs where we want
    to perform test compilation and execution as different steps on different hardware (e.g.
    compile on a GCE machine, run tests on a Skolo device). In this scenario, the test could be
    executed outside of Bazel by simply running the <name>.sh script without any arguments. See the
    skia_android_unit_test macro for an example.

    Note: The srcs attribute must explicitly include a test runner (e.g.
    //tests/BazelTestRunner.cpp).

    Args:
        name: The name of the test.
        srcs: C++ source files.
        deps: C++ library dependencies.
        requires_resources_dir: Indicates whether this test requires any files under //resources,
            such as images, fonts, etc. If so, the compiled C++ binary will be invoked with flag
            --resourcePath set to the path to the //resources directory under the runfiles tree.
            Note that this implies the test runner must recognize the --resourcePath flag for this
            to work.
        extra_args: Any additional command-line arguments to pass to the compiled C++ binary.
        flags: A map of strings to lists of strings to specify features that must be compiled in
            for these tests to work. For example, tests targeting our codec logic will want the
            various codecs included, but most tests won't need that.
        limit_to: A list of platform labels (e.g. @platform//os:foo; @platform//cpu:bar) which
            restrict where this test will be compiled and ran. If the list is empty, it will run
            anywhere. If it is non-empty, it will only run on platforms which match the entire set
            of constraints. See https://github.com/bazelbuild/platforms for these.
        tags: A list of tags for the generated test target.
        size: The size of the test.
    """
    test_binary = "%s_binary" % name

    # We compile the test as a cc_binary, rather than as as a cc_test, because we will not
    # "bazel test" this binary directly. Instead, we will "bazel test" a wrapper script that
    # invokes this binary with the required command-line parameters.
    cc_binary_with_flags(
        name = test_binary,
        srcs = srcs,
        deps = deps,
        data = ["//resources"] if requires_resources_dir else [],
        set_flags = flags,
        target_compatible_with = limit_to,
        testonly = True,  # Needed to gain access to test-only files.
    )

    test_runner = "%s.sh" % name

    test_args = ([
        "--resourcePath",
        "$$(realpath $$(dirname $(rootpath //resources:README)))",
    ] if requires_resources_dir else []) + extra_args

    # This test runner might run on Android devices, which might not have a /bin/bash binary.
    test_runner_template = remove_indentation("""
        #!/bin/sh
        $(rootpath {test_binary}) {test_args}
    """)

    # TODO(lovisolo): This should be an actual rule. This will allow us to select() the arguments
    #                 based on the device (e.g. for device-specific --skip flags to skip tests).
    native.genrule(
        name = "%s_runner" % name,
        srcs = [test_binary] + (
            # The script template computes the path to //resources under the runfiles tree via
            # $$(dirname $(rootpath //resources:README)), so we need to list //resources:README
            # here explicitly. This file was chosen arbitrarily; there is nothing special about it.
            ["//resources", "//resources:README"] if requires_resources_dir else []
        ),
        outs = [test_runner],
        cmd = "echo '%s' > $@" % test_runner_template.format(
            test_binary = test_binary,
            test_args = "\\\n    ".join(test_args),
        ),
        testonly = True,
    )

    native.sh_test(
        name = name,
        size = size,
        srcs = [test_runner],
        data = [test_binary] + (["//resources"] if requires_resources_dir else []),
        tags = tags,
    )

def skia_android_unit_test(
        name,
        srcs,
        deps = [],
        flags = {},
        extra_args = [],
        requires_condition = "//:always_true",
        requires_resources_dir = False):
    """Defines a Skia Android unit test.

    This macro compiles one or more C++ unit tests into a single Android binary and produces a
    script that runs the test on an attached Android device via `adb`.

    This macro requires a device-specific Android platform such as //bazel/devices:pixel_5. This is
    used to decide what device-specific set-up steps to apply, such as setting CPU/GPU frequencies.

    The test target produced by this macro can be executed on a machine attached to an Android
    device. This can be either via USB, or by port-forwarding a remote ADB server (TCP port 5037)
    running on a machine attached to the target device, such as a Skolo Raspberry Pi.

    High-level overview of how this rule works:

    - It produces a <name>.tar.gz archive containing the Android binary and a minimal launcher
      script that invokes the binary with the necessary command-line arguments.
    - It produces a <name>.sh test runner script that extracts the tarball into the device via
      `adb`, sets up the device, runs the test, cleans up and pipes through the test's exit code.

    For CI jobs, rather than invoking "bazel test" on a Raspberry Pi attached to the Android device
    under test, we compile and run the test in two separate tasks:

    - A build task running on a GCE machine compiles the test on RBE with Bazel, and stores the
      <name>.tar.gz and <name>.sh output files to CAS.
    - A test task running on a Skolo Raspberry Pi downloads <name>.tar.gz and <name>.sh from CAS
      and executes <name>.sh *outside of Bazel*.

    The reason for this is that we don't want to run Bazel on a Raspberry Pi due to its constrained
    resources.

    Note: Although not currently supported, we could use a similar approach for Apple devices in
    in the future.

    Args:
        name: The name of the test.
        srcs: A list of C++ source files. This list should not include a main function (see the
            requires_condition argument).
        deps: Any dependencies needed by the srcs. This list should not include a main function
            (see the requires_condition argument).
        flags: A map of strings to lists of strings to specify features that must be compiled in
            for these tests to work. For example, tests targeting our codec logic will want the
            various codecs included, but most tests won't need that.
        extra_args: Additional command-line arguments to pass to the test, for example, any
            device-specific --skip flags to skip incompatible or buggy test cases.
            TODO(lovisolo): Do we need to support skipping tests? IIUC today we only skip DMs, but
                            we don't skip any unit tests.
        requires_condition: A necessary condition for the test to work. For example, GPU tests
            should set this argument to "//src/gpu:has_gpu_backend". If the condition is satisfied,
            //tests:BazelTestRunner.cpp will be appended to the srcs attribute. If the condition is
            not satisfied, //tests:BazelNoopRunner.cpp will be included instead, and no deps will
            be included. This prevents spurious build failures when using wildcard expressions
            (e.g. "bazel build //tests/...") with a configuration that is incompatible with this
            test. TODO(lovisolo): Do we actually need this?
        requires_resources_dir: If set, the contents of the //resources directory will be included
            in the test runfiles, and the test binary will be invoked with flag --resourcePath set
            to the path to said directory.
    """

    skia_test(
        name = "%s_cpp_test" % name,
        srcs = select({
            requires_condition: srcs + ["//tests:BazelTestRunner.cpp"],
            "//conditions:default": ["//tests:BazelNoopRunner.cpp"],
        }),
        deps = select({
            requires_condition: deps,
            "//conditions:default": [],
        }),
        flags = flags,
        extra_args = extra_args,
        requires_resources_dir = requires_resources_dir,
        tags = [
            # Exclude it from wildcards, e.g. "bazel test //...". We never want to run this binary
            # directly.
            "manual",
            "no-remote",  # RBE workers cannot run Android tests.
        ],
        size = "large",  # Can take several minutes.
        # limit_to = ["@platforms//os:android"],
    )

    test_binary = "%s_cpp_test_binary" % name
    test_runner = "%s_cpp_test.sh" % name

    archive = "%s_archive" % name
    archive_srcs = [test_runner, test_binary] + (
        ["//resources"] if requires_resources_dir else []
    )

    # Create an archive containing the test and its resources, with a structure that emulates
    # the environment expected by the test when executed via "bazel test". This archive can be
    # pushed to an Android device via "adb push", and the contained test can be executed on the
    # device via "adb shell" as long as the working directory is set to the directory where the
    # archive is extracted.
    #
    # See https://bazel.build/reference/test-encyclopedia#initial-conditions.
    #
    # TODO(lovisolo): Replace this unreadable Bash script with a Go program.
    native.genrule(
        name = archive,
        srcs = archive_srcs,
        outs = ["%s.tar.gz" % name],
        cmd = remove_indentation("""
            # Create archive root directory, and ensure it is cleaned up on exit, even in the
            # case of errors.
            ARCHIVE_DIR=$$(mktemp -d);
            trap "rm -rf $$ARCHIVE_DIR" EXIT;

            # execpaths point to physical files generated by Bazel (e.g.
            # bazel-out/k8-linux_x64-dbg/bin/tests/some_test), whereas rootpaths are the paths
            # that a binary running via "bazel run" or "bazel test" expects (e.g
            # tests/some_test). Thus, we must map the former to the latter.
            #
            # Reference:
            # https://bazel.build/reference/be/make-variables#predefined_label_variables
            EXECPATHS=({execpaths});
            ROOTPATHS=({rootpaths});
            if [ $${{#EXECPATHS[*]}} -ne $${{#ROOTPATHS[*]}} ]; then
                echo EXECPATHS and ROOTPATHS have different lengths;
                exit 1;
            fi;

            # Copy each file in EXECPATHS into ARCHIVE_DIR, renamed as its corresponding entry
            # in ROOTPATHS.
            for (( i=0; i<$${{#EXECPATHS[*]}}; i++ )); do
                EXECPATH=$${{EXECPATHS[$$i]}};
                ROOTPATH=$${{ROOTPATHS[$$i]}};
                mkdir -p $$ARCHIVE_DIR/$$(dirname $$ROOTPATH);
                cp $$EXECPATH $$ARCHIVE_DIR/$$ROOTPATH;
            done;

            # Create archive.
            tar zcf $@ -C $$ARCHIVE_DIR .;
        """.format(
            execpaths = " ".join(["$(execpaths %s)" % src for src in archive_srcs]),
            rootpaths = " ".join(["$(rootpaths %s)" % src for src in archive_srcs]),
        )),
        testonly = True,  # Needed to gain access to test-only files.
    )

    adb_test_runner_script = "%s.sh" % name

    adb_test_runner(
        name = "%s_runner" % name,
        archive = archive,
        test_runner = test_runner,
        device = select(
            {
                "//bazel/devices:pixel_5": "pixel_5",
                "//bazel/devices:pixel_7": "pixel_7",
            },
            no_match_error = "Did you forget to select a device-specific platform? (Example: " +
                             "//bazel/devices:pixel_5).",
        ),
        output = adb_test_runner_script,
        testonly = True,
    )

    native.sh_test(
        name = name,
        size = "small",
        srcs = [adb_test_runner_script],
        data = [archive],
        tags = [
            # Exclude it from wildcards, e.g. "bazel test //...". We cannot assume that the machine
            # Bazel is running on is attached to an appropriate Android device.
            #
            # TODO(lovisolo): Use platforms and target_compatible_with to achieve this.
            "manual",
            # This test runner assumes there is an appropriate Android device connected via
            # USB, therefore it cannot run on RBE.
            "no-remote",
        ],
    )

# TODO(lovisolo): Should we move this to a common module with utility functions?
def remove_indentation(string):
    """Removes indentation from a multiline string.

    This utility function allows us to write multiline templates in a context that requires
    indentation, for example inside a macro. It discards the first and last lines if they only
    contain spaces or tabs. Then, it computes an indentation prefix based on the first remaining
    line and removes that prefix from all lines.

    Example:

    ```
    def greeter_script():
        return remove_indentation('''
            #!/bin/bash
            echo "Hello, {name}!"
        ''').format(name = "world")
    ```

    This is equivalent to:

    ```
    TEMPLATE = '''#!/bin/bash
    echo "Hello, {name}!"
    '''

    def greeter_script():
        return TEMPLATE.format(name = "world")
    ```

    Args:
        string: A multiline string.
    Returns:
        The input string minus any indentation.
    """

    def get_indentation(line):
        indentation = ""
        for char in line.elems():
            if char in [" ", "\t"]:
                indentation += char
            else:
                break
        return indentation

    lines = string.split("\n")

    # Skip first line if empty.
    if get_indentation(lines[0]) == lines[0]:
        lines = lines[1:]

    # Compute indentation based on the first remaining line, and remove indentation from all lines.
    indentation = get_indentation(lines[0])
    lines = [line.removeprefix(indentation) for line in lines]

    # Skip last line if empty.
    if get_indentation(lines[len(lines) - 1]) == lines[len(lines) - 1]:
        lines = lines[:-1]

    result = "\n".join(lines)
    if result[:-1] != "\n":
        # Ensure we always end with a newline.
        result += "\n"

    return result

def _adb_test_runner_impl(ctx):
    # TODO(lovisolo): Add device-specific (via ctx.attr.device) setup steps such as turning cores
    #                 on/off and setting the CPU/GPU frequencies.

    template = remove_indentation("""
        #!/bin/bash

        # Runner script for device "{device}".

        # TODO(lovisolo): Should we check that the machine is attached to the expected device type?
        #                 E.g. run "adb devices -l" and check that the output contains
        #                 "model:Pixel_5".

        # Note: this script was only tested on Pixel devices.
        #
        # The /sdcard/revenge_of_the_skiabot directory is writable for non-root users, but files in
        # this directory cannot be executed. For this reason, we extract the archive in a directory
        # under /data, which allows executing files but requires root privileges.
        #
        # TODO(lovisolo): Can we do this without "su"-ing as root? Does this work on non-rooted
        #                 devices?
        # TODO(lovisolo): Test on more devices.

        ARCHIVE_ON_DEVICE=/sdcard/revenge_of_the_skiabot/bazel-adb-test.tar.gz
        DIRECTORY_ON_DEVICE=/data/bazel-adb-test

        # Print commands and expand variables for easier debugging.
        set -x

        # Ensure that we clean up the device on exit, even in the case of failures.
        # TODO(lovisolo): Also clean up before running the test, as the device might be in a dirty
        #                 state if the previous task did not finish correctly (e.g. device reboot).
        trap "adb shell su root rm -rf ${{ARCHIVE_ON_DEVICE}} ${{DIRECTORY_ON_DEVICE}}" EXIT

        # Upload archive.
        adb push $(rootpath {archive}) ${{ARCHIVE_ON_DEVICE}}

        # Extract archive.
        adb shell su root mkdir ${{DIRECTORY_ON_DEVICE}}
        adb shell su root tar xzvf ${{ARCHIVE_ON_DEVICE}} -C ${{DIRECTORY_ON_DEVICE}}

        # Run test inside the directory where the archive was extracted. We set the working
        # directory to the root of the archive, which emulates the directory structure expected by
        # the test when invoked with "bazel test". See
        # https://bazel.build/reference/test-encyclopedia#initial-conditions.
        echo "cd ${{DIRECTORY_ON_DEVICE}} && $(rootpath {test_runner})" | adb shell su root
    """)

    # Expand variables.
    template = ctx.expand_location(template.format(
        device = ctx.attr.device,
        archive = ctx.attr.archive.label,
        test_runner = ctx.attr.test_runner.label,
    ), targets = [
        ctx.attr.archive,
        ctx.attr.test_runner,
    ])

    ctx.actions.write(ctx.outputs.output, template, is_executable = True)

    return [DefaultInfo(files = depset([ctx.outputs.output]))]

adb_test_runner = rule(
    implementation = _adb_test_runner_impl,
    attrs = {
        "device": attr.string(mandatory = True, values = [
            "pixel_5",
            "pixel_7",
        ]),
        "test_runner": attr.label(allow_files = [".sh"], mandatory = True),
        "archive": attr.label(allow_files = [".tar.gz"], mandatory = True),
        "output": attr.output(mandatory = True),
    },
)
