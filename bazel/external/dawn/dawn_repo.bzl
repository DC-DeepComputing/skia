"""
This module defines the dawn_repo repository rule.
"""

def _dawn_repo_impl(repo_ctx):
    # Fetch the Dawn source
    repo_ctx.execute(["git", "init"])
    repo_ctx.execute(["git", "remote", "add", "origin", repo_ctx.attr.remote])
    res = repo_ctx.execute(["git", "fetch", "--depth", "1", "origin", repo_ctx.attr.commit])
    if res.return_code != 0:
        fail("Failed to fetch Dawn: " + res.stderr)
    repo_ctx.execute(["git", "reset", "--hard", "FETCH_HEAD"])

    # Copy the BUILD.bazel from Skia
    repo_ctx.execute(["rm", "-f", "BUILD.bazel", "dawn_files.bzl"])
    repo_ctx.symlink(repo_ctx.path(repo_ctx.attr.build_file), "BUILD.bazel")

    # Run the generator to create dawn_files.bzl
    generator_path = repo_ctx.path(repo_ctx.attr.generator_py)
    
    # We must explicitly watch the generator script because external command executions via repo_ctx.execute
    # do not automatically register file content dependencies, which causes changes to the python script
    # to be ignored by Bazel's repository cache.
    repo_ctx.watch(repo_ctx.attr.generator_py)
    
    res = repo_ctx.execute(["python3", generator_path, ".", "dawn_files.bzl"])
    if res.return_code != 0:
        fail("Failed to generate dawn_files.bzl: " + res.stderr)

# https://bazel.build/rules/lib/globals/bzl#repository_rule
dawn_repo = repository_rule(
    implementation = _dawn_repo_impl,
    attrs = {
        "commit": attr.string(mandatory = True),
        "remote": attr.string(mandatory = True),
        "build_file": attr.label(mandatory = True),
        "generator_py": attr.label(mandatory = True),
    },
)
