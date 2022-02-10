load("@npm//@bazel/concatjs/web_test:karma_web_test.bzl", _actual_karma_web_test = "karma_web_test")

def karma_web_test(name, srcs, **kwargs):
    """A thin wrapper around concatjs's karma_web_test

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

    # pass all the arguments unchanged
    # For the source code of this rule, see
    # https://github.com/bazelbuild/rules_nodejs/blob/700b7a3c5f97f2877320e6e699892ee706f85269/packages/concatjs/web_test/karma_web_test.bzl
    _actual_karma_web_test(name = name, srcs = srcs, **kwargs)
