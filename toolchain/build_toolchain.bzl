"""
This file assembles a toolchain for Linux using the Clang Compiler and musl.

It currently makes use of musl and not glibc because the pre-compiled libraries from the latter
have absolute paths baked in and this makes linking difficult. The pre-compiled musl library
does not have this restriction and is much easier to statically link in as a result.

As inputs, it takes external URLs from which to download the clang binaries/libraries/includes
as well as the musl headers and pre-compiled binaries. Those files are downloaded and put
into one folder, with a little bit of re-arrangement so clang can find files (like the C runtime).
"""

# From https://github.com/llvm/llvm-project/releases/download/llvmorg-13.0.0/clang+llvm-13.0.0-x86_64-linux-gnu-ubuntu-20.04.tar.xz.sha256
clang_prefix = "clang+llvm-13.0.0-x86_64-linux-gnu-ubuntu-20.04/"
clang_sha256 = "2c2fb857af97f41a5032e9ecadf7f78d3eff389a5cd3c9ec620d24f134ceb3c8"
clang_url = "https://github.com/llvm/llvm-project/releases/download/llvmorg-13.0.0/clang+llvm-13.0.0-x86_64-linux-gnu-ubuntu-20.04.tar.xz"

debs_to_install = [
    # This puts the musl include files in ${PWD}/usr/include/x86_64-linux-musl
    # and the runtime files and lib.a files in ${PWD}/usr/lib/x86_64-linux-musl
    {
        # From https://packages.debian.org/bullseye/amd64/musl-dev/download
        "sha256": "b017792ad6ba3650b4889238c73cd19c1d6b0e39ca8319cdd3ad9e16374e614e",
        "url": "http://ftp.debian.org/debian/pool/main/m/musl/musl-dev_1.2.2-1_amd64.deb",
    },
    # These two put the X11 include files in ${PWD}/usr/include/X11
    # libx11-dev puts libX11.a in ${PWD}/usr/lib/x86_64-linux-gnu
    {
        # From https://packages.debian.org/bullseye/amd64/libx11-dev/download
        "sha256": "11e5f9dcded1a1226b3ee02847b86edce525240367b3989274a891a43dc49f5f",
        "url": "http://ftp.debian.org/debian/pool/main/libx/libx11/libx11-dev_1.7.2-1_amd64.deb",
    },
    {
        # From https://packages.debian.org/bullseye/all/x11proto-dev/download
        "sha256": "d5568d587d9ad2664c34c14b0ac538ccb3c567e126ee5291085a8de704a565f5",
        "url": "http://ftp.debian.org/debian/pool/main/x/xorgproto/x11proto-dev_2020.1-1_all.deb",
    },
    # xcb is a dep of X11
    {
        # From https://packages.debian.org/bullseye/amd64/libxcb1-dev/download
        "sha256": "b75544f334c8963b8b7b0e8a88f8a7cde95a714dddbcda076d4beb669a961b58",
        "url": "http://ftp.debian.org/debian/pool/main/libx/libxcb/libxcb1-dev_1.14-3_amd64.deb",
    },
    # These two put GL include files in ${PWD}/usr/include/GL
    {
        # From https://packages.debian.org/bullseye/amd64/libgl-dev/download
        "sha256": "a6487873f2706bbabf9346cdb190f47f23a1464f31cecf92c363bac37c342f2f",
        "url": "http://ftp.debian.org/debian/pool/main/libg/libglvnd/libgl-dev_1.3.2-1_amd64.deb",
    },
    {
        # From https://packages.debian.org/bullseye/amd64/libglx-dev/download
        "sha256": "5a50549948bc4363eab32b1083dad2165402c3628f2ee85e9a32563228cc61c1",
        "url": "http://ftp.debian.org/debian/pool/main/libg/libglvnd/libglx-dev_1.3.2-1_amd64.deb",
    },
]

def _download_and_extract_deb(ctx, deb, sha256, prefix, output = ""):
    """Downloads a debian file and extracts the data into the provided output directory"""

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

    # Clean up
    ctx.delete("deb.ar")
    ctx.delete("data.tar.xz")
    ctx.delete("control.tar.xz")

def _build_cpp_toolchain_impl(ctx):
    # Download the clang toolchain (the extraction can take a while)
    # https://docs.bazel.build/versions/main/skylark/lib/repository_ctx.html#download
    download_info = ctx.download_and_extract(
        url = clang_url,
        output = "",
        stripPrefix = clang_prefix,
        sha256 = clang_sha256,
    )

    for deb in debs_to_install:
        _download_and_extract_deb(
            ctx,
            deb["url"],
            deb["sha256"],
            ".",
        )

    # kjlubick@ cannot figure out how to get clang to look in ${PWD}/usr/lib/x86_64-linux-musl
    # for the crt1.o files, so we'll move them to ${PWD}/usr/lib/ where clang *is* looking.
    for file in ["crt1.o", "crtn.o", "Scrt1.o", "crti.o", "rcrt1.o"]:
        ctx.execute(["cp", "usr/lib/x86_64-linux-musl/" + file, "usr/lib/"])

    # Create a BUILD.bazel file that makes all the files in this subfolder
    # available for use in rules, i.e. in the toolchain declaration.
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
    attrs = {},
    doc = "",
)
