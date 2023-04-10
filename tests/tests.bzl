"""
This file contains macros that generate multiple test targets, one per file.
"""

load("//bazel:cc_test_with_flags.bzl", "cc_test_with_flags")
load("//bazel:cc_binary_with_flags.bzl", "cc_binary_with_flags")

def skia_cpu_tests(
        name,
        tests,
        harness,
        resources = [],
        flags = {},
        extra_deps = [],
        limit_to = []):
    """Defines tests that should run only with --config=cpu

    This macro will create one cc_test_with_flags rule for each file in tests.
    These tests are configured to use the BazelTestRunner and actually run all tests
    (e.g. those defined with DEF_TEST) in the file if no GPU backend has been specified,
    that is, the bazel test command was invoked with --config=cpu (see buildrc for this
    definition).

    This macro cannot un-define those targets completely if a GPU backend has been specified
    because the macro runs before the config flags are available for use. An analogy is a
    C++ macro cannot do a #if on the value of a C++ variable.

    Instead, if a GPU backend has been specified (e.g. --config=gl), we only compile the
    BazelNoopRunner.cpp and nothing else (e.g. no deps, not even Skia). This makes any CPU tests
    a trivially passing executable, which can easily be cached and effectively skipped.

    Args:
        name: The name of the test_suite that groups these tests together.
        tests: A list of strings, corresponding to C++ files with one or more DEF_TEST (see Test.h).
        harness: A label (string) corresponding to a skia_cc_library which all the supplementary
                 test helpers, utils, etc. (e.g. Test.h and Test.cpp). Ideally, this is as small
                 as feasible, to avoid unnecessary bloat or recompilation if something unrelated
                 changes.
        resources: A label corresponding to a file_group target that has any skia resource files
                   (e.g. images, fonts) needed to run these tests. Resources change infrequently,
                   so it's not super important that this be a precise list.
        flags: A map of strings to lists of strings to specify features that must be compiled in
               for these tests to work. For example, tests targeting our codec logic will want the
               various codecs included, but most tests won't need that.
        extra_deps: A list of labels corresponding to skia_cc_library targets which this test needs
                    to work, typically some utility located under //tools
        limit_to: A list of platform labels (e.g. @platform//os:foo; @platform//cpu:bar) which
                  restrict where this test will be compiled and ran. If the list is empty, it will
                  run anywhere. If it is non-empty, it will only run on platforms which match the
                  entire set of constraints. See https://github.com/bazelbuild/platforms for these.
    """
    test_targets = []
    for filename in tests:
        new_target = filename[:-4]  # trim .cpp
        test_targets.append(new_target)
        cc_test_with_flags(
            name = new_target,
            size = "small",
            srcs = select({
                # Make this a no-op test if compiling with a GPU backend.
                ":skip_cpu_tests": ["BazelNoopRunner.cpp"],
                "//conditions:default": [
                    "BazelTestRunner.cpp",
                    filename,
                ],
            }),
            deps = select({
                ":skip_cpu_tests": [],
                # Only build and apply deps if we have a no-op test.
                "//conditions:default": [
                    harness,
                    "//:skia_internal",
                ] + extra_deps,
            }),
            data = resources,
            set_flags = flags,
            target_compatible_with = limit_to,
        )

    # https://bazel.build/reference/be/general#test_suite
    native.test_suite(
        name = name,
        tests = test_targets,
    )

def skia_ganesh_tests(
        name,
        tests,
        harness,
        resources = [],
        flags = {},
        extra_deps = [],
        limit_to = []):
    """Defines tests that should run only when a Ganesh GPU backend is compiled in, e.g --config=gl

    This macro will create one cc_test_with_flags rule for each file in tests.
    These tests are configured to use the BazelTestRunner and actually run all tests (e.g. those
    defined with DEF_TEST or DEF_GANESH_TEST) in the file if a Ganesh GPU backend has been
    specified, that is, the bazel test command was invoked with --config=gl or --config=vk
    (see buildrc for this definition).

    This macro cannot un-define those targets completely if only the CPU backend is being used
    because the macro runs before the config flags are available for use. An analogy is a
    C++ macro cannot do a #if on the value of a C++ variable.

    Instead, if a Ganesh GPU backend has not been specified (e.g. --config=cpu), we only compile the
    BazelNoopRunner.cpp and nothing else (e.g. no deps, not even Skia). This makes any Ganesh GPU
    tests a trivially passing executable, which can easily be cached and effectively skipped.

    Test files may have both CPU tests (i.e. DEF_TEST) and GPU tests (e.g. DEF_GANESH_TEST). The
    BazelTestRunner will always run all tests that were compiled in. The CPU tests defined in this
    way will *not* be run with --config=cpu because they live in a file that can only be compiled
    with at least one GPU backend; they will be run with a config like --config=gl, along side the
    GPU tests defined with DEF_GANESH_TEST.

    Args:
        name: The name of the test_suite that groups these tests together.
        tests: A list of strings, corresponding to C++ files with one or more DEF_GANESH_TEST
               (see Test.h).
        harness: A label (string) corresponding to a skia_cc_library which all the supplementary
                 test helpers, utils, etc. (e.g. Test.h and Test.cpp). Ideally, this is as small
                 as feasible, to avoid unnecessary bloat or recompilation if something unrelated
                 changes.
        resources: A label corresponding to a file_group target that has any skia resource files
                   (e.g. images, fonts) needed to run these tests. Resources change infrequently,
                   so it's not super important that this be a precise list.
        flags: A map of strings to lists of strings to specify features that must be compiled in
               for these tests to work. For example, tests targeting our codec logic will want the
               various codecs included, but most tests won't need that.
        extra_deps: A list of labels corresponding to skia_cc_library targets which this test needs
                    to work, typically some utility located under //tools
        limit_to: A list of platform labels (e.g. @platform//os:foo; @platform//cpu:bar) which
                  restrict where this test will be compiled and ran. If the list is empty, it will
                  run anywhere. If it is non-empty, it will only run on platforms which match the
                  entire set of constraints. See https://github.com/bazelbuild/platforms for these.
    """
    test_targets = []
    for filename in tests:
        new_target = filename[:-4]  # trim .cpp
        test_targets.append(new_target)
        cc_test_with_flags(
            name = new_target,
            size = "small",
            srcs = select({
                "//src/gpu:has_gpu_backend": [
                    "BazelTestRunner.cpp",
                    filename,
                ],
                # Make this a no-op test if not compiling with a Ganesh GPU backend.
                "//conditions:default": ["BazelNoopRunner.cpp"],
            }),
            deps = select({
                # Only build and apply deps if we have a no-op test.
                "//src/gpu:has_gpu_backend": [
                    harness,
                    "//:skia_internal",
                ] + extra_deps,
                "//conditions:default": [],
            }),
            data = resources,
            set_flags = flags,
            target_compatible_with = limit_to,
            tags = [
                # We currently have no RBE machines with GPUs, so we cannot run these remotely.
                "no-remote",
            ],
        )

    # https://bazel.build/reference/be/general#test_suite
    native.test_suite(
        name = name,
        tests = test_targets,
    )

# This test runner might run on Android devices, which typically do not have a /bin/bash binary.
_TEST_RUNNER_TEMPLATE = """#!/bin/sh
$(rootpath {test_binary}) --resourcePath $$(realpath $$(dirname $(rootpath //resources:README)))
"""

# Curly braces are escaped ("{" becomes "{{", "}" becomes "}}") because we will format this
# template with https://bazel.build/rules/lib/string#format.
_ADB_TEST_RUNNER_TEMPLATE = """#!/bin/bash

# Note: this script was only tested on a Pixel 5.
#
# The /sdcard/revenge_of_the_skiabot directory is writable for non-root users, but files in this
# directory cannot be executed. For this reason, we extract the archive in a directory under /data,
# which allows executing files but requires root privileges.
#
# TODO(lovisolo): Can we do this without "su"-ing as root? Does this work on non-rooted devices?
# TODO(lovisolo): Test on more devices.
ARCHIVE_ON_DEVICE=/sdcard/revenge_of_the_skiabot/bazel-adb-test.tar.gz
DIRECTORY_ON_DEVICE=/data/bazel-adb-test

# Print commands and expand variables for easier debugging.
set -x

# Ensure that we clean up the device on exit, even in the case of failures.
trap "adb shell su root rm -rf $${{ARCHIVE_ON_DEVICE}} $${{DIRECTORY_ON_DEVICE}}" EXIT

# Upload archive.
adb push $(rootpath {archive}) $${{ARCHIVE_ON_DEVICE}}

# Extract archive.
adb shell su root mkdir $${{DIRECTORY_ON_DEVICE}}
adb shell su root tar xzvf $${{ARCHIVE_ON_DEVICE}} -C $${{DIRECTORY_ON_DEVICE}}

# Run test inside the directory where the archive was extracted. We set the working directory to
# the root of the archive, which emulates the directory structure expected by the test when invoked
# with "bazel test". See https://bazel.build/reference/test-encyclopedia#initial-conditions.
echo "cd $${{DIRECTORY_ON_DEVICE}} && $(rootpath {test_binary})" | adb shell su root
"""

def skia_ganesh_test(
        name,
        tests,
        harness,
        resources = [],
        flags = {},
        extra_deps = [],
        limit_to = []):
    """Defines tests that should run only when a Ganesh GPU backend is compiled in, e.g --config=gl

    This macro will create one cc_test_with_flags rule for each file in tests.
    These tests are configured to use the BazelTestRunner and actually run all tests (e.g. those
    defined with DEF_TEST or DEF_GANESH_TEST) in the file if a Ganesh GPU backend has been
    specified, that is, the bazel test command was invoked with --config=gl or --config=vk
    (see buildrc for this definition).

    This macro cannot un-define those targets completely if only the CPU backend is being used
    because the macro runs before the config flags are available for use. An analogy is a
    C++ macro cannot do a #if on the value of a C++ variable.

    Instead, if a Ganesh GPU backend has not been specified (e.g. --config=cpu), we only compile the
    BazelNoopRunner.cpp and nothing else (e.g. no deps, not even Skia). This makes any Ganesh GPU
    tests a trivially passing executable, which can easily be cached and effectively skipped.

    Test files may have both CPU tests (i.e. DEF_TEST) and GPU tests (e.g. DEF_GANESH_TEST). The
    BazelTestRunner will always run all tests that were compiled in. The CPU tests defined in this
    way will *not* be run with --config=cpu because they live in a file that can only be compiled
    with at least one GPU backend; they will be run with a config like --config=gl, along side the
    GPU tests defined with DEF_GANESH_TEST.

    Args:
        name: The name of the test_suite that groups these tests together.
        tests: A list of strings, corresponding to C++ files with one or more DEF_GANESH_TEST
               (see Test.h).
        harness: A label (string) corresponding to a skia_cc_library which all the supplementary
                 test helpers, utils, etc. (e.g. Test.h and Test.cpp). Ideally, this is as small
                 as feasible, to avoid unnecessary bloat or recompilation if something unrelated
                 changes.
        resources: A label corresponding to a file_group target that has any skia resource files
                   (e.g. images, fonts) needed to run these tests. Resources change infrequently,
                   so it's not super important that this be a precise list.
        flags: A map of strings to lists of strings to specify features that must be compiled in
               for these tests to work. For example, tests targeting our codec logic will want the
               various codecs included, but most tests won't need that.
        extra_deps: A list of labels corresponding to skia_cc_library targets which this test needs
                    to work, typically some utility located under //tools
        limit_to: A list of platform labels (e.g. @platform//os:foo; @platform//cpu:bar) which
                  restrict where this test will be compiled and ran. If the list is empty, it will
                  run anywhere. If it is non-empty, it will only run on platforms which match the
                  entire set of constraints. See https://github.com/bazelbuild/platforms for these.
    """
    test_binary = "%s_binary" % name

    # We compile the test as a cc_binary, rather than as as a cc_test, because we will not
    # "bazel test" this binary directly. Instead, we will "bazel test" a wrapper script that
    # invokes this binary with the required command-line parameters. Marking the binary itself as a
    # test would cause a wildcard invocation such as "bazel test //..." to execute the binary,
    # which would result in a test failure because the binary did not get the required arguments.
    cc_binary_with_flags(
        name = test_binary,
        srcs = select({
            "//src/gpu:has_gpu_backend": [
                "BazelTestRunner.cpp",
            ] + tests,
            # Make this a no-op test if not compiling with a Ganesh GPU backend.
            "//conditions:default": ["BazelNoopRunner.cpp"],
        }),
        deps = select({
            # Only build and apply deps if we have a no-op test.
            "//src/gpu:has_gpu_backend": [
                harness,
                "//:skia_internal",
            ] + extra_deps,
            "//conditions:default": [],
        }),
        data = resources,
        set_flags = flags,
        target_compatible_with = limit_to,
        testonly = True,  # Needed to gain access to test-only files.
    )

    test_runner = "%s.sh" % name

    native.genrule(
        name = "%s_runner" % name,
        srcs = [test_binary] + resources + ["//resources:README"],
        outs = [test_runner],
        cmd = "echo '%s' > $@" % _TEST_RUNNER_TEMPLATE.format(test_binary = test_binary),
        testonly = True,
    )

    native.sh_test(
        name = name,
        size = "small",
        srcs = [test_runner],
        data = [test_binary] + resources,
        tags = [
            # We currently have no RBE machines with GPUs, so we cannot run these remotely.
            "no-remote",
        ],
    )

    archive = "%s_archive" % name
    archive_srcs = [test_runner, test_binary] + resources

    # Create an archive containing the test and its resources, with a structure that emulates the
    # environment expected by the test when executed via "bazel test". This archive can be pushed
    # to an Android device via "adb push", and the contained test can be executed on the device via
    # "adb shell" as long as the working directory is set to the directory where the archive is
    # extracted.
    #
    # See https://bazel.build/reference/test-encyclopedia#initial-conditions.
    native.genrule(
        name = archive,
        srcs = archive_srcs,
        outs = ["%s.tar.gz" % name],
        cmd = "\\\n".join([
            # Create archive root directory, and ensure it is cleaned up on exit, even in the case
            # of errors.
            "ARCHIVE_DIR=$$(mktemp -d);",
            "trap \"rm -rf $$ARCHIVE_DIR\" EXIT;",

            # execpaths point to physical files generated by Bazel (e.g.
            # bazel-out/k8-linux_x64-dbg/bin/tests/some_test), whereas rootpaths are the paths that
            # a binary running via "bazel run" or "bazel test" expects (e.g. tests/some_test).
            # Thus, we must map the former to the latter.
            #
            # Reference: https://bazel.build/reference/be/make-variables#predefined_label_variables
            "EXECPATHS=(%s);" % " ".join(["$(execpaths %s)" % src for src in archive_srcs]),
            "ROOTPATHS=(%s);" % " ".join(["$(rootpaths %s)" % src for src in archive_srcs]),
            "if [ $${#EXECPATHS[*]} -ne $${#ROOTPATHS[*]} ]; then",
            "    echo EXECPATHS and ROOTPATHS have different lengths;",
            "    exit 1;",
            "fi;",

            # Copy each file in EXECPATHS into ARCHIVE_DIR, renamed as its corresponding entry in
            # ROOTPATHS.
            "for (( i=0; i<$${#EXECPATHS[*]}; i++ )); do",
            "    EXECPATH=$${EXECPATHS[$$i]};",
            "    ROOTPATH=$${ROOTPATHS[$$i]};",
            "    mkdir -p $$ARCHIVE_DIR/$$(dirname $$ROOTPATH);",
            "    cp $$EXECPATH $$ARCHIVE_DIR/$$ROOTPATH;",
            "done;",

            # Create archive.
            "tar zcf $@ -C $$ARCHIVE_DIR .;",
        ]),
        testonly = True,  # Needed to gain access to test-only files.
    )

    adb_test_runner = "%s_via_adb.sh" % name

    native.genrule(
        name = "%s_via_adb_runner" % name,
        srcs = [archive, test_binary],
        outs = [adb_test_runner],
        cmd = "echo '%s' > $@" % _ADB_TEST_RUNNER_TEMPLATE.format(
            archive = archive,
            test_binary = test_binary,
        ),
        testonly = True,
    )

    native.sh_test(
        name = "%s_via_adb" % name,
        size = "small",
        srcs = [adb_test_runner],
        data = [archive],
        tags = [
            # This test runner assumes there is an appropriate Android device connected via USB,
            # therefore it cannot run on RBE.
            "no-remote",
        ],
    )
