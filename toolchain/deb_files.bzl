def _deb_archive_impl(ctx):
    # https://docs.bazel.build/versions/main/skylark/lib/repository_ctx.html#download
    download_info = ctx.download(
        url = ctx.attr.url,
        output = "deb.ar",
        sha256 = ctx.attr.sha256,
    )

    # https://docs.bazel.build/versions/main/skylark/lib/repository_ctx.html#execute
    ctx.execute(["/usr/bin/ar", "x", "deb.ar"])

    # https://docs.bazel.build/versions/main/skylark/lib/repository_ctx.html#extract
    extract_info = ctx.extract(
        archive = "data.tar.xz",
        output = "",
        stripPrefix = ctx.attr.strip_prefix,
    )

    # https://docs.bazel.build/versions/main/skylark/lib/repository_ctx.html#file
    ctx.file(
        "BUILD.bazel",
        content = ctx.attr.build_file_content,
        executable = False,
    )

deb_archive = repository_rule(
    implementation = _deb_archive_impl,
    attrs = {
        "url": attr.string(mandatory = True),
        "sha256": attr.string(mandatory = True),
        "strip_prefix": attr.string(),
        "build_file_content": attr.string(mandatory = True),
    },
    doc = "",
)
