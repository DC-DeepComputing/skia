load("//bazel:cipd_install.bzl", "cipd_install")

def _install_gn_impl(ctx):
    # https://bazel.build/rules/lib/builtins/module_ctx

    cipd_install(
        name = "gn_linux_amd64",
        build_file_content = """
exports_files(
    ["gn"],
    visibility = ["//visibility:public"]
)
    """,
        cipd_package = "gn/gn/linux-amd64",
        # From https://chrome-infra-packages.appspot.com/p/gn/gn/linux-amd64/+/git_revision:1c4151ff5c1d6fbf7fa800b8d4bb34d3abc03a41
        sha256 = "7195291488d08f3a10e85b85d8c4816e077015f1c5f196f770003a97aa42caf8",
        tag = "git_revision:1c4151ff5c1d6fbf7fa800b8d4bb34d3abc03a41",
    )

    cipd_install(
        name = "gn_mac_arm64",
        build_file_content = """
exports_files(
    ["gn"],
    visibility = ["//visibility:public"]
)
    """,
        cipd_package = "gn/gn/mac-arm64",
        # From https://chrome-infra-packages.appspot.com/p/gn/gn/mac-arm64/+/git_revision:1c4151ff5c1d6fbf7fa800b8d4bb34d3abc03a41
        sha256 = "1123907ac3317530e9dd537d50cd83fd83e852aacc07d286f45753c8fc5287ed",
        tag = "git_revision:1c4151ff5c1d6fbf7fa800b8d4bb34d3abc03a41",
    )

    cipd_install(
        name = "gn_mac_amd64",
        build_file_content = """
exports_files(
    ["gn"],
    visibility = ["//visibility:public"]
)
    """,
        cipd_package = "gn/gn/mac-amd64",
        # From https://chrome-infra-packages.appspot.com/p/gn/gn/mac-amd64/+/git_revision:1c4151ff5c1d6fbf7fa800b8d4bb34d3abc03a41
        sha256 = "ed96f7d2f49b83b016e4bdbed432e4734a5a133accb025d7c07685e01489ba93",
        tag = "git_revision:1c4151ff5c1d6fbf7fa800b8d4bb34d3abc03a41",
    )

install_gn = module_extension(
    implementation = _install_gn_impl,
)
