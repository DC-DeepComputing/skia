"""This module defines the skia_android_unit_test macro."""

load("//bazel:cc_binary_with_flags.bzl", "cc_binary_with_flags")
load(
    ":testgroups.bzl",
    "CODEC_TESTS",
    "GANESH_TESTS",
    "PATHOPS_TESTS",
)

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

# Device names recognized by the skia_android_unit_test macro.
PIXEL_5 = "pixel_5"
PIXEL_7 = "pixel_7"

# Named test groups recognized by the skia_android_unit_test macro.
CODEC = "codec"
GANESH = "ganesh"
PATHOPS = "pathops"

# Flavors recognized by the skia_android_unit_test macro.
DEBUG = "debug"
RELEASE = "release"
GL = "gl"
VULKAN = "vulkan"

def skia_android_unit_test(name, device, test_group, flavors = []):
    """Defines a Skia Android test.

    This macro takes a device name (e.g. "pixel_5"), a named group of C++ unit tests (e.g.
    "ganesh") and an optional list of flavors (e.g. ["debug", "vulkan"]), and produces a skia_test
    target with label <name>_cpp_test. This target compiles the C++ unit tests, along with
    //tests/BazelTestRunner.cpp, with a set of flags appropriate for the given combination of
    device, tests and flavors. These flags include //command_line_option:platforms, which is
    set to @platforms//os:android. This allows us to "bazel build" the target without passing any
    additional command-line arugments to Bazel.

    This macro also generates a <name> sh_test that runs the skia_test on an attached Android
    device via ADB. This target works as follows:

     - It produces a <name>.tar.gz tarball containing the C++ binary and wrapper script produced by
       the skia_test rule, and the contents of the //resources directory if needed by the test.
     - It produces a <name>.sh script that copies the tarball into the attached Android device via
       ADB, extracts it, runs the test on the device, cleans up the copied files, and pipes
       through the test's exit code.
     - When executed via "bazel test", it runs the <name>.sh script and reports the test results.

    The <name> target can be executed with "bazel test" on a developer workstation if one of these
    conditions is met:

     - The machine has an Android device attached via USB and "adb" is installed.
     - TCP port 5037 is forwarded from a different machine running an ADB server with an attached
       Android device, such as a Raspberry Pi leased from the Skolo.

    For CI jobs, it is possible to perform test compilation and test execution as separate steps.
    For example, test compilation can happen on a GCE machine via "bazel build <name>", and test
    execution can happen on a Raspberry Pi attached to an appropriate Android device. Given that we
    do not want to run Bazel on Raspberry Pis due to their limited resources, we would execute the
    test by copying into the Pi the <name>.tar.gz and <name>.sh artifacts generated from the
    previous step and by running <name>.sh with no arguments.

    Note: Although not currently supported, we could use a similar approach for Apple devices in
    in the future.

    Args:
        name: The name of the test.
        device: The device that this target should run on (e.g. "pixel_5").
        test_group: Which named collection of C++ tests to run (e.g. "ganesh").
        flavors: A list of named variants (e.g. ["debug", "vulkan"]). Optional.
    """

    # These delegate functions determine what gets tested and how.
    srcs = _skia_android_unit_test_srcs(device, test_group, flavors)
    deps = _skia_android_unit_test_deps(device, test_group, flavors)
    flags = _skia_android_unit_test_flags(device, test_group, flavors)
    extra_args = _skia_android_unit_test_extra_args(device, test_group, flavors)
    requires_resources_dir = _skia_android_unit_test_requires_resources_dir(device, test_group, flavors)
    adb_test_runner_template = _skia_android_unit_test_adb_test_runner_template(device, test_group, flavors)

    skia_test(
        name = "%s_cpp_test" % name,
        srcs = srcs,
        deps = deps,
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

    adb_test_runner = "%s.sh" % name

    native.genrule(
        name = "%s_runner" % name,
        srcs = [archive, test_runner],
        outs = [adb_test_runner],
        cmd = "echo '%s' > $@" % adb_test_runner_template.format(
            archive = archive,
            test_runner = test_runner,
        ),
        testonly = True,
    )

    native.sh_test(
        name = name,
        size = "small",
        srcs = [adb_test_runner],
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

def _skia_android_unit_test_srcs(device, test_group, flavors):  # buildifier: disable=unused-variable
    srcs = ["BazelTestRunner.cpp"]
    if test_group == CODEC:
        srcs += CODEC_TESTS
    elif test_group == GANESH:
        srcs += GANESH_TESTS
    elif test_group == PATHOPS:
        srcs += PATHOPS_TESTS
    else:
        fail("Unknown test_group:", test_group)
    return srcs

def _skia_android_unit_test_deps(device, test_group, flavors):  # buildifier: disable=unused-variable
    deps = ["//:skia_internal"]
    if test_group == CODEC:
        deps.append(":tests_base")
    elif test_group == GANESH:
        deps.append(":tests_base")
    elif test_group == PATHOPS:
        deps.append(":pathops_tests_base")
    else:
        fail("Unknown test_group:", test_group)
    return deps

def _skia_android_unit_test_flags(device, test_group, flavors):  # buildifier: disable=unused-variable
    _check_conflicting_flavors(flavors, [DEBUG, RELEASE])
    _check_conflicting_flavors(flavors, [GL, VULKAN])

    compilation_mode = "dbg" if DEBUG in flavors else "opt"  # Default to RELEASE flavor.
    gpu_backend = "vulkan_backend" if VULKAN in flavors else "gl_backend"  # Default to GL flavor.

    flags = {
        "platforms": [
            # TODO(lovisolo): This can be decided based on the device (32-bit vs. 64-bit).
            # TODO(lovisolo): Should this be a configurable flavor? That is, Does it make sense to
            #                 have 32-bit and 64-bit builds for the same device?
            "//bazel/platform:android_arm64",
        ],
        "compilation_mode": [compilation_mode],

        # TODO(lovisolo): Are there any test groups that do not need a GPU backend under Android?
        "gpu_backend": [gpu_backend],
        "enable_gpu_test_utils": ["true"],
    }
    if test_group == CODEC:
        flags.update({
            "include_decoder": [
                "avif_decode_codec",
                "gif_decode_codec",
                "jpeg_decode_codec",
                "jxl_decode_codec",
                "png_decode_codec",
                "raw_decode_codec",
                "webp_decode_codec",
            ],
            "include_encoder": [
                "jpeg_encode_codec",
                "png_encode_codec",
                "webp_encode_codec",
            ],
        })
    elif test_group == GANESH:
        pass  # Nothing to add.
    elif test_group == PATHOPS:
        pass  # Nothing to add.
    else:
        fail("Unknown test_group:", test_group)
    return flags

def _skia_android_unit_test_extra_args(device, test_group, flavors):  # buildifier: disable=unused-variable
    # TODO(lovisolo): This is a good place to compute a list of tests to skip (e.g. via --skip).
    return []

def _skia_android_unit_test_requires_resources_dir(device, test_group, flavors):  # buildifier: disable=unused-variable
    requires_resources_dir = False
    if test_group == CODEC:
        requires_resources_dir = True
    elif test_group == GANESH:
        requires_resources_dir = True
    elif test_group == PATHOPS:
        requires_resources_dir = False
    else:
        fail("Unknown test_group:", test_group)
    return requires_resources_dir

def _skia_android_unit_test_adb_test_runner_template(device, test_group, flavors):  # buildifier: disable=unused-variable
    # TODO(lovisolo): Add device-specific setup steps such as setting the CPU/GPU frequencies.

    # Curly braces are escaped ("{" becomes "{{", "}" becomes "}}") because we will format this
    # template with https://bazel.build/rules/lib/string#format.
    return remove_indentation("""
        #!/bin/bash

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
        trap "adb shell su root rm -rf $${{ARCHIVE_ON_DEVICE}} $${{DIRECTORY_ON_DEVICE}}" EXIT

        # Upload archive.
        adb push $(rootpath {archive}) $${{ARCHIVE_ON_DEVICE}}

        # Extract archive.
        adb shell su root mkdir $${{DIRECTORY_ON_DEVICE}}
        adb shell su root tar xzvf $${{ARCHIVE_ON_DEVICE}} -C $${{DIRECTORY_ON_DEVICE}}

        # Run test inside the directory where the archive was extracted. We set the working
        # directory to the root of the archive, which emulates the directory structure expected by
        # the test when invoked with "bazel test". See
        # https://bazel.build/reference/test-encyclopedia#initial-conditions.
        echo "cd $${{DIRECTORY_ON_DEVICE}} && $(rootpath {test_runner})" | adb shell su root
    """)

def _check_conflicting_flavors(actual_flavors, conflicting_flavors):
    found = [flavor for flavor in actual_flavors if flavor in conflicting_flavors]
    if len(found) > 1:
        fail("Conflicting flavors: " + str(found))

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

    return "\n".join(lines)
