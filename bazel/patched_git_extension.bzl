load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

patched_git_repo_tags = tag_class(
    attrs = {
        "name": attr.string(mandatory = True, doc = "The name to create."),
        "remote": attr.string(mandatory = True, doc = "The URL of the Git repository."),
        "commit": attr.string(mandatory = True, doc = "The Git commit hash to fetch."),
        "build_file": attr.label(allow_single_file = True, doc = "Label of the BUILD file to copy to the root of the repository."),
        "to_delete": attr.string_list(doc = "List of paths (relative to repo root) to delete from the fetched repository."),
        "to_copy": attr.string_dict(
            doc = "Map of label (as a string) of the source file to its destination path " +
                  "within the fetched repository (relative to the repo root).",
        ),
    },
)

def _my_patches_extension_impl(ctx):
    for mod in ctx.modules:
        for tag in mod.tags.patched_git_repository:
            repo_name = tag.name

            # There appears to be no easy way to copy files in via module_ctx
            # https://bazel.build/rules/lib/builtins/module_ctx
            # Instead, we get the absolute path for the provided labels and have the
            # post-patch commands copy them into the relative path of the repo that is
            # created.
            files_to_copy = []
            files_to_copy_win = []
            for label_str, target_relative_path in tag.to_copy.items():
                source_file_path = ctx.path(Label(label_str))
                files_to_copy.append("cp %s %s" % (source_file_path, target_relative_path))
                files_to_copy_win.append("copy %s %s" % (source_file_path, target_relative_path))

            git_repository(
                name = repo_name,
                remote = tag.remote,
                commit = tag.commit,
                build_file = tag.build_file,
                patch_cmds = ["rm " + file for file in tag.to_delete] + files_to_copy,
                patch_cmds_win = ["del " + file for file in tag.to_delete] + files_to_copy_win,
            )

# Define the module extension itself
patched_git_extension = module_extension(
    implementation = _my_patches_extension_impl,
    tag_classes = {
        "patched_git_repository": patched_git_repo_tags,  # Associate the tag class with the desired invocation name
    },
)
