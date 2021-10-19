def _download_and_extract_deb(ctx, deb, sha256, prefix, output = ""):
    # https://docs.bazel.build/versions/main/skylark/lib/repository_ctx.html#download
    download_info = ctx.download(
        url = deb,
        output = "deb.ar",
        sha256 = sha256,
    )

    # https://docs.bazel.build/versions/main/skylark/lib/repository_ctx.html#execute
    # This uses the extracted llvm-ar that comes with clang.
    ctx.execute(["bin/llvm-ar", "x", "deb.ar"])

    # https://docs.bazel.build/versions/main/skylark/lib/repository_ctx.html#extract
    extract_info = ctx.extract(
        archive = "data.tar.xz",
        output = output,
        stripPrefix = prefix,
    )
    ctx.delete("deb.ar")

    #ctx.delete("data.tar.xz")
    ctx.delete("control.tar.xz")

def _build_cpp_toolchain_impl(ctx):
    # https://docs.bazel.build/versions/main/skylark/lib/repository_ctx.html#download
    download_info = ctx.download_and_extract(
        url = ctx.attr.clang_url,
        output = "",
        stripPrefix = ctx.attr.clang_prefix,
        sha256 = ctx.attr.clang_sha256,
    )

    _download_and_extract_deb(
        ctx,
        ctx.attr.libc_url,
        ctx.attr.libc_sha256,
        ctx.attr.libc_prefix,
    )

    # kjlubick cannot figure out how to get clang to look in this subdirectory
    # for the crt1.o files, so we'll move them up to here.
    for file in ["crt1.o", "crtn.o", "Scrt1.o", "crti.o", "rcrt1.o"]:
        ctx.execute(["cp", "usr/lib/x86_64-linux-musl/" + file, "usr/lib/"])

    _download_and_extract_deb(
        ctx,
        ctx.attr.cross_libc_url,
        ctx.attr.cross_libc_sha256,
        ctx.attr.cross_libc_prefix,
    )

    # https://docs.bazel.build/versions/main/skylark/lib/repository_ctx.html#file
    ctx.file(
        "BUILD.bazel",
        content = """
filegroup(
    name = "all_files",
    srcs = glob([
        "**",
    ]),
    visibility = ["//visibility:public"]
)
""",
        executable = False,
    )

build_cpp_toolchain = repository_rule(
    implementation = _build_cpp_toolchain_impl,
    attrs = {
        "clang_url": attr.string(mandatory = True),
        "clang_sha256": attr.string(mandatory = True),
        "clang_prefix": attr.string(mandatory = True),
        "libc_url": attr.string(mandatory = True),
        "libc_sha256": attr.string(mandatory = True),
        "libc_prefix": attr.string(mandatory = True),
        "cross_libc_url": attr.string(mandatory = True),
        "cross_libc_sha256": attr.string(mandatory = True),
        "cross_libc_prefix": attr.string(mandatory = True),
    },
    doc = "",
)
