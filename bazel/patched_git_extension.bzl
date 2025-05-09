load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

patched_git_repo_tags = tag_class(
    attrs = {
        "name": attr.string(mandatory = True, doc = "The name to create."),
        "remote": attr.string(mandatory = True, doc = "The URL of the Git repository."),
        "commit": attr.string(mandatory = True, doc = "The Git commit hash to fetch."),
        "build_file": attr.label(allow_single_file = True, doc = "Label of the BUILD file to copy to the root of the repository."),
        "to_delete": attr.string_list(doc = "List of paths (relative to repo root) to delete from the fetched repository."),
        "patches": attr.label_list(doc="List of patch files to include")
    },
)

def _my_patches_extension_impl(ctx):
    for mod in ctx.modules:
        for tag in mod.tags.patched_git_repository:
            repo_name = tag.name

            git_repository(
                name = repo_name,
                remote = tag.remote,
                commit = tag.commit,
                build_file = tag.build_file,
                patches = tag.patches,
                patch_cmds = ["rm " + file for file in tag.to_delete],
                patch_cmds_win = ["del " + file for file in tag.to_delete],
            )

# Define the module extension itself
patched_git_extension = module_extension(
    implementation = _my_patches_extension_impl,
    tag_classes = {
        "patched_git_repository": patched_git_repo_tags,  # Associate the tag class with the desired invocation name
    },
)
