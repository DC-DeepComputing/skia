"""This module defines the test_on_env rule."""

def test_on_env(name, test, env, timeout_secs = 10, tags = None, test_on_env_binary = None):
    """Allows running test targets that require launching a test environment before their execution.

    TODO(kjlubick) de-fork this after https://skia-review.googlesource.com/c/buildbot/+/510717 lands
    """
    if not tags:
        tags = []
    if not test_on_env_binary:
        test_on_env_binary = "@org_skia_go_infra//bazel/test_on_env:test_on_env"

    # Even though test_on_env is a go binary, it seems perfectly happy to be run with the
    # sh_test rule.
    native.sh_test(
        name = name,
        srcs = [test_on_env_binary],
        args = [
            "--test_bin=$(location %s)" % test,
            "--env_bin=$(location %s)" % env,
            "--ready_check=%ds" % timeout_secs,
        ],
        data = [test, env],
        tags = tags,
    )
