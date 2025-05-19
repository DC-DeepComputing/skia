"""
This module extension creates git_repositories loaded from a json file.

It is important to separate the implementation from the actual data because
the MODULE.bazel.lock file has a checksum of the .bzl files

"""

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

# https://bazel.build/rules/lib/globals/bzl.html#tag_class
_from_file_tag = tag_class(
    doc = "A JSON file containing attributes for git_repository",
    attrs = {
        "deps_json": attr.label(
            doc = "The deps.json file to read and create repos from.",
        ),
    },
)

# ctx is https://bazel.build/rules/lib/builtins/module_ctx
def _cpp_modules_impl(ctx):
    # https://bazel.build/rules/lib/builtins/bazel_module.html
    for module in ctx.modules:
        #https://bazel.build/rules/lib/builtins/bazel_module_tags
        if len(module.tags.from_file) != 1:
            fail("Must have exactly one from_file attribute set")
        json_file_label = module.tags.from_file[0].deps_json

        # https://bazel.build/rules/lib/builtins/module_ctx#read
        json_content = ctx.read(json_file_label)
        data = json.decode(json_content)

        if "repos" not in data:
            fail("JSON file must contain a 'repos' list: {}".format(json_file_label))

        for repo_data in data["repos"]:
            name = repo_data.get("name")
            build_file = repo_data.get("build_file")
            commit = repo_data.get("commit")
            remote = repo_data.get("remote")

            if not all([name, build_file, commit, remote]):
                fail("Each repository in the JSON file must have 'name', 'build_file', 'commit', and 'remote' fields.")

            git_repository(
                name = name,
                build_file = build_file,
                commit = commit,
                remote = remote,
            )

cpp_modules = module_extension(
    implementation = _cpp_modules_impl,
    tag_classes = {
        "from_file": _from_file_tag,
    },
)