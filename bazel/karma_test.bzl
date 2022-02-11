load("@npm//@bazel/concatjs/web_test:karma_web_test.bzl", "karma_web_test_suite")

def karma_test(name, srcs, **kwargs):
    """Runs the given tests on Chromium downloaded via Bazel
    This is a thin wrapper around concatjs's karma_web_test_suite which in turn is a wrapper
    around karma_web_test and io_bazel_rules_webtesting's web_test_suite. karma_web_test
    configures karma to use the files from Bazel (e.g. it generates a karma.conf.js which
    lists the files to run and to make available). web_test_suite takes care of running the
    files on one or more browsers that have specific versions pinned and downloaded by
    Bazel.

    If we were to just use karma_web_test, the default is to run on the user's installed version
    of Chrome. We want this to be hermetic and self-contained, so we use karma_web_test_suite.

    Args:
        name: string, the name for the rule
        srcs: a non-empty list of js files that cannot end in spec.js or test.js.
        **kwargs: Any flags that karma_web_test normally takes.
    """
    if len(srcs) == 0:
        fail("Must pass at least one file into srcs or there will be no tests to run")

    # Beware the footgun of trying to include some_test.spec.js. The generated require.js code
    # https://github.com/bazelbuild/rules_nodejs/blob/9a065cdefd89c4e7bfac4a0c7dd41edb56cd7614/packages/concatjs/web_test/karma.conf.js#L316
    # renames any file that ends with .spec.js to just be .spec, as in, a *module* that should be
    # included. This results in the test file not being loaded properly, a la
    # https://github.com/bazelbuild/rules_nodejs/issues/1872
    for s in srcs:
        if s.endswith("spec.js") or s.endswith("test.js"):
            fail("srcs cannot end with spec.js or test.js or they will not load properly.")

    # Pass all the arguments unchanged. For the source code of this rule, see
    # https://github.com/bazelbuild/rules_nodejs/blob/700b7a3c5f97f2877320e6e699892ee706f85269/packages/concatjs/web_test/karma_web_test.bzl
    karma_web_test_suite(
        name = name,
        srcs = srcs,
        # Despite the name, "chromium-local" means to use the version of Chromium that is
        # downloaded via bazel into the sandbox.
        browsers = ["@io_bazel_rules_webtesting//browsers:chromium-local"],
        # The native tag is required by
        # https://github.com/bazelbuild/rules_webtesting/blob/6b2ef24cfe95d6e31d5e61414949ff8f0f184b13/browsers/BUILD.bazel#L54
        # It is not clear if it has any meaning to Bazel or is just to be explicit about something.
        tags = ["native"],
        **kwargs
    )
