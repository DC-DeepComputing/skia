"""
This module extension creates git_repositories loaded from a json file.

It is important to separate the implementation from the actual data because
the MODULE.bazel.lock file has a checksum of the .bzl files

"""

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

# https://bazel.build/rules/lib/globals/bzl.html#tag_class
_use_git_tag = tag_class(
    doc = "The attributes for an external git_repository",
    # A partial set of https://bazel.build/rules/lib/repo/git
    # If something has been omitted, it was through lack of need, not intent of omission.
    attrs = {
        # https://bazel.build/rules/lib/toplevel/attr#string
        "name": attr.string(
            doc = "The name of the git_repository",
            mandatory = True,
        ),
        "build_file": attr.label(
            doc = "If provided, will be injected into the git_repository after cloning",
            allow_single_file = True,
        ),
        "commit": attr.string(
            doc = "The commit at which to clone the git_repository",
            mandatory = True,
        ),
        "patches": attr.label_list(
            doc = "If provided, will be applied to the git_repository after cloning",
        ),
        "patch_cmds": attr.string_list(
            doc = "If provided, will be run in the git_repository on Linux/Mac",
        ),
        "patch_cmds_win": attr.label_list(
            doc = "If provided, will be run in the git_repository on Windows",
        ),
        "remote": attr.string(
            doc = "The url to the git_repository",
            mandatory = True,
        ),
    },
)

# ctx is https://bazel.build/rules/lib/builtins/module_ctx
def _cpp_modules_impl(ctx):
    # https://bazel.build/rules/lib/builtins/bazel_module.html
    for module in ctx.modules:
        # https://bazel.build/rules/lib/builtins/bazel_module_tags
        for repo_data in module.tags.use_git:
            git_repository(
                name = repo_data.name,
                build_file = repo_data.build_file,
                commit = repo_data.commit,
                patch_cmds = repo_data.patch_cmds,
                patch_cmds_win = repo_data.patch_cmds_win,
                patches = repo_data.patches,
                remote = repo_data.remote,
            )

cpp_modules = module_extension(
    implementation = _cpp_modules_impl,
    tag_classes = {
        "use_git": _use_git_tag,
    },
)
