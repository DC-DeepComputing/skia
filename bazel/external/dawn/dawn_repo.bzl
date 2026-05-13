"""
This module defines the dawn_repo repository rule.
"""

def _dawn_repo_impl(repo_ctx):
    # Fetch the Dawn source
    # We use git clone because it's easier to handle specific commits and submodules if needed,
    # though for now we just need the source at a commit.
    # Alternatively, use download_and_extract for performance if we have a stable URL.
    # Since we are already using git_repository in cpp_modules, we'll mimic that.

    # https://bazel.build/rules/lib/builtins/repository_ctx#download_and_extract
    # However, dawn.googlesource.com doesn't provide easy tarballs for commits.
    # We'll use git.

    repo_ctx.execute(["git", "init"])
    repo_ctx.execute(["git", "remote", "add", "origin", repo_ctx.attr.remote])
    res = repo_ctx.execute(["git", "fetch", "--depth", "1", "origin", repo_ctx.attr.commit])
    if res.return_code != 0:
        fail("Failed to fetch Dawn: " + res.stderr)
    repo_ctx.execute(["git", "reset", "--hard", "FETCH_HEAD"])

    # Apply patches
    for patch in repo_ctx.attr.patches:
        res = repo_ctx.execute(["patch", "-p1", "-i", repo_ctx.path(patch)])
        if res.return_code != 0:
            fail("Failed to apply patch {}: {}".format(patch, res.stderr))

    # Copy the BUILD.bazel and dawn_files.bzl from Skia
    # We remove existing files to avoid "File exists" errors when symlinking
    repo_ctx.execute(["rm", "-f", "BUILD.bazel", "dawn_files.bzl"])
    repo_ctx.symlink(repo_ctx.path(repo_ctx.attr.build_file), "BUILD.bazel")
    repo_ctx.symlink(repo_ctx.path(repo_ctx.attr.files_bzl), "dawn_files.bzl")

    # If there are any other files we need to inject, do it here.

# https://bazel.build/rules/lib/globals/bzl#repository_rule
dawn_repo = repository_rule(
    implementation = _dawn_repo_impl,
    attrs = {
        "commit": attr.string(mandatory = True),
        "remote": attr.string(mandatory = True),
        "patches": attr.label_list(),
        "build_file": attr.label(mandatory = True),
        "files_bzl": attr.label(mandatory = True),
    },
)
